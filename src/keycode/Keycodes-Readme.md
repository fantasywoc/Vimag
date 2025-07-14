### 🛠️ **使用说明**

#### 1. **基础调用**

```cpp
#include "keycodes.h"

void handleKeyEvent(int rawKeyCode) {
    using namespace PlatformKey;

    switch(rawKeyCode) {
        case KEY_ENTER:
            // 处理回车键
            break;
        case KEY_ESCAPE:
            // 处理ESC键
            break;
        case KEY_W:  // 直接使用字母键
            moveForward();
            break;
        #if defined(_WIN32)
        case KEY_WIN_LEFT:  // Windows专属键
            openStartMenu();
            break;
        #endif
    }
}
```

#### 2. **修饰键组合检测**

```cpp
bool isCtrlPressed() {
    // 需调用平台API实现
    #ifdef _WIN32
    return (GetAsyncKeyState(PlatformKey::KEY_CONTROL) & 0x8000);
    #elif __linux__
    // Linux下通过Xlib或evdev检测
    #endif
}

// 检测Ctrl+S组合键
if (rawKeyCode == PlatformKey::KEY_S && isCtrlPressed()) {
    saveFile();
}
```

#### 3. **键码转换工具（可选扩展）**

在头文件中添加转换函数：

```cpp
/**
 * 将原始键码转换为通用键码（需在各平台实现）
 * @param rawKey 系统原始键码
 * @return 标准KeyCode值，未知键返回-1
 */
int toStandardKeyCode(int rawKey) {
    #if defined(_WIN32)
    // Windows虚拟键码转换逻辑
    #elif defined(__linux__)
    // Linux X11键码转换逻辑
    #endif
}
```

------

### ⚠️ **注意事项**

1. **平台限制**

   - Linux 需安装 `libx11-dev` 或 `libevdev-dev` 库处理原始键码

   - Windows 键值仅适用于 Windows 平台（通过#ifdef _WIN32隔离）

     

2. **不可用键码**

   - 媒体键（音量/播放）因系统差异较大未包含
   - 小键盘键码因 Num Lock 状态行为不同需单独处理

3. **输入法兼容性**
   此键码仅代表**物理按键位置**，字符输入请用 `text` 字段