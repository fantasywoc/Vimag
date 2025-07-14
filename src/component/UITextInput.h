#pragma once
#include "UIComponent.h"
#include <functional>
#include <chrono>

/**
 * @class UITextInput
 * @brief 文本输入框控件类
 * @description 支持文字输入、光标显示、选择文本等功能的输入组件
 */
class UITextInput : public UIComponent {
public:
    // 构造函数
    UITextInput(float x, float y, float width, float height, const std::string& placeholder = "");
    
    // 重写基类虚函数
    void render(NVGcontext* vg) override;
    void update(double deltaTime) override;
    bool handleEvent(const UIEvent& event) override;
    
    // 文本输入特有接口
    void setText(const std::string& text);
    const std::string& getText() const { return m_text; }
    
    void setPlaceholder(const std::string& placeholder) { m_placeholder = placeholder; }
    const std::string& getPlaceholder() const { return m_placeholder; }
    
    void setMaxLength(size_t maxLength) { m_maxLength = maxLength; }
    size_t getMaxLength() const { return m_maxLength; }
    
    void setReadOnly(bool readOnly) { m_readOnly = readOnly; }
    bool isReadOnly() const { return m_readOnly; }
    
    void setPassword(bool isPassword) { m_isPassword = isPassword; }
    bool isPassword() const { return m_isPassword; }
    
    // 焦点管理
    void setFocused(bool focused);
    bool isFocused() const { return m_isFocused; }
    
    // 回调函数
    void setOnTextChanged(std::function<void(const std::string&)> callback) { m_onTextChanged = callback; }
    void setOnEnterPressed(std::function<void(const std::string&)> callback) { m_onEnterPressed = callback; }
    void setOnFocusChanged(std::function<void(bool)> callback) { m_onFocusChanged = callback; }
    
    // 样式设置
    void setTextColor(NVGcolor color) { m_textColor = color; }
    void setPlaceholderColor(NVGcolor color) { m_placeholderColor = color; }
    void setCursorColor(NVGcolor color) { m_cursorColor = color; }
    void setSelectionColor(NVGcolor color) { m_selectionColor = color; }
    void setFocusedBorderColor(NVGcolor color) { m_focusedBorderColor = color; }
    void setFontSize(float size) { m_fontSize = size; }
    void setPadding(float padding) { m_padding = padding; }
    
    // 文本操作
    void selectAll();
    void clearSelection();
    void deleteSelection();
    void insertText(const std::string& text);
    
private:
    // 文本内容
    std::string m_text;
    std::string m_placeholder;
    size_t m_maxLength = 1000;
    bool m_readOnly = false;
    bool m_isPassword = false;
    bool m_isDragging = false;  // 是否正在拖拽选择
    
    // 光标和选择
    size_t m_cursorPos = 0;
    size_t m_selectionStart = 0;
    size_t m_selectionEnd = 0;
    bool m_hasSelection = false;
    
    // 状态
    bool m_isFocused = false;
    bool m_isHovered = false;
    
    // 光标闪烁
    std::chrono::steady_clock::time_point m_lastCursorBlink;
    bool m_cursorVisible = true;
    static constexpr double CURSOR_BLINK_INTERVAL = 0.5; // 秒
    
    // 回调函数
    std::function<void(const std::string&)> m_onTextChanged;
    std::function<void(const std::string&)> m_onEnterPressed;
    std::function<void(bool)> m_onFocusChanged;
    
    // 样式
    NVGcolor m_textColor = nvgRGB(255, 255, 255);
    NVGcolor m_placeholderColor = nvgRGB(128, 128, 128);
    NVGcolor m_cursorColor = nvgRGB(255, 255, 255);
    NVGcolor m_selectionColor = nvgRGBA(100, 150, 200, 100);
    NVGcolor m_focusedBorderColor = nvgRGB(100, 150, 200);
    float m_fontSize = 16.0f;
    float m_padding = 8.0f;
    
    // 辅助方法
    void renderText(NVGcontext* vg);
    void renderCursor(NVGcontext* vg);
    void renderSelection(NVGcontext* vg);
    void updateCursorBlink();
    size_t getCharIndexAtPosition(NVGcontext* vg, float x);
    float getCharPositionX(NVGcontext* vg, size_t index);
    void moveCursor(int delta, bool selecting = false);
    void handleKeyInput(int key, bool shift, bool ctrl);
    void handleCharInput(unsigned int codepoint);
    std::string getDisplayText() const;
};