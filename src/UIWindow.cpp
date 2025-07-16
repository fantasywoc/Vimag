/**
 * @file UIWindow.cpp
 * @brief UI窗口管理类的实现文件
 * @description 实现UIWindow类的所有功能
 */

#include "UIWindow.h"
#include <iostream>

// 平台检测
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>

#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11  // 启用 X11 原生接口
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#elif defined(__APPLE__)
#include <CoreGraphics/CoreGraphics.h>
#endif

// 定义NanoVG OpenGL3实现宏并包含实现头文件
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"
#include "./utils/utils.h"
#include "stb_image.h"


/**
 * @brief UIWindow构造函数
 * @description 初始化所有成员变量为默认值
 */
UIWindow::UIWindow(int width, int height, const std::string& title)
    : window(nullptr),           // GLFW窗口句柄初始化为空
      vg(nullptr),              // NanoVG上下文初始化为空
      windowWidth(width),       // 设置窗口宽度
      windowHeight(height),     // 设置窗口高度
      windowTitle(title),       // 设置窗口标题
      initialized(false) {      // 初始化状态为false
}

/**
 * @brief UIWindow析构函数
 * @description 确保所有资源被正确释放
 */
UIWindow::~UIWindow() {
    cleanup(); // 调用清理函数释放资源
}

/**
 * @brief 初始化窗口和所有相关组件
 * @return bool 成功返回true，失败返回false
 * @description 按顺序初始化GLFW、创建窗口、初始化GLEW和NanoVG
 */
void UIWindow::setDropCallback(std::function<void(int, const char**)> callback) {
    dropCallback = callback;
    if (window) {
        glfwSetDropCallback(window, dropCallbackWrapper);
    }
}

void UIWindow::dropCallbackWrapper(GLFWwindow* window, int count, const char** paths) {
    UIWindow* self = static_cast<UIWindow*>(glfwGetWindowUserPointer(window));
    if (self && self->dropCallback) {
        self->dropCallback(count, paths);
    }
}

// 在initialize()方法中添加
bool UIWindow::initialize() {
    // 防止重复初始化
    if (initialized) return true;

    // 第一步：初始化GLFW库
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // 第二步：设置窗口创建提示
    setWindowHints();




    // 第三步：创建GLFW窗口
    window = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate(); // 创建失败时清理GLFW
        return false;
    }


    
    // 配置窗口图标
    GLFWimage icon;
    icon.pixels = stbi_load("./logo.png", &icon.width, &icon.height, nullptr, 4); // 强制 RGBA
    if (icon.pixels) {
        glfwSetWindowIcon(window, 1, &icon); // 设置系统图标
        stbi_image_free(icon.pixels); // 立即释放内存
    } else {
        std::cerr << "Warning: Failed to load window icon" << std::endl;
    }


    // 第四步：设置OpenGL上下文为当前上下文
    // 在glfwMakeContextCurrent之后添加
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // 启用垂直同步
    // 在glfwMakeContextCurrent(window)之后添加
    if (dropCallback) {
        glfwSetDropCallback(window, dropCallbackWrapper);
    }
    glfwSetWindowUserPointer(window, this);

    // 第六步：初始化GLEW（OpenGL扩展加载库）
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        cleanup();
        return false;
    }

    // 第七步：创建NanoVG渲染上下文
    // NVG_ANTIALIAS: 启用抗锯齿
    // NVG_STENCIL_STROKES: 启用模板缓冲区用于描边
    // 在 initialize() 方法中，创建NanoVG上下文之后添加
    vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
    if (!vg) {
        std::cerr << "Failed to initialize NanoVG" << std::endl;
        cleanup();
        return false;
    }
    
    // 添加字体加载代码
    int font = nvgCreateFont(vg, "default", "./msyh.ttc");
    if (font == -1) {
        std::cerr << "Warning: Failed to load  HarmonyOS_Sans_Regular font, text may not display properly" << std::endl;
        // 如果系统字体加载失败，尝试加载其他字体
        font = nvgCreateFont(vg, "default", "C:/Windows/Fonts/msyh.ttc");
        if (font == -1) {
            std::cerr << "Warning: Failed to load font, text may not display properly" << std::endl;
        }else{
            std::cout << "Successfully loaded font  C:/Windows/Fonts/msyh.ttc" << std::endl;
        }
    }else{
        std::cout << "Successfully loaded font  ./msyh.ttc" << std::endl;
    }

    nvgFontFace(vg, "default");

    // 所有组件初始化成功
    initialized = true;
    return true;
}

/**
 * @brief 清理所有分配的资源
 * @description 按相反顺序清理资源：NanoVG -> GLFW窗口 -> GLFW库
 */
