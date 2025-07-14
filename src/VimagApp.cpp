#include "VimagApp.h"
#include "animation/UIAnimationManager.h"
#include "utils/utils.h"
#include "utils/setting.h"
#include "TinyEXIF/EXIF.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <thread>
#include <mutex>

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <unistd.h>
#endif

VimagApp::VimagApp() : window(Config::DEFAULT_WINDOW_WIDTH, Config::DEFAULT_WINDOW_HEIGHT, "VIMAG") {
    loadSettings();
}

VimagApp::~VimagApp() {
    // 等待后台线程完成
    if (m_scanThread.joinable()) {
        m_scanThread.join();
    }
}

bool VimagApp::initialize(int argc, char** argv) {
    // 加载图像
    if (argc > 1) {
        loadImages(argv[1]);
    } else {
        std::cout << "Usage: " << argv[0] << " <file_path>" << std::endl;
        // 使用默认图像
        imagePaths.push_back("Vimag.png");
        imageNames.push_back("Vimag.png");
    }
    
    // 初始化窗口
    window.enableDynamicTitleBar(true, 15.0);
    window.setTransparentFramebuffer(true);
    
    if (!window.initialize()) {
        std::cerr << "Failed to initialize window" << std::endl;
        return false;
    }
     // 添加窗口居中逻辑
    GLFWwindow* glfwWindow = window.getGLFWWindow();
    if (glfwWindow) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        if (monitor) {
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            if (mode) {
                int windowX = (mode->width - Config::DEFAULT_WINDOW_WIDTH) / 2;
                int windowY = (mode->height - Config::DEFAULT_WINDOW_HEIGHT) / 2;
                glfwSetWindowPos(glfwWindow, windowX, windowY);
                // 设置位置后显示窗口
                glfwShowWindow(glfwWindow);
            }
        }
    }
    
    window.getFramebufferSize(currentWindowWidth, currentWindowHeight);
    
    createUI();
    
    setupEventHandlers();
    
    return true;
}

void VimagApp::loadSettings() {
    ::loadSetting(); // 调用全局函数
    imageCycle = getSettingBool("Display", "image_cycle", true);
    imageNameDisplay = getSettingBool("Display", "image_name_display", true);
    showIndex = getSettingBool("Display", "image_index", true);
    showExif = getSettingBool("Display", "image_EXIF", true);
    enableExifOrientation = getSettingBool("Display", "Enable_Exif_orientation", true);
}

void VimagApp::loadImages(const std::string& filePath) {
    if (isFile(filePath)) {
        // 如果是文件，只加载这一个文件，延迟扫描目录
        fs::path directory = getDirectoryFromPath(filePath);
        imagePaths.push_back(fs::path(filePath));
        imageNames.push_back(fs::path(filePath).filename().string());
        currentIndex = 0;
        
        // 标记需要后台扫描
        m_needsDirectoryScan = true;
        m_scanDirectory = directory;
        m_originalFilePath = filePath;
    } else if (isDirectory(filePath)) {
        fs::path directory = filePath;
        find_image_files(directory, imagePaths, imageNames);
    } else {
        // 原有的默认处理逻辑
        fs::path directory = "./";
        getImages(filePath, directory, imagePaths, imageNames, currentIndex);
    }
    
    if (imagePaths.empty()) {
        imagePaths.push_back("Vimag.png");
        imageNames.push_back("Vimag.png");
        currentIndex = 0;
    }
}

