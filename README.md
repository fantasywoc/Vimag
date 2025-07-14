





# UI Window Project

这是一个基于 GLFW、OpenGL 和 NanoVG 的 UI 窗口项目，提供了一个封装良好的 `UIWindow` 类来简化窗口和图形界面的开发。





# Debug模式
xmake config -m debug

# Release模式
xmake config -m release

# 清理构建
xmake clean

# 编译
xmake g --pkg_searchdirs=./src/thrity_party/

xmake 
xmake run vimag-demo


## UIWindow 类的主要功能

### 1. 窗口管理
- 创建、初始化、清理GLFW窗口
- 窗口生命周期管理
- 自动资源清理

### 2. 渲染上下文
- 管理OpenGL和NanoVG上下文
- 自动初始化GLEW
- 提供统一的渲染环境

### 3. 事件处理
- 封装鼠标、键盘事件处理
- 简化输入状态查询
- 支持光标位置获取

### 4. 渲染循环
- 提供便捷的渲染开始/结束方法
- 自动处理帧缓冲区管理
- 简化渲染流程

### 5. 回调系统
- 支持自定义事件回调函数
- 键盘、鼠标、窗口大小变化回调
- 类型安全的回调接口

### 6. 窗口属性
- 透明度、大小等属性设置
- 透明帧缓冲区支持
- 灵活的窗口配置选项

## 项目结构

```
UI/
├── src/
│   ├── UIWindow.h      # UIWindow 类头文件
│   ├── UIWindow.cpp    # UIWindow 类实现
│   └── main.cpp        # 示例程序
├── xmake.lua           # 构建配置
└── README.md           # 项目说明
```

## 依赖库

- **GLFW 3.3.8**: 窗口管理和输入处理
- **GLEW 1.13.0**: OpenGL 扩展加载
- **NanoVG**: 2D 矢量图形渲染

## 构建方法

使用 xmake 构建项目：

```bash
xmake
```

运行程序：

```bash
xmake run
```

## 使用示例

```cpp
#include "UIWindow.h"

int main() {
    // 创建窗口
    UIWindow window(800, 600, "My UI App");
    
    // 初始化
    if (!window.initialize()) {
        return -1;
    }
    
    // 设置透明度
    window.setWindowOpacity(0.8f);
    
    // 主循环
    while (!window.shouldClose()) {
        window.clearBackground();
        window.beginFrame();
        
        // 在这里添加你的渲染代码
        
        window.endFrame();
        window.swapBuffers();
        window.pollEvents();
    }
    
    return 0;
}
```

## 特性

- ✅ 跨平台支持（Windows/Linux）
- ✅ 透明窗口支持
- ✅ 事件回调系统
- ✅ 简洁的 API 设计
- ✅ 自动资源管理
- ✅ NanoVG 集成

## 许可证

本项目采用开源许可证，具体请查看 LICENSE 文件。

## 性能优化

### 常见的CPU和GPU占用高的问题

#### 问题原因
1. **无限制的渲染循环**: 主循环以最大速度运行，没有帧率限制
2. **缺少垂直同步**: 未启用VSync，导致GPU过度渲染
3. **每帧清除所有缓冲区**: 不必要的缓冲区清除操作
4. **事件处理效率低**: 频繁的事件轮询和处理

#### 解决方案

##### 1. 启用垂直同步 (VSync)
在 `UIWindow.cpp` 的 `initialize()` 方法中添加：
```cpp
// 启用垂直同步，限制帧率到显示器刷新率
glfwSwapInterval(1);
```
 

##### 2. 添加帧率控制
在主循环中添加帧率限制：
```cpp
#include <thread>
#include <chrono>

// 目标帧率 (60 FPS)
const double targetFrameTime = 1.0 / 60.0;
double lastTime = glfwGetTime();

while (!window.shouldClose()) {
    double currentTime = glfwGetTime();
    double deltaTime = currentTime - lastTime;
    
    if (deltaTime >= targetFrameTime) {
        // 渲染和更新逻辑
        window.clearBackground();
        window.beginFrame();
        // ... 渲染代码 ...
        window.endFrame();
        window.swapBuffers();
        
        lastTime = currentTime;
    } else {
        // 休眠剩余时间
        auto sleepTime = std::chrono::duration<double>(targetFrameTime - deltaTime);
        std::this_thread::sleep_for(sleepTime);
    }
    
    window.pollEvents();
}
```

##### 3. 优化缓冲区清除
在 `UIWindow.cpp` 的 `clearBackground()` 方法中：
```cpp
void UIWindow::clearBackground() {
    // 只清除颜色缓冲区，避免不必要的深度和模板缓冲区清除
    glClear(GL_COLOR_BUFFER_BIT);
}
```

##### 4. 事件处理优化
- 使用 `glfwWaitEvents()` 替代 `glfwPollEvents()` 在不需要连续渲染时
- 实现脏矩形更新，只在需要时重绘
- 合理使用事件回调，避免频繁的状态查询

### 性能监控