void UIWindow::cleanup() {
    // 清理NanoVG上下文
    if (vg) {
        nvgDeleteGL3(vg);
        vg = nullptr;
    }
    
    // 销毁GLFW窗口
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    
    // 终止GLFW库
    glfwTerminate();
    
    // 重置初始化状态
    initialized = false;
}

/**
 * @brief 设置GLFW窗口创建提示
 * @description 配置OpenGL版本和窗口属性
 */
void UIWindow::setWindowHints() {
    // 设置OpenGL版本为3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    // 考虑添加多重采样抗锯齿
    glfwWindowHint(GLFW_SAMPLES, 4);
    // 启用透明帧缓冲区，支持窗口透明效果
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    
    // 创建时隐藏窗口，避免闪烁
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    

}

/**
 * @brief 检查窗口是否应该关闭
 * @return bool 应该关闭返回true
 */
bool UIWindow::shouldClose() const {
    return window ? glfwWindowShouldClose(window) : true;
}

/**
 * @brief 交换前后缓冲区
 * @description 将后缓冲区的渲染结果显示到前缓冲区（屏幕）
 */
void UIWindow::swapBuffers() {
    if (window) glfwSwapBuffers(window);
}

/**
 * @brief 轮询并处理窗口事件
 * @description 处理键盘、鼠标、窗口大小变化等事件
 */
void UIWindow::pollEvents() {
    glfwPollEvents();
}

/**
 * @brief 设置窗口透明度
 * @param opacity 透明度值，0.0为完全透明，1.0为完全不透明
 */
void UIWindow::setWindowOpacity(float opacity) {
    if (window) glfwSetWindowOpacity(window, opacity);
}

/**
 * @brief 设置透明帧缓冲区属性
 * @param transparent 是否启用透明帧缓冲区
 * @note 此函数必须在窗口创建前调用才有效
 */
void UIWindow::setTransparentFramebuffer(bool transparent) {
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, transparent ? GLFW_TRUE : GLFW_FALSE);
}

/**
 * @brief 获取帧缓冲区尺寸
 * @param width 输出参数，帧缓冲区宽度（像素）
 * @param height 输出参数，帧缓冲区高度（像素）
 * @note 帧缓冲区尺寸可能与窗口尺寸不同（高DPI显示器）
 */
void UIWindow::getFramebufferSize(int& width, int& height) const {
    if (window) glfwGetFramebufferSize(window, &width, &height);
}

/**
 * @brief 获取鼠标光标在窗口中的位置
 * @param x 输出参数，光标X坐标（相对于窗口左上角）
 * @param y 输出参数，光标Y坐标（相对于窗口左上角）
 */
void UIWindow::getCursorPos(double& x, double& y) const {
    if (window) glfwGetCursorPos(window, &x, &y);
}

/**
 * @brief 检查指定鼠标按键是否处于按下状态
 * @param button 鼠标按键代码（如GLFW_MOUSE_BUTTON_LEFT）
 * @return bool 按键被按下返回true，否则返回false
 */
bool UIWindow::isMouseButtonPressed(int button) const {
    return window ? glfwGetMouseButton(window, button) == GLFW_PRESS : false;
}

/**
 * @brief 检查指定键盘按键是否处于按下状态
 * @param key 键盘按键代码（如GLFW_KEY_SPACE）
 * @return bool 按键被按下返回true，否则返回false
 */
bool UIWindow::isKeyPressed(int key) const {
    return window ? glfwGetKey(window, key) == GLFW_PRESS : false;
}

/**
 * @brief 开始新的渲染帧
 * @description 设置OpenGL视口并开始NanoVG渲染帧
 */
void UIWindow::beginFrame() {
    if (!vg) return; // 确保NanoVG上下文有效
    
    // 获取当前帧缓冲区尺寸
    int width, height;
    getFramebufferSize(width, height);
    
    // 设置OpenGL视口为整个帧缓冲区
    glViewport(0, 0, width, height);
    
    // 开始NanoVG渲染帧
    // 参数：上下文、宽度、高度、设备像素比
    nvgBeginFrame(vg, width, height, 1.0f);
}

/**
 * @brief 结束当前渲染帧
 * @description 完成NanoVG渲染并提交绘制命令
 */
void UIWindow::endFrame() {
    if (vg) nvgEndFrame(vg);
}

/**
 * @brief 清除背景颜色
 * @param r 红色分量（0.0-1.0）
 * @param g 绿色分量（0.0-1.0）
 * @param b 蓝色分量（0.0-1.0）
 * @param a 透明度分量（0.0-1.0）
 * @description 清除颜色缓冲区、深度缓冲区和模板缓冲区
 */
void UIWindow::clearBackground(float r, float g, float b, float a) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT); // 只清除颜色缓冲区
}

// ==================== 事件回调函数设置 ====================

/**
 * @brief 设置键盘事件回调函数
 * @param callback 回调函数，接收键盘事件参数
 */