// 修复 run 方法中的错误
void VimagApp::run() {
    OneTimeTimer timer;
    timer.start(1.5);
    
    const double targetFrameTime = 1.0 / Config::TARGET_FPS;
    auto lastTime = glfwGetTime();
    
    // 启动后台目录扫描
    if (m_needsDirectoryScan) {
        startBackgroundDirectoryScan();
    }
    glfwSwapInterval(1); // 强制启用垂直同步
    // 确保OpenGL设置正确
    glEnable(GL_MULTISAMPLE); // 启用多重采样抗锯齿
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    while (!window.shouldClose()) {
        auto currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        
        // === 优化帧率控制 ===
        if (deltaTime < targetFrameTime) {
            double sleepTime = (targetFrameTime - deltaTime) * 1000;
            if (sleepTime > 0.5) {
                #ifdef _WIN32
                                Sleep(static_cast<DWORD>(sleepTime * 0.8));
                #else
                                usleep((sleepTime * 0.8) * 1000);
                #endif
            }
            continue;
        }
        
        lastTime = currentTime;

        // 更新
        texture->update(deltaTime);
        window.pollEvents();
        UIAnimationManager::getInstance().update(deltaTime);
        
        // 定时器检查
        if (timer.check()) {
            updateImageLabels();
            updateWindowSize();
        }
        
        // 检查后台扫描是否完成
        checkBackgroundScanCompletion();

        
        // === 优化渲染条件 ===
        bool needsRender = (
            UIAnimationManager::getInstance().getAnimationCount() != 0 || 
            !texture->isPaintValid() || 
            // texture->isGif() ||
            settingPanel->isDisplay()
        );

       
       
        //全屏重绘
        if (needsRender) {
            window.beginFrame();
            window.clearBackground(0.3f, 0.3f, 0.3f, 1.0f);
            mainPanel->render(window.getNVGContext());
            indexLabel->render(window.getNVGContext());
            window.endFrame();
            window.swapBuffers();
            glFinish();
        }else{
            // // 局部重绘设置面板
            // if(settingPanel->isDisplay()){  
            //     window.beginFrame();
            //     nvgSave(window.getNVGContext());
            //     nvgScissor(window.getNVGContext(), 
            //             settingPanel->getX(), settingPanel->getY(),
            //             settingPanel->getWidth(), settingPanel->getHeight());
            //     nvgBeginPath(window.getNVGContext());
            //     nvgRect(window.getNVGContext(), 
            //         settingPanel->getX(), settingPanel->getY(),
            //         settingPanel->getWidth(), settingPanel->getHeight());
            //     nvgFillColor(window.getNVGContext(), nvgRGB(255, 255, 255));
            //     nvgFill(window.getNVGContext());
                
            //     // 重绘设置面板
            //     settingPanel->render(window.getNVGContext());
            //     indexLabel->render(window.getNVGContext());
            //     nvgRestore(window.getNVGContext());
            //     window.endFrame();
            //     window.swapBuffers();
            //     glFinish();
            //   }
        }


    }
    
    // 清理后台线程
    if (m_scanThread.joinable()) {
        m_scanThread.join();
    }
}

// 添加后台扫描方法的实现
void VimagApp::startBackgroundDirectoryScan() {
    m_scanThread = std::thread([this]() {
        std::vector<fs::path> allImagePaths;
        std::vector<std::string> allImageNames;
        
        // 在后台线程中扫描目录
        find_image_files(m_scanDirectory, allImagePaths, allImageNames);
        
        // 找到原始文件的索引
        size_t newIndex = findPathIndex(allImagePaths, m_originalFilePath);
        
        // 原子性地更新主线程数据
        {
            std::lock_guard<std::mutex> lock(m_imageDataMutex);
            imagePaths = std::move(allImagePaths);
            imageNames = std::move(allImageNames);
            if (newIndex != static_cast<size_t>(-1)) {
                currentIndex = newIndex;
            }
        }
        
        m_scanCompleted = true;
    });
}

void VimagApp::checkBackgroundScanCompletion() {
    if (m_scanCompleted.load()) {
        {
            std::lock_guard<std::mutex> lock(m_imageDataMutex);
            updateImageLabels(); // 更新显示的图片信息
        }
        m_scanCompleted = false;
        m_needsDirectoryScan = false;
    }
}