可以添加简单的性能监控来跟踪帧率：
```cpp
// 帧率计算
static int frameCount = 0;
static double lastFPSTime = glfwGetTime();

frameCount++;
double currentTime = glfwGetTime();
if (currentTime - lastFPSTime >= 1.0) {
    double fps = frameCount / (currentTime - lastFPSTime);
    std::cout << "FPS: " << fps << std::endl;
    frameCount = 0;
    lastFPSTime = currentTime;
}
```

### 推荐配置

对于大多数应用，推荐以下配置：
- 启用VSync (`glfwSwapInterval(1)`)
- 限制帧率到60 FPS
- 只在UI有变化时重绘
- 使用适当的事件处理策略

这些优化可以显著降低CPU和GPU的使用率，同时保持流畅的用户体验。


# 控件布局
根据FlexLayout的实现，控件间距和边距是通过以下两个参数控制的：

## 控件间距和边距控制

### 1. **Spacing（间距）**
- **作用**：控制相邻子控件之间的距离
- **参数**：`spacing`（float类型）
- **应用**：在水平布局中控制左右相邻控件的间距，在垂直布局中控制上下相邻控件的间距

### 2. **Padding（内边距）**
- **作用**：控制第一个子控件与父控件边缘的距离，以及最后一个子控件与父控件边缘的距离
- **参数**：`padding`（float类型）
- **应用**：在所有方向上为容器内容提供内边距

## 当前代码中的使用示例

在 <mcfile name="main.cpp" path="e:\Program\Learning-progect\UI\src\main.cpp"></mcfile> 中：

```cpp
// 主面板：水平布局，间距20px，内边距15px
mainPanel->setHorizontalLayout(FlexLayout::START, 20.0f, 15.0f);

// 左面板：垂直布局，间距10px，内边距10px
leftPanel->setVerticalLayout(FlexLayout::START, 10.0f, 10.0f);
```

## 布局方法签名

```cpp
void setHorizontalLayout(FlexLayout::Alignment alignment, float spacing, float padding);
void setVerticalLayout(FlexLayout::Alignment alignment, float spacing, float padding);
```

## 参数详解

### Spacing（第二个参数）
- **水平布局**：控制左右相邻控件的水平间距
- **垂直布局**：控制上下相邻控件的垂直间距
- **计算方式**：`总间距 = (子控件数量 - 1) × spacing`

### Padding（第三个参数）
- **作用范围**：四个方向都应用相同的内边距
- **计算方式**：
  - 可用宽度 = `容器宽度 - 2 × padding`
  - 可用高度 = `容器高度 - 2 × padding`
  - 起始位置 = `容器位置 + padding`

## 实际效果示例

以左面板为例（垂直布局，spacing=10px，padding=10px）：
```
┌─────────────────────────────────┐
│ ↑ 10px padding                 │
│ ┌─────────────────────────────┐ │
│ │ Label                       │ │
│ └─────────────────────────────┘ │
│ ↕ 10px spacing                  │
│ ┌─────────────────────────────┐ │
│ │ Button 1                    │ │
│ └─────────────────────────────┘ │
│ ↕ 10px spacing                  │
│ ┌─────────────────────────────┐ │
│ │ Button 2                    │ │
│ └─────────────────────────────┘ │
│ ↕ 10px spacing                  │
│ ┌─────────────────────────────┐ │
│ │ Button 3                    │ │
│ └─────────────────────────────┘ │
│ ↓ 10px padding                 │
└─────────────────────────────────┘
```

## 如何调整

如果你想要修改间距和边距，可以调用相应的设置方法：

```cpp
// 增大控件间距到20px，内边距到15px
leftPanel->setVerticalLayout(FlexLayout::START, 20.0f, 15.0f);

// 或者单独设置
auto layout = leftPanel->getFlexLayout();
if (layout) {
    layout->setSpacing(20.0f);  // 设置间距
    layout->setPadding(15.0f);  // 设置内边距
    leftPanel->updateLayout();  // 更新布局
}
```
        



## 已知问题与解决方案

### 动画系统崩溃问题

**问题描述**：
在旋转动画播放过程中点击其他带有动画的按钮会导致程序崩溃，错误代码为 `-1073741819`（访问违规）。

**崩溃原因**：
1. **容器修改冲突**：`UIAnimationManager::update()` 方法在遍历动画列表时，其他操作（如 `removeAnimation`）同时修改了该列表，导致迭代器失效
2. **动画生命周期问题**：在同一组件上创建多个动画时，新动画的添加可能与正在执行的动画产生冲突
3. **回调函数时序问题**：动画完成回调可能在动画被移除后仍然被触发

**解决方案**：

#### 方案一：延迟移除机制（已实施）
在 `UIAnimationManager` 中添加延迟移除机制，避免在遍历过程中直接修改容器：

```cpp
// 在 UIAnimationManager.h 中添加成员变量
private:
    bool m_isUpdating = false;
    std::vector<UIComponent*> m_pendingRemovals;

// 在 UIAnimationManager.cpp 中修改 removeAnimation 和 update 方法
void UIAnimationManager::removeAnimation(UIComponent* target) {
    if (!target) return;
    
    if (m_isUpdating) {
        m_pendingRemovals.push_back(target);
        return;
    }
    
    // 立即移除逻辑...
}

void UIAnimationManager::update(double deltaTime) {
    m_isUpdating = true;
    
    // 更新动画...
    
    m_isUpdating = false;
    
    // 处理延迟移除...
}
```



