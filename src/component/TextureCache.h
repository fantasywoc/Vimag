#pragma once

#include <string>
#include <functional>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <memory>
#include <nanovg.h>
#include "../utils/utils.h"

#if defined(_WIN32)
    #include <windows.h>
    #ifdef LoadImage
        #undef LoadImage
    #endif
#elif defined(__APPLE__) || defined(__unix__) 
    #include <unistd.h>
    #include <cstdint>
#endif

class TextureCache {
public:
    struct TextureInfo {
        int nvgHandle = -1;
        int refCount = 0;
        bool loading = false;
        int width = 0;
        int height = 0;
        int channels = 0;
    };

    // 禁止拷贝和赋值
    TextureCache(const TextureCache&) = delete;
    TextureCache& operator=(const TextureCache&) = delete;

    // 单例模式
    static TextureCache& getInstance();

    // 纹理管理
    void addRef(const std::string& path);
    void removeRef(const std::string& path, NVGcontext* vg);
    void preloadTexture(NVGcontext* vg, const std::string& path);
    int getTexture(const std::string& path) const;
    bool isTextureLoaded(const std::string& path) const;
    bool getTextureInfo(const std::string& path, TextureInfo& info) const;

    // 资源清理
    void cleanup(NVGcontext* vg);
    void processMainThreadTasks();

    // 析构函数
    ~TextureCache();

private:
    TextureCache() = default;
    
    void postToMainThread(std::function<void()> task);
    void loadTextureAsync(NVGcontext* vg, const std::string& path);

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, TextureInfo> m_textureCache;
    std::queue<std::function<void()>> m_mainThreadTasks;
};