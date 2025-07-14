#include "TextureCacheData.h"
#include <iostream>
#include <filesystem>
#include <algorithm>

TextureCaches::TextureCaches(NVGcontext* vg) : nvgContext(vg) {
    workerThread = std::thread(&TextureCaches::workerThreadFunc, this);
}

TextureCaches::~TextureCaches() {
    shouldStop = true;
    queueCondition.notify_all();
    if (workerThread.joinable()) {
        workerThread.join();
    }
    cleanup();
}

void TextureCaches::workerThreadFunc() {
    while (!shouldStop) {
        std::unique_lock<std::mutex> lock(queueMutex);
        queueCondition.wait(lock, [this] { return !loadQueue.empty() || shouldStop; });
        
        if (shouldStop) break;
        
        fs::path path = loadQueue.front();
        loadQueue.pop();
        lock.unlock();
        
        // 检查是否已经在缓存中
        {
            std::lock_guard<std::mutex> cacheLock(cacheMutex);
            auto it = cache.find(path);
            if (it != cache.end() && (it->second.loaded || it->second.loading)) {
                continue;
            }
            cache[path].loading = true;
        }
        
        // 在后台线程中只加载图像数据
        ImageData imageData = loadImageData(path);
        
        if (imageData.data) {
            // 将纹理创建任务提交到主线程
            std::lock_guard<std::mutex> mainLock(mainThreadMutex);
            mainThreadTasks.push([this, path, imageData]() {
                createTexturesFromData(path, imageData);
            });
        } else {
            std::lock_guard<std::mutex> cacheLock(cacheMutex);
            cache.erase(path);
            std::cerr << "Failed to load image data: " << path << std::endl;
        }
    }
}

ImageType TextureCaches::detectImageType(const fs::path& path) {
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == ".png") return PNG;
    if (ext == ".jpg" || ext == ".jpeg") return JPG;
    if (ext == ".gif") return GIF;
    if (ext == ".bmp") return BMP;
    if (ext == ".tiff" || ext == ".tif") return TIFF;
    if (ext == ".hdr") return HDR;
    if (ext == ".tga") return TGA;
    
    return UNKNOWN;
}

ImageData TextureCaches::loadImageData(const fs::path& path) {
    ImageData result;
    result.type = detectImageType(path);
    std::string pathStr = path.generic_string();
    
    if (result.type == GIF) {
        // result.data = loadGifImage(pathStr, result.width, result.height, result.channels, result.frames);
    } else if (result.type != UNKNOWN) {
        result.data = LoadImage(pathStr, result.width, result.height, result.channels);
        result.frames = 1;
    }
    
    return result;
}

void TextureCaches::createTexturesFromData(const fs::path& path, const ImageData& imageData) {
    if (!imageData.data) return;
    
    TextureCacheData cacheData;
    cacheData.type = imageData.type;
    cacheData.width = imageData.width;
    cacheData.height = imageData.height;
    cacheData.channels = imageData.channels;
    cacheData.frame_count = imageData.frames;
    cacheData.imageId.clear();
    
    if (imageData.type == GIF && imageData.frames > 1) {
        // GIF 多帧处理
        std::cout << "[TextureCache] Creating " << imageData.frames << " GPU textures for GIF..." << std::endl;
        cacheData.imageId.reserve(imageData.frames);
        size_t frameSize = imageData.width * imageData.height * 4; // RGBA
        
        for (int i = 0; i < imageData.frames; i++) {
            unsigned char* frameData = imageData.data + (i * frameSize);
            int textureId = nvgCreateImageRGBA(nvgContext, imageData.width, imageData.height, 0, frameData);
            
            if (textureId == -1) {
                std::cerr << "[TextureCache] Failed to create texture for frame " << i  << " of " << path.filename() << std::endl;
                // 清理已创建的纹理
                for (int texId : cacheData.imageId) {
                    nvgDeleteImage(nvgContext, texId);
                }
                cacheData.imageId.clear();
                break;
            }
            cacheData.imageId.push_back(textureId);
        }
    } else {
        // 普通图片处理
        std::cout << "[TextureCache] Creating single GPU texture..." << std::endl;
        int textureId = nvgCreateImageRGBA(nvgContext, imageData.width, imageData.height, 0, imageData.data);
        if (textureId != -1) {
            cacheData.imageId.push_back(textureId);
            std::cout << "[TextureCache] GPU texture created successfully (ID: " << textureId << ")" << std::endl;
        }else {
            std::cerr << "[TextureCache] Failed to create GPU texture for: " << path.filename() << std::endl;
        }
    }
    
    // 释放图像数据
    std::string pathStr = path.generic_string();
    unsigned char* mutableData = const_cast<unsigned char*>(imageData.data);
    FreeImage(mutableData, pathStr);
    std::cout << "[TextureCache] Released image data memory for: " << path.filename() << std::endl;
    // 更新缓存
    {
        std::lock_guard<std::mutex> lock(cacheMutex);
        if (!cacheData.imageId.empty()) {
            cacheData.loading = false;
            cacheData.loaded = true;
            cache[path] = cacheData;
            std::cout << "[TextureCache] ✓ Successfully cached: " << path.filename() 
                      << " (" << cacheData.width << "x" << cacheData.height << ", " 
                      << cacheData.imageId.size() << " textures)" << std::endl;
        } else {
            cache.erase(path);
            std::cerr << "[TextureCache] ✗ Failed to create textures for: " << path.filename() << std::endl;
        }
    }
}

void TextureCaches::processMainThreadTasks() {
    std::queue<std::function<void()>> tasksToProcess;
    {
        std::lock_guard<std::mutex> lock(mainThreadMutex);
        tasksToProcess = std::move(mainThreadTasks);
    }
    
    while (!tasksToProcess.empty()) {
        auto task = std::move(tasksToProcess.front());
        tasksToProcess.pop();
        try {
            task();
        } catch (const std::exception& e) {
            std::cerr << "Exception in main thread task: " << e.what() << std::endl;
        }
    }
}

bool TextureCaches::getImageCacheData(const fs::path& path, int& width, int& height, int& frame_count, std::vector<int>& imageId) {
    std::lock_guard<std::mutex> lock(cacheMutex);
    auto it = cache.find(path);
    if (it != cache.end() && it->second.loaded) {
        width = it->second.width;
        height = it->second.height;
        frame_count = it->second.frame_count;
        imageId = it->second.imageId;
        return true;
    }
    return false;
}

void TextureCaches::preloadImages(const std::vector<fs::path>& imagePaths) {
    std::lock_guard<std::mutex> lock(queueMutex);
    for (const auto& path : imagePaths) {
        loadQueue.push(path);
    }
    queueCondition.notify_all();
}

void TextureCaches::cleanup() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    for (auto& pair : cache) {
        for (int texId : pair.second.imageId) {
            if (texId != -1) {
                nvgDeleteImage(nvgContext, texId);
            }
        }
    }
    cache.clear();
}