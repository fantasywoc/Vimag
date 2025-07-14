/**
 * @file UIWindow.h
 * @brief UI窗口管理类的头文件
 * @description 封装了GLFW窗口管理和NanoVG渲染上下文的功能
 * @author UI Library Team
 * @date 2024
 */

#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "nanovg.h"
// 移除这行：#include "nanovg_gl.h"
#include <functional>
#include <string>

// #include "stb_image.h"


/**
 * @class UIWindow
 * @brief UI窗口管理类
 * @description 提供窗口创建、事件处理、渲染管理等功能的封装类
 * 
 * 主要功能：
 * - GLFW窗口的创建和管理
 * - OpenGL上下文的初始化
 * - NanoVG渲染上下文的管理
 * - 事件回调系统
 * - 窗口属性设置（透明度、大小等）
 */
class UIWindow {
public:
    /**
     * @brief 构造函数
     * @param width 窗口宽度，默认800像素
     * @param height 窗口高度，默认600像素
     * @param title 窗口标题，默认"UI Window"
     */
    UIWindow(int width = 800, int height = 600, const std::string& title = "UI Window");
    
    /**
     * @brief 析构函数
     * @description 自动清理所有资源
     */
    ~UIWindow();

    // ==================== 基本窗口操作 ====================
    
    /**
     * @brief 初始化窗口和渲染上下文
     * @return bool 初始化成功返回true，失败返回false
     * @description 依次初始化GLFW、创建窗口、初始化GLEW和NanoVG
     */
    bool initialize();
    
    /**
     * @brief 清理所有资源
     * @description 释放NanoVG上下文、销毁GLFW窗口、终止GLFW
     */
    void cleanup();
    
    /**
     * @brief 检查窗口是否应该关闭
     * @return bool 应该关闭返回true，否则返回false
     */
    bool shouldClose() const;
    
    /**
     * @brief 交换前后缓冲区
     * @description 将渲染结果显示到屏幕上
     */
    void swapBuffers();
    
    /**
     * @brief 轮询事件
     * @description 处理窗口事件（键盘、鼠标、窗口大小变化等）
     */
    void pollEvents();

    // ==================== 窗口属性设置 ====================
    
    /**
     * @brief 设置窗口透明度
     * @param opacity 透明度值，范围0.0-1.0（0.0完全透明，1.0完全不透明）
     */
    void setWindowOpacity(float opacity);
    
    /**
     * @brief 设置透明帧缓冲区
     * @param transparent 是否启用透明帧缓冲区
     * @description 必须在窗口创建前调用
     */
    void setTransparentFramebuffer(bool transparent);
    
    /**
     * @brief 设置窗口提示
     * @description 配置OpenGL版本、透明帧缓冲区等窗口属性
     */
    void setWindowHints();

    // ==================== 获取窗口信息 ====================
    
    /**
     * @brief 获取帧缓冲区大小
     * @param width 输出参数，帧缓冲区宽度
     * @param height 输出参数，帧缓冲区高度
     */
    void getFramebufferSize(int& width, int& height) const;
    
    /**
     * @brief 获取鼠标光标位置
     * @param x 输出参数，光标X坐标
     * @param y 输出参数，光标Y坐标
     */
    void getCursorPos(double& x, double& y) const;
    
    /**
     * @brief 检查鼠标按键是否被按下
     * @param button 鼠标按键代码（GLFW_MOUSE_BUTTON_LEFT等）
     * @return bool 按键被按下返回true，否则返回false
     */
    bool isMouseButtonPressed(int button) const;
    
    /**
     * @brief 检查键盘按键是否被按下
     * @param key 键盘按键代码（GLFW_KEY_*系列常量）
     * @return bool 按键被按下返回true，否则返回false
     */
    bool isKeyPressed(int key) const;

    // ==================== 渲染相关 ====================
    
    /**
     * @brief 开始渲染帧
     * @description 设置视口、开始NanoVG渲染帧
     */
    void beginFrame();
    
    /**
     * @brief 结束渲染帧
     * @description 结束NanoVG渲染帧
     */
    void endFrame();
    
    /**
     * @brief 清除背景
     * @param r 红色分量，范围0.0-1.0，默认0.0
     * @param g 绿色分量，范围0.0-1.0，默认0.0
     * @param b 蓝色分量，范围0.0-1.0，默认0.0
     * @param a 透明度分量，范围0.0-1.0，默认0.0
     */
    void clearBackground(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 0.0f);

    // ==================== 上下文访问 ====================
    
    /**
     * @brief 获取NanoVG上下文
     * @return NVGcontext* NanoVG上下文指针
     * @description 用于直接调用NanoVG绘图函数
     */
    NVGcontext* getNVGContext() const { return vg; }
    
    /**
     * @brief 获取GLFW窗口句柄
     * @return GLFWwindow* GLFW窗口指针
     * @description 用于直接调用GLFW函数
     */
    GLFWwindow* getGLFWWindow() const { return window; }

    // ==================== 事件回调设置 ====================
    
    /**
     * @brief 设置键盘事件回调函数
     * @param callback 回调函数，参数为(key, scancode, action, mods)
     */
    void setKeyCallback(std::function<void(int, int, int, int)> callback);
    
    /**
     * @brief 设置鼠标按键事件回调函数
     * @param callback 回调函数，参数为(button, action, mods)
     */
    void setMouseButtonCallback(std::function<void(int, int, int)> callback);
    
    /**
     * @brief 设置鼠标移动事件回调函数
     * @param callback 回调函数，参数为(xpos, ypos)
     */
    void setCursorPosCallback(std::function<void(double, double)> callback);
    