void VimagApp::createUI() {
    createMainPanel();
    createImagePanel();
    createSettingPanel();
    createLabels();
    updateImageLabels();
    mainPanel->updateLayout();
}

void VimagApp::createMainPanel() {
    mainPanel = std::make_shared<UIPanel>(0, 0, currentWindowWidth, currentWindowHeight);
    mainPanel->setHorizontalLayoutWithAlignment(FlexLayout::X_CENTER, FlexLayout::Y_CENTER, 0.0f, 0.0f);
    mainPanel->setBackgroundColor(Config::BGCOLOR);
    // mainPanel->setBorderColor(nvgRGBA(255, 255, 255, 255));
    // mainPanel->setBorderWidth(0.0f);
    // mainPanel->setCornerRadius(0.0f);
}

void VimagApp::createImagePanel() {
    rightPanel = std::make_shared<UIPanel>(0, 0, currentWindowWidth, currentWindowHeight);
    rightPanel->setVerticalLayoutWithAlignment(FlexLayout::X_CENTER, FlexLayout::Y_CENTER, 10.0f, 10.0f);
    rightPanel->setBackgroundColor(Config::BGCOLOR);
    
    // 创建纹理组件
    std::string imagePath = imagePaths[currentIndex].generic_string();
    texture = std::make_shared<UITexture>(0, 0, 
        currentWindowWidth * Config::IMAGE_SCALE_RATIO, 
        currentWindowHeight * Config::IMAGE_SCALE_RATIO, 
        imagePath);
    
    texture->setScaleMode(UITexture::ScaleMode::KEEP_ASPECT);
    texture->setAlpha(1.0f);
    // texture->setCornerRadius(1.0f);
    
    rightPanel->addChild(texture);
    mainPanel->addChild(rightPanel);
}

void VimagApp::createSettingPanel() {
    settingPanel = std::make_shared<UIPanel>(0, 0, 500, 480);
    settingPanel->setVerticalLayoutWithAlignment(FlexLayout::X_START, FlexLayout::Y_CENTER, 10.0f, 10.0f);
    settingPanel->setBackgroundColor(nvgRGBA(100, 100, 100, 100));
    settingPanel->setBorderColor(nvgRGBA(50, 50, 50, 10));
    // settingPanel->setBorderWidth(5.0f);
    settingPanel->setCornerRadius(5.0f);

    settingPanel->setDisplay(false);
    
    loadSettings();

    NVGcolor HoverColor = nvgRGBA(120, 170, 220, 150);
    NVGcolor FocusColor = nvgRGBA(80, 120, 180, 150);
    NVGcolor PressedColor = nvgRGBA(50, 100, 150, 150);

    // 创建索引按钮
    std::string indexButtonString = "Show Image Index";
    NVGcolor indexButtonBGCOLOR = nvgRGBA(255, 255, 255, 255);
    if (showIndex) {
        indexButtonString = "Hide Image Index";
        indexButtonBGCOLOR = nvgRGBA(0, 0, 0, 255);
    } 
    indexButton = std::make_shared<UIButton>(0, 0, 200, 50, indexButtonString);
    indexButton->setTextColor(indexButtonBGCOLOR);
    indexButton->setFontSize(18.0f);
    indexButton->setCornerRadius(25.0f);
    indexButton->setHoverColor(HoverColor);
    indexButton->setFocusColor(FocusColor);
    indexButton->setPressedColor(PressedColor);
    // 在 createSettingPanel() 函数中修改按钮回调设置
    indexButton->setOnClick([this]() {
        handleIndexButtonClick(indexButton);  // 直接传递 shared_ptr
    });
    std::string showExifInfoString = "Show EXIF";
    NVGcolor showExifInfoBGCOLOR = nvgRGBA(255, 255, 255, 255);
    if(showExifInfo){
        showExifInfoString = "Hide EXIF";
        showExifInfoBGCOLOR = nvgRGBA(0, 0, 0, 255);
    }
    showExifInfo = std::make_shared<UIButton>(0, 0, 200, 50, showExifInfoString);
    showExifInfo->setTextColor(showExifInfoBGCOLOR);
    showExifInfo->setFontSize(18.0f);
    showExifInfo->setCornerRadius(25.0f);
    showExifInfo->setHoverColor(HoverColor);
    showExifInfo->setFocusColor(FocusColor);
    showExifInfo->setPressedColor(PressedColor);
    showExifInfo->setOnClick([this]() {
        handleExifButtonClick(showExifInfo);  // 如果这里也要使用相同的逻辑
    });
    std::string imagCycleButtonString = "Enable index cycle";
    NVGcolor imageCycleButtonBGCOLOR = nvgRGBA(255, 255, 255, 255);
    if(imageCycle){
        imagCycleButtonString = "Disable index cycle";
        imageCycleButtonBGCOLOR = nvgRGBA(0, 0, 0, 255);
    }
    imageCycleButton = std::make_shared<UIButton>(0, 0, 200, 50,imagCycleButtonString);
    imageCycleButton->setTextColor(imageCycleButtonBGCOLOR);
    imageCycleButton->setFontSize(18.0f);
    imageCycleButton->setCornerRadius(25.0f);
    imageCycleButton->setHoverColor(HoverColor);
    imageCycleButton->setFocusColor(FocusColor);
    imageCycleButton->setPressedColor(PressedColor);
    imageCycleButton->setOnClick([this]() {
        handleCycleButtonClick(imageCycleButton);  // 如果这里也要使用相同的逻辑
    });
    settingPanel->addChild(indexButton);
    settingPanel->addChild(showExifInfo);
    settingPanel->addChild(imageCycleButton);
    mainPanel->addChild(settingPanel);
}