void UIWindow::setKeyCallback(std::function<void(int, int, int, int)> callback) {
    keyCallback = callback;
    if (window) glfwSetKeyCallback(window, keyCallbackWrapper);
}

/**
 * @brief 设置鼠标按键事件回调函数
 * @param callback 回调函数，接收鼠标按键事件参数
 */
void UIWindow::setMouseButtonCallback(std::function<void(int, int, int)> callback) {
    mouseButtonCallback = callback;
    if (window) glfwSetMouseButtonCallback(window, mouseButtonCallbackWrapper);
}

/**
 * @brief 设置鼠标移动事件回调函数
 * @param callback 回调函数，接收鼠标位置参数
 */
// 修改现有的 setCursorPosCallback 方法
void UIWindow::setCursorPosCallback(std::function<void(double, double)> callback) {
    cursorPosCallback = callback;
    if (window) {
        glfwSetCursorPosCallback(window, cursorPosCallbackWrapper);
    }
}

// 修改 cursorPosCallbackWrapper 方法
void UIWindow::cursorPosCallbackWrapper(GLFWwindow* window, double xpos, double ypos) {
    UIWindow* uiWindow = static_cast<UIWindow*>(glfwGetWindowUserPointer(window));
    if (uiWindow) {
        // 先处理动态标题栏
        if (uiWindow->dynamicTitleBarEnabled) {
            uiWindow->handleTitleBarToggle(xpos, ypos);
        }
        
        // 然后调用用户设置的回调
        if (uiWindow->cursorPosCallback) {
            uiWindow->cursorPosCallback(xpos, ypos);
        }
    }
}

/**
 * @brief 设置窗口大小变化事件回调函数
 * @param callback 回调函数，接收新的窗口尺寸参数
 */
void UIWindow::setWindowSizeCallback(std::function<void(int, int)> callback) {
    windowSizeCallback = callback;
    if (window) glfwSetWindowSizeCallback(window, windowSizeCallbackWrapper);
}

// ==================== 静态回调函数包装器实现 ====================
// 这些函数将GLFW的C风格回调转换为C++成员函数调用

/**
 * @brief 键盘事件静态回调包装器
 * @description 从GLFW窗口用户指针获取UIWindow实例，并调用其键盘回调
 */
void UIWindow::keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods) {
    UIWindow* uiWindow = static_cast<UIWindow*>(glfwGetWindowUserPointer(window));
    if (uiWindow) {
        // 处理 F 全屏切换
        // if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        //     uiWindow->toggleFullscreen();
        // }
        
        // 调用用户设置的回调
        if (uiWindow->keyCallback) {
            uiWindow->keyCallback(key, scancode, action, mods);
        }
    }
}

/**
 * @brief 鼠标按键事件静态回调包装器
 * @description 从GLFW窗口用户指针获取UIWindow实例，并调用其鼠标按键回调
 */
void UIWindow::mouseButtonCallbackWrapper(GLFWwindow* window, int button, int action, int mods) {
    UIWindow* uiWindow = static_cast<UIWindow*>(glfwGetWindowUserPointer(window));
    if (uiWindow && uiWindow->mouseButtonCallback) {
        uiWindow->mouseButtonCallback(button, action, mods);
    }
}

/**
 * @brief 鼠标移动事件静态回调包装器
 * @description 从GLFW窗口用户指针获取UIWindow实例，并调用其鼠标移动回调
 */
// 删除这个重复的函数定义（第361-367行）
// void UIWindow::cursorPosCallbackWrapper(GLFWwindow* window, double xpos, double ypos) {
//     UIWindow* uiWindow = static_cast<UIWindow*>(glfwGetWindowUserPointer(window));
//     if (uiWindow && uiWindow->cursorPosCallback) {
//         uiWindow->cursorPosCallback(xpos, ypos);
//     }
// }

/**
 * @brief 窗口大小变化事件静态回调包装器
 * @description 从GLFW窗口用户指针获取UIWindow实例，并调用其窗口大小变化回调
 */
void UIWindow::windowSizeCallbackWrapper(GLFWwindow* window, int width, int height) {
    UIWindow* uiWindow = static_cast<UIWindow*>(glfwGetWindowUserPointer(window));
    if (uiWindow && uiWindow->windowSizeCallback) {
        uiWindow->windowSizeCallback(width, height);
    }
}

/**
 * @brief 设置滚轮事件回调函数
 * @param callback 回调函数，接收滚轮事件参数
 */
void UIWindow::setScrollCallback(std::function<void(double, double)> callback) {
    scrollCallback = callback;
    if (window) glfwSetScrollCallback(window, scrollCallbackWrapper);
}

/**
 * @brief 设置字符输入事件回调函数
 * @param callback 回调函数，接收字符输入参数
 */
