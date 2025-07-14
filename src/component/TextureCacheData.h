#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <functional>
#include <algorithm>
#include <filesystem>
#include <atomic>
#include <condition_variable>
#include <filesystem>
#include "../utils/utils.h"
#include "nanovg.h"

namespace fs = std::filesystem;

enum ImageType {
    PNG, JPG, GIF, BMP, TIFF, HDR, TGA, UNKNOWN
};

struct ImageData {
    unsigned char* data = nullptr;
    int width = 0;
    int height = 0;
    int channels = 0;
    int frames = 0;
    ImageType type = UNKNOWN;
};

struct TextureCacheData {
    ImageType type = UNKNOWN;
    int frame_count = 0;
    bool loading = false;
    bool loaded = false;
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<int> imageId;
};

class TextureCaches {
private:
    std::unordered_map<fs::path, TextureCacheData> cache;
    std::mutex cacheMutex;
    
    // 后台线程相关
    std::thread workerThread;
    std::queue<fs::path> loadQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::atomic<bool> shouldStop{false};
    
    // 主线程任务队列
    std::queue<std::function<void()>> mainThreadTasks;
    std::mutex mainThreadMutex;
    
    NVGcontext* nvgContext;
    
    void workerThreadFunc();
    ImageType detectImageType(const fs::path& path);
    ImageData loadImageData(const fs::path& path);
    void createTexturesFromData(const fs::path& path, const ImageData& imageData);

public:
    TextureCaches(NVGcontext* vg);
    ~TextureCaches();
    
    // 处理主线程任务（必须在主线程调用）
    void processMainThreadTasks();
    
    bool getImageCacheData(const fs::path& path, int& width, int& height, int& frame_count, std::vector<int>& imageId);
    bool addImageCacheData(const fs::path& path, const TextureCacheData& data);
    bool removeImageCacheData(const fs::path& path);
    void preloadImages(const std::vector<fs::path>& imagePaths);
    bool isImageLoaded(const fs::path& path);
    int getImageTexture(const fs::path& path, int frameIndex = 0);
    void cleanup();
};