void VimagApp::createLabels() {
    label = std::make_shared<UILabel>(0, 0, 200, 50, " ");
    label->setTextAlign(UILabel::TextAlign::CENTER);
    label->setTextColor(nvgRGBA(150, 150, 150, 250));
    label->setFontSize(26.0f);
    
    indexLabel = std::make_shared<UILabel>(4, 2, 300, 50, " ");
    indexLabel->setTextAlign(UILabel::TextAlign::LEFT);
    indexLabel->setVerticalAlign(UILabel::VerticalAlign::TOP);
    indexLabel->setTextColor(nvgRGBA(140, 170, 194, 250));
    indexLabel->setFontSize(18.0f);
    indexLabel->setAnimationOpacity(1.0f); // 初始透明
    indexLabel->setDisplay(true);
}

void VimagApp::handleImageChange(int direction) {
    currentIndex += direction;
    
    // 修复参数传递 - enableImageCycle 需要引用参数
    size_t limitIndex = imagePaths.size();
    // bool imageCycle = true; // 从配置读取
    enableImageCycle(currentIndex, limitIndex, imageCycle);
    
    updateImageDisplay();
    updateImageLabels();
}

void VimagApp::updateImageDisplay() {
    std::string imagePath = imagePaths[currentIndex].generic_string();
    texture->setImagePath(window.getNVGContext(), imagePath);
    
    updateImageLabels();
    updateWindowSize();
}