### UI事件处理问题
问题描述 ：
在动态布局更新后，某些面板内的按钮无法响应鼠标点击事件，特别是在执行布局变更操作（如显示/隐藏面板）后。

问题原因 ：

1. 坐标系统不同步 ：布局更新后，面板位置发生变化，但事件处理系统使用的仍是旧的位置信息
2. 事件传递链断裂 ： UIPanel::handleEvent 将全局坐标转换为相对坐标传递给子组件，但转换基于的面板位置可能已过时
3. 位置缓存问题 ：组件的位置信息没有在布局更新后及时同步到事件处理系统
典型症状 ：

- 布局更新前按钮可以正常点击
- 布局更新后按钮失去响应
- 调试输出显示按钮位置发生了显著变化（如坐标偏移250像素）
- 其他未受布局影响的按钮仍然正常工作
解决方案 ：
 推荐方案：实时布局同步
在 UIPanel.cpp 的 handleEvent 方法中，处理鼠标事件前强制更新布局：

```
bool UIPanel::handleEvent(const UIEvent& event) {
    if (!m_visible || !m_enabled) {
        return false;
    }
    
    // 在处理鼠标事件前强制更新布局，确保位置信息是最新的
    if (event.type == UIEvent::MOUSE_PRESS || 
        event.type == UIEvent::MOUSE_RELEASE || 
        event.type == UIEvent::MOUSE_MOVE) {
        updateLayout();
    }
    
    // ... 现有的事件处理逻辑 ...
    if (event.type == UIEvent::MOUSE_PRESS || 
        event.type == UIEvent::MOUSE_RELEASE || 
        event.type == UIEvent::MOUSE_MOVE) {
        
        // 计算相对于面板的坐标
        UIEvent localEvent = event;
        localEvent.mouseX = event.mouseX - (m_x + 
        m_animationOffsetX);
        localEvent.mouseY = event.mouseY - (m_y + 
        m_animationOffsetY);
        
        // 从后往前遍历子组件（后添加的在上层）
        for (auto it = m_children.rbegin(); it != m_children.rend
        (); ++it) {
            if ((*it)->handleEvent(localEvent)) {
                return true;
            }
        }
    }
    
    return false;
}
```
备选方案：主动位置同步
在布局更新操作后，主动调用位置同步：

```
// 在布局更新后添加
mainPanel->updateLayout();
mainPanel->resetAllAnimationOffsets();

// 强制同步所有组件位置
rightPanel->setPosition(rightPanel->getX(), rightPanel->getY());
okButton->setPosition(okButton->getX(), okButton->getY());
```
验证方法 ：

1. 执行布局更新操作（如显示/隐藏面板）
2. 观察调试输出中的位置变化信息
3. 测试受影响面板中按钮的点击响应
4. 确认坐标转换的准确性
预防措施 ：

- 在所有可能改变布局的操作后调用 updateLayout()
- 使用一致的坐标系统进行事件处理
- 添加调试输出来监控位置变化
- 定期验证事件处理链的完整性
这个解决方案确保了事件处理系统始终使用最新的组件位置信息，避免了布局更新后的坐标不同步问题。

#clangd 配置xmake 
##clangd安装
(以 VSCode 为例)安装 ​**vscode-clangd**​ 插件（在扩展商店搜索安装）
安装->https://github.com/llvm/llvm-project/releases ,bin路径添加到环境变量
##配置xmake(​核心配置步骤​)
1. ​生成 compile_commands.json（编译数据库）​​

​自动更新​：在 xmake.lua 中添加规则：
```lua
add_rules("plugin.compile_commands.autoupdate", { outputdir = ".vscode" })
```
​手动生成​：在项目根目录执行：
```bash
xmake project -k compile_commands
```
默认生成在项目根目录，可通过 --outputdir=.vscode 指定输出到 .vscode 目录。
每次编译时自动更新该文件。
2. ​ 配置 VS Code 的 Clangd 插件​
在 .vscode/settings.json 中指定编译数据库路径：

```json
{
  "clangd.arguments": [
    "--compile-commands-dir=${workspaceFolder}/.vscode",  // 指向 compile_commands.json 所在目录
    "--header-insertion=never",                           // 禁止自动插入头文件
    "--completion-style=detailed"                         // 启用详细补全提示
  ]
}
```
若文件在根目录，则路径改为 ${workspaceFolder}

#VIMAG
##格式支持
   - 支持多种图像格式（JPG/JPEG/PNG/BMP/GIF/TGA/HDR/PSD）

#支持高刷
 - 帧率（60FPS/120FPS）

#快捷键
[鼠标]
- 滚轮：缩放
- 左键拖拽：移动
- 左键双击：恢复移动和缩放
- 左键单击+滚轮||左右方向键：切换图片
- 中键||"F"：窗口最大化/最小化

依赖OPENGL