void UIWindow::setCharCallback(std::function<void(unsigned int)> callback) {
    charCallback = callback;
    if (window) glfwSetCharCallback(window, charCallbackWrapper);
}

/**
 * @brief 字符输入事件静态回调包装器
 * @description 从GLFW窗口用户指针获取UIWindow实例，并调用其字符输入回调
 */
void UIWindow::charCallbackWrapper(GLFWwindow* window, unsigned int codepoint) {
    UIWindow* uiWindow = static_cast<UIWindow*>(glfwGetWindowUserPointer(window));
    if (uiWindow && uiWindow->charCallback) {
        uiWindow->charCallback(codepoint);
    }
}

/**
 * @brief 滚轮事件静态回调包装器
 * @param window GLFW窗口句柄
 * @param xoffset 水平滚动偏移量
 * @param yoffset 垂直滚动偏移量
 */
void UIWindow::scrollCallbackWrapper(GLFWwindow* window, double xoffset, double yoffset) {
    UIWindow* uiWindow = static_cast<UIWindow*>(glfwGetWindowUserPointer(window));
    if (uiWindow && uiWindow->scrollCallback) {
        uiWindow->scrollCallback(xoffset, yoffset);
    }
}


// ==================== 动态标题栏功能实现 ====================

void UIWindow::enableDynamicTitleBar(bool enable, double threshold) {
    dynamicTitleBarEnabled = enable;
    showThreshold = threshold;
    
    if (enable) {
        // 设置鼠标移动回调来处理标题栏切换
        glfwSetCursorPosCallback(window, cursorPosCallbackWrapper);
    }
}

void UIWindow::showTitleBar() {
    if (!titleBarVisible) {
#ifdef _WIN32
        HWND hwnd = glfwGetWin32Window(window);
        if (hwnd) {
            // 获取当前窗口样式
            LONG style = GetWindowLong(hwnd, GWL_STYLE);
            // 添加标题栏样式
            style |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
            SetWindowLong(hwnd, GWL_STYLE, style);
            
            // 重新设置窗口位置以应用样式变化
            SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
                        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
            
            titleBarVisible = true;
        }
#elif defined(__linux__)
        // // Linux 平台使用 X11 窗口管理器提示
        // GLFWwindow* window = getGLFWWindow();
        if (window) {
        //     glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
            glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
            titleBarVisible = true;
        }
#endif
    }
}

void UIWindow::hideTitleBar() {
    if (titleBarVisible) {
    #ifdef _WIN32
            HWND hwnd = glfwGetWin32Window(window);
            if (hwnd) {
                // 获取当前窗口样式
                LONG style = GetWindowLong(hwnd, GWL_STYLE);
                // 移除标题栏样式
                style &= ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
                SetWindowLong(hwnd, GWL_STYLE, style);
                
                // 重新设置窗口位置以应用样式变化
                SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
                            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
                
                titleBarVisible = false;
            }
    #elif defined(__linux__)
            GLFWwindow* window = getGLFWWindow();
            if (window) {
                // // 设置窗口为无装饰
                // glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
                
         
                glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
                titleBarVisible = false;
            }
    #endif
    }
}

void UIWindow::handleTitleBarToggle(double xpos, double ypos) {
    if (!dynamicTitleBarEnabled) return;
    
    // 如果鼠标在顶部阈值范围内，显示标题栏
    if (ypos <= showThreshold) {
        showTitleBar();
    } else {
        hideTitleBar();
    }
}

void UIWindow::toggleFullscreen() {
    setFullscreen(!fullscreenMode);
}

void UIWindow::setFullscreen(bool fullscreen) {
    if (fullscreenMode == fullscreen) return;
    
    if (fullscreen) {
        // 保存当前窗口信息以便恢复
        glfwGetWindowPos(window, &windowedModeInfo.x, &windowedModeInfo.y);
        glfwGetWindowSize(window, &windowedModeInfo.width, &windowedModeInfo.height);
        
        // 获取主显示器
        GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
        if (primaryMonitor) {
            // 获取显示器视频模式
            const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
            // 设置全屏
            glfwSetWindowMonitor(window, primaryMonitor, 0, 0,
                                mode->width, mode->height,
                                mode->refreshRate);
            // 自动隐藏标题栏
            hideTitleBar();
        }
    } else {
        // 恢复窗口模式
        glfwSetWindowMonitor(window, nullptr,
                            windowedModeInfo.x, windowedModeInfo.y,
                            windowedModeInfo.width, windowedModeInfo.height,
                            GLFW_DONT_CARE);
        // 如果启用了动态标题栏，让它自动处理显示/隐藏
        if (!dynamicTitleBarEnabled) {
            showTitleBar();
        }
    }
    
    fullscreenMode = fullscreen;
}