void VimagApp::updateImageLabels() {
    std::string label_info="";
    std::string indexString = "[" + std::to_string(currentIndex + 1) + "/" + 
                             std::to_string(imagePaths.size()) + "]";
    std::string imageName = imageNames[currentIndex];
    
    if (texture->isLoadError()) {
        
        indexLabel->setText(indexString+" -> "+ imageName + "●●● LOAD FAIL! ●●●");
        texture -> setImagePath(window.getNVGContext(),"./imageFail.gif");
    } else {
        if (!showIndex) indexString = "";
        if (!imageNameDisplay) imageName = "";
        label_info = indexString +" ● " + imageName + " ● " + std::to_string(texture->getImageWidth()) + "x" + std::to_string(texture->getImageHeight());

        std::string exif_info;
        bool ExifInfo_S = getExifInfo(imagePaths[currentIndex].generic_string(), exif_info,textureOrientation);
        
        if(enableExifOrientation){
            if (ExifInfo_S) {
                if(textureOrientation != -100){
                        if(textureOrientation==90 || textureOrientation==-90){
                        float scaleOrientation = currentWindowHeight/texture->getWidth() + 0.05;
                        texture->scaleTo(scaleOrientation,scaleOrientation,0.01);
                    }else{
                        texture->scaleTo(1,1,0.01) ;
                    }
                    texture->rotateTo(textureOrientation,0.01);
                }
            } else{
                texture->rotateTo(0,0.01);
            }
        }
        if (showExif) {
            label_info += "\n"+exif_info;
        }
        indexLabel->setText(label_info);
    }
      
}

void VimagApp::updateWindowSize() {
    mainPanel->setSize(currentWindowWidth, currentWindowHeight);
    rightPanel->setSize(currentWindowWidth, currentWindowHeight);
    
    float newWidth = currentWindowWidth * Config::IMAGE_SCALE_RATIO;
    float newHeight = currentWindowHeight * Config::IMAGE_SCALE_RATIO;
    
    texture->setSize(newWidth, newHeight);
    texture->setOriginSize(newWidth, newHeight);
    texture->setPaintValid(false);
    
    rightPanel->updateLayout();
}

void VimagApp::handleIndexButtonClick(std::shared_ptr<UIButton> btn) {
    // 按钮淡出动画
    UIAnimationManager::getInstance().fadeOut(btn.get(), Config::ANIMATION_DURATION, UIAnimation::EASE_IN);

    // 状态切换逻辑
    if (btn->getText() == "Show Image Index") {
        btn->setTextColor(nvgRGBA(0, 0, 0, 255));
        btn->setText("Hide Image Index");
        showIndex = true;
        imageNameDisplay = true;
    } else {
        btn->setText("Show Image Index");
        btn->setTextColor(nvgRGBA(255, 255, 255, 255));
        imageNameDisplay = false;
        showIndex = false;
    }

    // 保存设置
    setSettingBool("Display", "image_index", showIndex);
    setSettingBool("Display", "image_name_display", imageNameDisplay);
    saveSetting();
    updateImageLabels();
}
void VimagApp::handleExifButtonClick(std::shared_ptr<UIButton> btn) {
    // 按钮淡出动画
    UIAnimationManager::getInstance().fadeOut(btn.get(), Config::ANIMATION_DURATION, UIAnimation::EASE_IN);

    // 状态切换逻辑
    if (btn->getText() == "Show EXIF") {
        btn->setTextColor(nvgRGBA(0, 0, 0, 255));
        btn->setText("Hide EXIF");
        showExif = true;
    } else {
        btn->setText("Show EXIF");
        btn->setTextColor(nvgRGBA(255, 255, 255, 255));
        showExif = false;
    }
    // 保存设置
    setSettingBool("Display", "image_EXIF", showExif);
    saveSetting();
    updateImageLabels();
}
void VimagApp::handleCycleButtonClick(std::shared_ptr<UIButton> btn){
// 按钮淡出动画
    UIAnimationManager::getInstance().fadeOut(btn.get(), Config::ANIMATION_DURATION, UIAnimation::EASE_IN);

    // 状态切换逻辑
    if (btn->getText() == "Enable index cycle") {
        btn->setTextColor(nvgRGBA(0, 0, 0, 255));
        btn->setText("Disable index cycle");
        imageCycle = true;
    } else {
        btn->setText("Enable index cycle");
        btn->setTextColor(nvgRGBA(255, 255, 255, 255));
        imageCycle = false;
    }
    // 保存设置
    setSettingBool("Display", "image_cycle", imageCycle);
    saveSetting();

}
void VimagApp::setupEventHandlers() {
    // 窗口大小变化回调
    window.setWindowSizeCallback([this](int width, int height) {
        currentWindowWidth = width;
        currentWindowHeight = height;
        updateWindowSize();
        mainPanel->updateLayout();
    });
    
    // 纹理事件处理
    setupTextureEvents();
    
    // 窗口事件处理
    setupWindowEvents();
}

