#pragma once
#include "UIWindow.h"
#include "component/UIPanel.h"
#include "component/UIButton.h"
#include "component/UILabel.h"
#include "component/UITexture.h"
#include "component/FlexLayout.h"
#include "utils/utils.h"
#include <nanovg.h>
#include <memory>
#include <vector>
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <thread>     
#include <mutex>     
#include <atomic>    
namespace fs = std::filesystem;

class VimagApp {
public:
    // 配置常量
    struct Config {
        static constexpr int DEFAULT_WINDOW_WIDTH = 1600;
        static constexpr int DEFAULT_WINDOW_HEIGHT = 1000;
        static constexpr float IMAGE_SCALE_RATIO = 1.0f;
        static constexpr float ANIMATION_DURATION = 0.25f;
        static constexpr float SCALE_STEP = 0.15f;
        static constexpr float MAX_SCALE = 13.0f;
        static constexpr float MIN_SCALE = 0.2f;
        static constexpr double TARGET_FPS = 120.0;
        static inline const NVGcolor BGCOLOR = nvgRGBA(32, 32, 32, 255);
    };

private:
    // 核心组件
    UIWindow window;
    std::shared_ptr<UIPanel> mainPanel;
    std::shared_ptr<UIPanel> rightPanel;
    std::shared_ptr<UIPanel> settingPanel;
    std::shared_ptr<UITexture> texture;
    std::shared_ptr<UILabel> label;
    std::shared_ptr<UILabel> indexLabel;
    std::shared_ptr<UIButton> indexButton;
    std::shared_ptr<UIButton> imageCycleButton;
    std::shared_ptr<UIButton> showExifInfo;

    // 应用状态
    std::vector<fs::path> imagePaths;
    std::vector<std::string> imageNames;
    size_t currentIndex = 0;
    int currentWindowWidth, currentWindowHeight;
    
    // 添加缺失的成员变量
    float totalDeltaX = 0.0f;
    float totalDeltaY = 0.0f;
    float scaleX = 1.0f, scaleY = 1.0f;
    int changeSpeed = 0;
    
    // 添加鼠标状态跟踪变量
    bool isLeftMousePressed = false;
    bool isDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    
    // 添加双击检测变量
    double lastClickTime = 0.0;
    static constexpr double DOUBLE_CLICK_TIME = 0.3; // 双击时间间隔（秒）
    
    // 配置变量
    bool imageCycle = true;
    bool imageNameDisplay = true;
    bool showIndex = true;
    bool showExif = true;
    bool enableExifOrientation = true;

    // 添加后台扫描相关成员变量
    bool m_needsDirectoryScan = false;
    fs::path m_scanDirectory;
    std::string m_originalFilePath;
    std::thread m_scanThread;
    std::mutex m_imageDataMutex;
    std::atomic<bool> m_scanCompleted{false};

    int textureOrientation = 0;

public:
    VimagApp();
    ~VimagApp();
    
    bool initialize(int argc, char** argv);
    void run();
    
private:
    // 初始化方法
    void loadSettings();
    void loadImages(const std::string& filePath);
    void createUI();
    void setupEventHandlers();
    
    // UI创建辅助方法
    void createMainPanel();
    void createImagePanel();
    void createSettingPanel();
    void createLabels();
    
    // 事件处理方法
    void handleImageChange(int direction);
    void updateImageDisplay();
    void updateWindowSize();
    void handleFullscreenToggle();
    void handleSettingToggle();
    void handleIndexButtonClick(std::shared_ptr<UIButton> btn);
    void handleExifButtonClick(std::shared_ptr<UIButton> btn);
    void handleCycleButtonClick(std::shared_ptr<UIButton> btn);
    
    // 事件设置方法
    void setupTextureEvents();
    void setupWindowEvents();
    
    // 动画和缩放
    void handleImageScale(float scrollY);
    void resetImageTransform();
    
    // 工具方法
    std::string getImageInfo() const;
    void updateImageLabels();
    
    // 添加后台扫描相关方法声明
    void startBackgroundDirectoryScan();
    void checkBackgroundScanCompletion();
    


    // 图像切换优化相关变量
    std::chrono::steady_clock::time_point lastImageChangeTime;
    std::chrono::steady_clock::time_point pendingImageLoadTime;
    bool hasPendingImageLoad = false;
    size_t pendingImageIndex = 0;
    static constexpr int FAST_SWITCH_THRESHOLD_MS = 20; // 快速切换阈值（毫秒）
    static constexpr int DELAYED_LOAD_MS = 50; // 延迟加载时间（毫秒）
};
