#pragma once
#include "UIComponent.h"
#include <functional>

/**
 * @class UIButton
 * @brief 按钮控件类
 * @description 可点击的按钮组件，支持文字显示和点击事件
 */
class UIButton : public UIComponent {
private:
    std::string m_text;
    float m_fontSize = 16.0f;
    NVGcolor m_textColor = nvgRGB(255, 255, 255);
    NVGcolor m_hoverColor = nvgRGB(100, 150, 200);
    NVGcolor m_pressedColor = nvgRGB(50, 100, 150);
    bool m_isHovered = false;
    bool m_isPressed = false;
    std::function<void()> m_onClick;
    bool m_isFocused = false;
    NVGcolor m_focusColor = nvgRGB(80, 120, 180);
    
public:
    // 构造函数
    UIButton(float x, float y, float width, float height, const std::string& text = "");
    
    // 重写基类虚函数
    void render(NVGcontext* vg) override;
    void update(double deltaTime) override;
    bool handleEvent(const UIEvent& event) override;
    
    // 按钮特有接口
    void setText(const std::string& text) { m_text = text; }
    const std::string& getText() const { return m_text; }
    
    void setOnClick(std::function<void()> callback) { m_onClick = callback; }
    
    // 状态查询
    bool isHovered() const { return m_isHovered; }
    bool isPressed() const { return m_isPressed; }
    
    // 样式设置
    void setTextColor(NVGcolor color) { m_textColor = color; }
    void setHoverColor(NVGcolor color) { m_hoverColor = color; }
    void setPressedColor(NVGcolor color) { m_pressedColor = color; }
    void setFontSize(float size) { m_fontSize = size; }
    
    // 状态
    bool m_wasPressed = false;
    
    // 样式
    NVGcolor getCurrentBackgroundColor() const;
    void renderText(NVGcontext* vg);
    
    // 新增 focus 相关方法
    bool isFocused() const { return m_isFocused; }
    void setFocusColor(NVGcolor color) { m_focusColor = color; }
    void setFocus(bool focused) { m_isFocused = focused; }
};