void VimagApp::setupTextureEvents() {
    // 拖拽事件 - 现在可以直接使用成员变量
    texture->setOnDrag([this](float deltaX, float deltaY) {
        totalDeltaX += deltaX * 2;
        totalDeltaY += deltaY * 2;
        rightPanel->moveTo(totalDeltaX, totalDeltaY, 0.3f);
        mainPanel->updateLayout();
    });
    
    // 滚轮缩放
    texture->setOnScroll([this](float scrollX, float scrollY) {
        handleImageScale(scrollY);
    });
    

    
    // 双击重置
    texture->setOnDoubleClick([this](int mouseButton) {
        if (mouseButton == 0) {
            resetImageTransform();
        }
    });
    
    // 启用事件
    texture->setDragEnabled(true);
    texture->setScrollEnabled(true);
    texture->setDoubleClickEnabled(true);
    // texture->setDragScrollEnabled(true); // 禁用纹理的拖拽滚轮
}

void VimagApp::setupWindowEvents() {
    // 设置鼠标按钮事件回调
    window.setMouseButtonCallback([this](int button, int action, int mods) {
        double xpos, ypos;
        glfwGetCursorPos(window.getGLFWWindow(), &xpos, &ypos);
        
        // 跟踪左键状态和拖拽
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                isLeftMousePressed = true;
                isDragging = false;
                lastMouseX = xpos;
                lastMouseY = ypos;
                
                // 双击检测
                double currentTime = glfwGetTime();
                if (currentTime - lastClickTime < DOUBLE_CLICK_TIME) {
                    // 双击事件 - 重置缩放和移动
                    std::cout << "双击检测到，重置图像变换" << std::endl;
                    resetImageTransform();
                    lastClickTime = 0.0; // 重置时间，避免三击触发
                    return;
                }
                lastClickTime = currentTime;
            } else if (action == GLFW_RELEASE) {
                isLeftMousePressed = false;
                isDragging = false;
            }
        }
        
        UIEvent event;
        event.type = (action == GLFW_PRESS) ? UIEvent::MOUSE_PRESS : UIEvent::MOUSE_RELEASE;
        event.mouseX = static_cast<float>(xpos);
        event.mouseY = static_cast<float>(ypos);
        event.mouseButton = button;
        event.clickTime = glfwGetTime();
        
        // 中键全屏切换
        if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
            handleFullscreenToggle();
        }
        // 右键设置面板切换
        else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
            handleSettingToggle();
        }
        
        mainPanel->handleEvent(event);
        
        // 添加设置面板事件处理
        if (settingPanel && settingPanel->isDisplay()) {
            settingPanel->handleEvent(event);
        }
    });
    
    // 设置鼠标移动事件回调 - 添加拖拽功能
    window.setCursorPosCallback([this](double xpos, double ypos) {
        // 处理拖拽移动
        if (isLeftMousePressed) {
            if (!isDragging) {
                // 开始拖拽
                isDragging = true;
            } else {
                // 计算拖拽偏移
                double deltaX = xpos - lastMouseX;
                double deltaY = ypos - lastMouseY;
                
                // 更新总偏移量
                totalDeltaX += static_cast<float>(deltaX * 2);
                totalDeltaY += static_cast<float>(deltaY * 2);
                
                // 移动右侧面板
                rightPanel->moveTo(totalDeltaX, totalDeltaY, 0.3f);
                mainPanel->updateLayout();
                
                std::cout << "拖拽移动: deltaX=" << deltaX << ", deltaY=" << deltaY << std::endl;
            }
            
            lastMouseX = xpos;
            lastMouseY = ypos;
            return; // 拖拽时不传递给其他组件
        }
        
        // 正常鼠标移动事件
        UIEvent event;
        event.type = UIEvent::MOUSE_MOVE;
        event.mouseX = static_cast<float>(xpos);
        event.mouseY = static_cast<float>(ypos);
        event.mouseButton = -1;
        
        mainPanel->handleEvent(event);
        
        // 添加设置面板事件处理
        if (settingPanel && settingPanel->isDisplay()) {
            settingPanel->handleEvent(event);
        }
    });
    
    // 设置键盘事件回调 - 包含图片切换逻辑
    window.setKeyCallback([this](int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            UIEvent event;
            event.type = UIEvent::KEY_PRESS;
            event.keyCode = key;
            event.modifiers = mods;
            
            int direction = 0;
            
            // 方向键处理
            if (key == GLFW_KEY_LEFT) {
                std::cout << "Left key pressed" << std::endl;
                direction = -1;
            } 
            else if (key == GLFW_KEY_RIGHT) {
                std::cout << "Right key pressed" << std::endl;
                direction = 1;
            }
            else if (key == GLFW_KEY_F) {
                handleFullscreenToggle();
                return;
            }
            
            // 执行图片切换
            if (direction != 0) {
                handleImageChange(direction);
            }
            
            mainPanel->handleEvent(event);
        }
    });

    // 设置字符输入事件回调
    window.setCharCallback([this](unsigned int codepoint) {
        UIEvent event;
        event.type = UIEvent::CHAR_INPUT;
        event.character = codepoint;
        
        mainPanel->handleEvent(event);
    });
    
    // 设置滚轮事件处理 - 添加拖拽滚轮切换图片功能
    window.setScrollCallback([this](double xoffset, double yoffset) {
        UIEvent event;
        event.type = UIEvent::MOUSE_SCROLL;
        
        double xpos, ypos;
        glfwGetCursorPos(window.getGLFWWindow(), &xpos, &ypos);
        event.mouseX = static_cast<float>(xpos);
        event.mouseY = static_cast<float>(ypos);
        event.scrollX = static_cast<float>(xoffset);
        event.scrollY = static_cast<float>(yoffset);
        
        // 检查是否按住左键进行拖拽滚轮切换图片
        if (isLeftMousePressed) {
            changeSpeed += -static_cast<int>(yoffset);
            if (changeSpeed % 2 != 0) return;
            
            int direction = changeSpeed / 2;
            changeSpeed = 0;
            
            std::cout << "拖拽滚轮切换图片，方向: " << direction << std::endl;
            handleImageChange(direction);

            return; // 不传递给其他组件
        }
        
        // 正常滚轮事件传递给组件
        mainPanel->handleEvent(event);
    });
    
    // 设置拖放回调
    window.setDropCallback([this](int count, const char** paths) {
        std::cout << "Drop event: " << count << " files dropped" << std::endl;
        for (int i = 0; i < count; ++i) {
            std::cout << "File path: " << paths[i] << std::endl;
            // 这里可以添加拖放文件的处理逻辑
        }
    });
}

