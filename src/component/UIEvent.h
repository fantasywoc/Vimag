struct UIEvent {
    enum Type {
        MOUSE_MOVE,
        MOUSE_PRESS,
        MOUSE_RELEASE,
        MOUSE_SCROLL,      // 添加滚轮事件
        MOUSE_DOUBLE_CLICK, // 添加双击事件
        KEY_PRESS,
        KEY_RELEASE,
        CHAR_INPUT  // 添加字符输入事件
    };
    
    Type type;
    double mouseX, mouseY;
    int mouseButton;
    double scrollX, scrollY;  // 添加滚轮滚动量
    double clickTime;         // 添加点击时间戳，用于双击检测
    int keyCode;
    int modifiers;
    unsigned int character;  // 添加字符字段
};