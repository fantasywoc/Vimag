#pragma once
#include "UIComponent.h"

/**
 * @class UILabel
 * @brief 文字标签控件类
 * @description 用于显示静态文字的组件
 */
class UILabel : public UIComponent {
public:
    enum TextAlign {
        LEFT = NVG_ALIGN_LEFT,
        CENTER = NVG_ALIGN_CENTER,
        RIGHT = NVG_ALIGN_RIGHT
    };
    
    enum VerticalAlign {
        TOP = NVG_ALIGN_TOP,
        MIDDLE = NVG_ALIGN_MIDDLE,
        BOTTOM = NVG_ALIGN_BOTTOM
    };
    
    // 构造函数
    UILabel(float x, float y, float width, float height, const std::string& text = "");
    
    // 重写基类虚函数
    void render(NVGcontext* vg) override;
    void update(double deltaTime) override;
    bool handleEvent(const UIEvent& event) override;
    
    // 文字标签特有接口
    void setText(const std::string& text) { m_text = text; }
    const std::string& getText() const { return m_text; }
    
    void setTextColor(NVGcolor color) { m_textColor = color; }
    void setFontSize(float size) { m_fontSize = size; }
    void setTextAlign(TextAlign align) { m_textAlign = align; }
    void setVerticalAlign(VerticalAlign align) { m_verticalAlign = align; }
    
    // 自动调整大小
    void autoResize(NVGcontext* vg);
    
private:
    std::string m_text;
    NVGcolor m_textColor = nvgRGBA(0, 0, 0, 200);
    float m_fontSize = 18.0f;
    TextAlign m_textAlign = LEFT;
    VerticalAlign m_verticalAlign = MIDDLE;
    
    void renderText(NVGcontext* vg);
};