void VimagApp::handleImageScale(float scrollY) {
    scaleX += scrollY * Config::SCALE_STEP;
    scaleY += scrollY * Config::SCALE_STEP;
    
    scaleX = std::clamp(scaleX, Config::MIN_SCALE, Config::MAX_SCALE);
    scaleY = std::clamp(scaleY, Config::MIN_SCALE, Config::MAX_SCALE);
    
    // 使用更短的动画时间和更平滑的缓动
    UIAnimationManager::getInstance().scaleTo(texture.get(), scaleX, scaleY, 0.1f, UIAnimation::EASE_OUT);
    
    // 立即标记需要重绘
    texture->setPaintValid(false);
}

void VimagApp::resetImageTransform() {
    // 重置缩放
    if(textureOrientation != 0){
        if(textureOrientation==90 || textureOrientation==-90){
            float scaleOrientation = currentWindowHeight/texture->getWidth() + 0.00;
            // texture->scaleTo(scaleOrientation,scaleOrientation,0.01);
            UIAnimationManager::getInstance().scaleTo(texture.get(), scaleOrientation, scaleOrientation, 0.35f, UIAnimation::EASE_OUT);
        }
    }else{
            scaleX = scaleY = 1.0f;
            UIAnimationManager::getInstance().scaleTo(texture.get(), scaleX, scaleY, 0.35f, UIAnimation::EASE_OUT);
    }
    
    // 重置位置
    int aX = rightPanel->getX();
    int aY = rightPanel->getY();
    rightPanel->moveTo(aX, aY, 0.35f); // 移动到原始位置
    
    // 更新显示
    texture->setPaintValid(false);
    mainPanel->updateLayout();
    
    // std::cout << "图像变换已重置：缩放=1.0, 位置=(0,0)" << std::endl;
}

