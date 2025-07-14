#include "UILabel.h"
#include <nanovg.h>
#include <sstream>

UILabel::UILabel(float x, float y, float width, float height, const std::string& text)
    : UIComponent(x, y, width, height), m_text(text) {
}

void UILabel::render(NVGcontext* vg) {
    if (!m_visible || !m_display) return;  // 同时检查visible和display属性
    
    nvgSave(vg);
    
    // 先设置透明度，再应用其他变换
    nvgGlobalAlpha(vg, m_animationOpacity);
    
    // 应用动画变换
    nvgTranslate(vg, m_x + m_animationOffsetX, m_y + m_animationOffsetY);
    nvgRotate(vg, m_animationRotation);
    nvgScale(vg, m_animationScaleX, m_animationScaleY);
    
    // 渲染文字
    renderText(vg);
    
    nvgRestore(vg);
}

void UILabel::update(double deltaTime) {
    // UILabel 通常不需要更新逻辑
}

bool UILabel::handleEvent(const UIEvent& event) {
    // UILabel 通常不处理事件
    return false;
}

void UILabel::autoResize(NVGcontext* vg) {
    if (m_text.empty()) return;
    
    nvgFontSize(vg, m_fontSize);
    nvgTextAlign(vg, m_textAlign | m_verticalAlign);
    
    float bounds[4];
    nvgTextBounds(vg, 0, 0, m_text.c_str(), nullptr, bounds);
    
    m_width = bounds[2] - bounds[0];
    m_height = bounds[3] - bounds[1];
}

void UILabel::renderText(NVGcontext* vg) {
    if (m_text.empty()) return;
    
    nvgFontSize(vg, m_fontSize);
    nvgFillColor(vg, m_textColor);
    nvgTextAlign(vg, m_textAlign | m_verticalAlign);
    
    // 分割文本为多行
    std::vector<std::string> lines;
    std::stringstream ss(m_text);
    std::string line;
    
    while (std::getline(ss, line, '\n')) {
        lines.push_back(line);
    }
    
    // 计算行高
    float lineHeight = m_fontSize * 1.2f; // 通常行高是字体大小的1.2倍
    float totalHeight = lines.size() * lineHeight;
    
    float textX = 0;
    float startY = 0;
    
    // 根据对齐方式调整文字位置
    if (m_textAlign == CENTER) {
        textX += m_width / 2.0f;
    } else if (m_textAlign == RIGHT) {
        textX += m_width;
    }
    
    if (m_verticalAlign == MIDDLE) {
        startY += (m_height - totalHeight) / 2.0f + lineHeight;
    } else if (m_verticalAlign == BOTTOM) {
        startY += m_height - totalHeight + lineHeight;
    } else {
        startY += lineHeight; // TOP对齐
    }
    
    // 逐行渲染文本
    for (size_t i = 0; i < lines.size(); ++i) {
        float textY = startY + i * lineHeight;
        nvgText(vg, textX, textY, lines[i].c_str(), nullptr);
    }
}