    /**
     * @brief 设置窗口大小变化事件回调函数
     * @param callback 回调函数，参数为(width, height)
     */
    void setWindowSizeCallback(std::function<void(int, int)> callback);

    /**
     * @brief 设置字符输入事件回调函数
     * @param callback 回调函数，参数为(codepoint)
     */
    void setCharCallback(std::function<void(unsigned int)> callback);
    
    /**
     * @brief 设置滚轮事件回调函数
     * @param callback 回调函数，参数为(xoffset, yoffset)
     */
    void setScrollCallback(std::function<void(double, double)> callback);

    /**
     * @brief 设置文件拖拽回调函数
     * @param callback 回调函数，参数为(pathCount, paths)
     */
    void setDropCallback(std::function<void(int, const char**)> callback);

private:
    // ==================== 私有成员变量 ====================
    
    GLFWwindow* window;          ///< GLFW窗口句柄
    NVGcontext* vg;              ///< NanoVG渲染上下文
    int windowWidth, windowHeight; ///< 窗口尺寸
    std::string windowTitle;     ///< 窗口标题
    bool initialized;            ///< 初始化状态标志
    std::function<void(int, const char**)> dropCallback;
    static void dropCallbackWrapper(GLFWwindow* window, int count, const char** paths);
    
    // ==================== 回调函数存储 ====================
    
    std::function<void(int, int, int, int)> keyCallback;        ///< 键盘事件回调
    std::function<void(int, int, int)> mouseButtonCallback;     ///< 鼠标按键回调
    std::function<void(double, double)> cursorPosCallback;      ///< 鼠标移动回调
    std::function<void(int, int)> windowSizeCallback;           ///< 窗口大小变化回调
    // 在 windowSizeCallback 后添加
    std::function<void(unsigned int)> charCallback;           ///< 字符输入回调
    std::function<void(double, double)> scrollCallback;       ///< 滚轮事件回调
    
    // ==================== 静态回调函数包装器 ====================
    // GLFW要求回调函数为静态函数或C风格函数，这些包装器用于转发到成员函数
    
    /**
     * @brief 键盘事件静态回调包装器
     * @param window GLFW窗口句柄
     * @param key 按键代码
     * @param scancode 扫描码
     * @param action 动作（按下/释放/重复）
     * @param mods 修饰键状态
     */
    static void keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
    
    /**
     * @brief 鼠标按键事件静态回调包装器
     * @param window GLFW窗口句柄
     * @param button 鼠标按键代码
     * @param action 动作（按下/释放）
     * @param mods 修饰键状态
     */
    static void mouseButtonCallbackWrapper(GLFWwindow* window, int button, int action, int mods);
    
    /**
     * @brief 鼠标移动事件静态回调包装器
     * @param window GLFW窗口句柄
     * @param xpos 鼠标X坐标
     * @param ypos 鼠标Y坐标
     */
    static void cursorPosCallbackWrapper(GLFWwindow* window, double xpos, double ypos);
    
    /**
     * @brief 窗口大小变化事件静态回调包装器
     * @param window GLFW窗口句柄
     * @param width 新窗口宽度
     * @param height 新窗口高度
     */
    static void windowSizeCallbackWrapper(GLFWwindow* window, int width, int height);
    
    /**
     * @brief 字符输入事件静态回调包装器
     * @param window GLFW窗口句柄
     * @param codepoint Unicode字符代码点
     */
    static void charCallbackWrapper(GLFWwindow* window, unsigned int codepoint);
    
    /**
     * @brief 滚轮事件静态回调包装器
     * @param window GLFW窗口句柄
     * @param xoffset 水平滚动偏移量
     * @param yoffset 垂直滚动偏移量
     */
    static void scrollCallbackWrapper(GLFWwindow* window, double xoffset, double yoffset);

public:
    // ==================== 标题栏控制 ====================
    
    /**
     * @brief 启用动态标题栏功能
     * @param enable 是否启用动态标题栏
     * @param threshold 鼠标距离顶部多少像素时显示标题栏，默认10像素
     */
    void enableDynamicTitleBar(bool enable, double threshold = 10.0);
    
    /**
     * @brief 显示标题栏
     */
    void showTitleBar();
    
    /**
     * @brief 隐藏标题栏
     */
    void hideTitleBar();
    
    /**
     * @brief 检查标题栏是否可见
     * @return bool 标题栏可见返回true
     */
    bool isTitleBarVisible() const { return titleBarVisible; }

private:
    // ==================== 动态标题栏相关 ====================
    bool dynamicTitleBarEnabled = false;  ///< 是否启用动态标题栏
    bool titleBarVisible = true;          ///< 标题栏当前是否可见
    double showThreshold = 10.0;          ///< 显示标题栏的鼠标距离阈值
    
    /**
     * @brief 处理鼠标移动以控制标题栏显示
     * @param xpos 鼠标X坐标
     * @param ypos 鼠标Y坐标
     */
    void handleTitleBarToggle(double xpos, double ypos);
    // 在 UIWindow 类的 public 部分添加
public:
    // 切换全屏状态
    void toggleFullscreen();
    // 设置全屏状态
    void setFullscreen(bool fullscreen);
    // 检查是否全屏
    bool isFullscreen() const { return fullscreenMode; }
    
    private:
    // 在私有成员变量部分添加
    bool fullscreenMode = false;
    // 存储窗口恢复所需的信息
    struct {
        int x, y, width, height;
    } windowedModeInfo;
    
    /**
     * @brief 处理鼠标移动以控制标题栏显示
     * @param xpos 鼠标X坐标
     * @param ypos 鼠标Y坐标
     */
    // private:
    //     // 只保留这一个声明
    //     void handleTitleBarToggle(double xpos, double ypos);
};
