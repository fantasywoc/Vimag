// keycodes.h
// 跨平台键盘键码定义（Windows/Linux 通用）


#pragma once

#if defined(_WIN32) || defined(__linux__)

// namespace PlatformKey {
    /**
     * 跨平台通用键盘键码枚举
     * 物理按键位置在 Windows/Linux 上保持一致
     */
    enum KeyCode {
        // 基础控制键
        KEY_BACKSPACE   = 8,
        KEY_TAB         = 9,
        KEY_ENTER       = 13,
        KEY_SHIFT       = 16,
        KEY_CONTROL     = 17,
        KEY_ALT         = 18,
        KEY_ESCAPE      = 27,
        KEY_SPACE       = 32,

        // 方向键
        KEY_LEFT        = 37,
        KEY_UP          = 38,
        KEY_RIGHT       = 39,
        KEY_DOWN        = 40,

        // 编辑键
        KEY_INSERT      = 45,
        KEY_DELETE      = 46,
        KEY_HOME        = 36,
        KEY_END         = 35,
        KEY_PAGE_UP     = 33,
        KEY_PAGE_DOWN   = 34,

        // 字母键（A-Z）
        KEY_A = 65, KEY_B = 66, KEY_C = 67, KEY_D = 68, KEY_E = 69,
        KEY_F = 70, KEY_G = 71, KEY_H = 72, KEY_I = 73, KEY_J = 74,
        KEY_K = 75, KEY_L = 76, KEY_M = 77, KEY_N = 78, KEY_O = 79,
        KEY_P = 80, KEY_Q = 81, KEY_R = 82, KEY_S = 83, KEY_T = 84,
        KEY_U = 85, KEY_V = 86, KEY_W = 87, KEY_X = 88, KEY_Y = 89,
        KEY_Z = 90,

        // 数字键（主键盘）
        KEY_0 = 48, KEY_1 = 49, KEY_2 = 50, KEY_3 = 51, KEY_4 = 52,
        KEY_5 = 53, KEY_6 = 54, KEY_7 = 55, KEY_8 = 56, KEY_9 = 57,

        // 功能键（F1-F12）
        KEY_F1 = 112, KEY_F2 = 113, KEY_F3 = 114, KEY_F4 = 115,
        KEY_F5 = 116, KEY_F6 = 117, KEY_F7 = 118, KEY_F8 = 119,
        KEY_F9 = 120, KEY_F10 = 121, KEY_F11 = 122, KEY_F12 = 123,

        // 符号键（物理位置一致）
        KEY_EQUAL       = 187,   // =
        KEY_MINUS       = 189,   // -
        KEY_COMMA       = 188,   // ,
        KEY_PERIOD      = 190,   // .
        KEY_SLASH       = 191,   // /
        KEY_BACKSLASH   = 220,   // 
        KEY_SEMICOLON   = 186,   // ;
        KEY_APOSTROPHE  = 222,   // '
        KEY_LBRACKET    = 219,   // [
        KEY_RBRACKET    = 221,   // ]
        KEY_GRAVE       = 192    // `
    };

    #if defined(_WIN32)
    /**
     * Windows 专属键码扩展
     */
    enum WindowsKey {
        KEY_WIN_LEFT    = 91,    // 左Windows键
        KEY_WIN_RIGHT   = 92,    // 右Windows键
        KEY_APPS        = 93     // 应用菜单键
    };
    #endif
// } // namespace PlatformKey

#endif // _WIN32 || __linux__