void VimagApp::handleFullscreenToggle() {
    window.toggleFullscreen();
    window.getFramebufferSize(currentWindowWidth, currentWindowHeight);
    updateWindowSize();
}

void VimagApp::handleSettingToggle() {
    std::cout << "右键单击 切换设置面板" << std::endl;
    
    // 获取当前位置
    int prePositionX = rightPanel->getX();
    int prePositionY = rightPanel->getY();
    
    // 预计算目标位置
    int targetX, targetY;
    
    if (settingPanel->isDisplay()) {
        // 隐藏设置面板：rightPanel 会向左移动
        targetX = prePositionX - settingPanel->getWidth() - 10; // 减去设置面板宽度和间距
        targetY = prePositionY;
        
        std::cout << "隐藏设置面板" << std::endl;
        
        // 先移动 rightPanel 到目标位置
        rightPanel->moveTo(0, 0, 0.3f);
        settingPanel->setDisplay(false);
        settingPanel->setEnabled(false);
        mainPanel->updateLayout(); // 最后更新布局确保一致性
        // // 然后执行设置面板隐藏动画
        // UIAnimationManager::getInstance().fadeOut(settingPanel.get(), 0.3f, UIAnimation::EASE_OUT);
        
        // auto hideAnim = std::make_shared<UIAnimation>(UIAnimation::CUSTOM, 0.01f);
        // hideAnim->setOnComplete([this]() {

        // });
        // UIAnimationManager::getInstance().addAnimation(hideAnim, settingPanel.get());
        
    } else {
        // 显示设置面板：rightPanel 会向右移动

        std::cout << "显示设置面板" << std::endl;
        rightPanel->moveTo(-250, 0, 0.3f);
        // 先设置面板可见
        settingPanel->setDisplay(true);
        settingPanel->setEnabled(true);
        
        // 同时执行两个动画
        
        UIAnimationManager::getInstance().fadeIn(settingPanel.get(), 0.3f, UIAnimation::EASE_OUT);
        
        // 延迟更新布局确保一致性
        auto layoutAnim = std::make_shared<UIAnimation>(UIAnimation::CUSTOM, 0.35f);
        layoutAnim->setOnComplete([this]() {
            mainPanel->updateLayout();
        });
        UIAnimationManager::getInstance().addAnimation(layoutAnim, mainPanel.get());
    }
    
    std::cout << "prePositionX: " << prePositionX << " prePositionY: " << prePositionY << std::endl;
    std::cout << "targetPositionX: " << targetX << " targetPositionY: " << targetY << std::endl;
}

std::string VimagApp::getImageInfo() const {
    // 实现获取图像信息逻辑
    return "";
}
