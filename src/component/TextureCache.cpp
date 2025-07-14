#include "TextureCache.h"
#include <iostream>

TextureCache& TextureCache::getInstance() {
    static TextureCache instance;
    return instance;
}

void TextureCache::addRef(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_textureCache.find(path);
    if (it != m_textureCache.end()) {
        it->second.refCount++;
    } else {
        m_textureCache[path] = TextureInfo{-1, 1, false};
    }
}

void TextureCache::removeRef(const std::string& path, NVGcontext* vg) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_textureCache.find(path);
    if (it != m_textureCache.end()) {
        it->second.refCount--;
        if (it->second.refCount <= 0) {
            if (it->second.nvgHandle != -1) {
                nvgDeleteImage(vg, it->second.nvgHandle);
            }
            m_textureCache.erase(it);
        }
    }
}

void TextureCache::preloadTexture(NVGcontext* vg, const std::string& path) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_textureCache.find(path);
        if (it != m_textureCache.end()) {
            if (it->second.loading || it->second.nvgHandle != -1) return;
        }
        m_textureCache[path] = TextureInfo{-1, 0, true};
    }
    
    loadTextureAsync(vg, path);
}

void TextureCache::loadTextureAsync(NVGcontext* vg, const std::string& path) {
    std::thread([this, vg, path]() {
        try {
            int width, height, channels;
            unsigned char* data = ::LoadImage(path.c_str(), width, height, channels);
            if (!data) {
                std::cerr << "Failed to load texture: " << path << std::endl;
                return;
            }
            //在 lambda 捕获列表中添加 mutable 关键字，使得 lambda 可以修改捕获的变量
            postToMainThread([this, vg, path, data, width, height, channels]() mutable {
                int nvgImage = nvgCreateImageRGBA(vg, width, height, 0, data);
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    auto it = m_textureCache.find(path);
                    if (it != m_textureCache.end()) {
                        it->second.nvgHandle = nvgImage;
                        it->second.loading = false;
                        it->second.width = width;
                        it->second.height = height;
                        it->second.channels = channels;
                    }
                }
                // 创建一个非const的副本来传递给FreeImage
                unsigned char* mutableData = data;
                FreeImage(mutableData, path);
            });
        } catch (const std::exception& e) {
            std::cerr << "Exception in loadTextureAsync: " << e.what() << std::endl;
        }
    }).detach();
}

int TextureCache::getTexture(const std::string& path) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_textureCache.find(path);
    return (it != m_textureCache.end()) ? it->second.nvgHandle : -1;
}

bool TextureCache::isTextureLoaded(const std::string& path) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_textureCache.find(path);
    return (it != m_textureCache.end()) && (it->second.nvgHandle != -1);
}

bool TextureCache::getTextureInfo(const std::string& path, TextureInfo& info) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_textureCache.find(path);
    if (it != m_textureCache.end()) {
        info = it->second;
        return true;
    }
    return false;
}

void TextureCache::cleanup(NVGcontext* vg) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& pair : m_textureCache) {
        if (pair.second.nvgHandle != -1) {
            nvgDeleteImage(vg, pair.second.nvgHandle);
        }
    }
    m_textureCache.clear();
    m_mainThreadTasks = std::queue<std::function<void()>>();
}

void TextureCache::processMainThreadTasks() {
    std::queue<std::function<void()>> tasksToExecute;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        tasksToExecute = std::move(m_mainThreadTasks);
    }
    
    while (!tasksToExecute.empty()) {
        auto task = std::move(tasksToExecute.front());
        tasksToExecute.pop();
        try {
            task();
        } catch (const std::exception& e) {
            std::cerr << "Exception in texture task: " << e.what() << std::endl;
        }
    }
}

void TextureCache::postToMainThread(std::function<void()> task) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_mainThreadTasks.push(std::move(task));
}

TextureCache::~TextureCache() {
    // 清理任务队列
    m_mainThreadTasks = std::queue<std::function<void()>>();
}
// 删除这三行旧的静态成员变量定义
// std::mutex TextureCache::s_mutex;
// std::unordered_map<std::string, TextureCache::TextureInfo> TextureCache::s_textureCache;
// std::queue<std::function<void()>> TextureCache::s_mainThreadTasks;