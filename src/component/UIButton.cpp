#include "UIButton.h"
#include <iostream>

UIButton::UIButton(float x, float y, float width, float height, const std::string& text)
    : UIComponent(x, y, width, height), m_text(text) {
    m_backgroundColor = nvgRGB(70, 130, 180);
   
}

void UIButton::render(NVGcontext* vg) {
    if (!m_visible) return;
    
    nvgSave(vg);
    
    // 应用动画变换 - 使用中心坐标变换
    nvgTranslate(vg, m_x + m_animationOffsetX, m_y + m_animationOffsetY);
    
    // 旋转变换（以中心为原点）
    if (m_animationRotation != 0.0f) {
        nvgTranslate(vg, m_width/2, m_height/2);
        nvgRotate(vg, m_animationRotation);
        nvgTranslate(vg, -m_width/2, -m_height/2);
    }
    
    // 缩放变换（以中心为原点）
    if (m_animationScaleX != 1.0f || m_animationScaleY != 1.0f) {
        nvgTranslate(vg, m_width/2, m_height/2);
        nvgScale(vg, m_animationScaleX, m_animationScaleY);
        nvgTranslate(vg, -m_width/2, -m_height/2);
    }
    
    // 应用透明度
    nvgGlobalAlpha(vg, m_animationOpacity);
    
    // 渲染背景（使用相对坐标）
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0, 0, m_width, m_height, m_cornerRadius);
    nvgFillColor(vg, getCurrentBackgroundColor());
    nvgFill(vg);
    
    // 渲染边框
    if (m_borderWidth > 0) {
        nvgStrokeColor(vg, m_borderColor);
        nvgStrokeWidth(vg, m_borderWidth);
        nvgStroke(vg);
    }
    
    // 渲染文字
    renderText(vg);
    
    nvgRestore(vg);
}

void UIButton::update(double deltaTime) {
    // 按钮通常不需要更新逻辑
    // 可以在这里添加动画效果
}

bool UIButton::handleEvent(const UIEvent& event) {
    if (!m_visible || !m_enabled) return false;
    
    // 添加调试信息
    if (event.type == UIEvent::MOUSE_PRESS) {
        std::cout << "Button事件: 鼠标坐标(" << event.mouseX << ", " << event.mouseY << ")" << std::endl;
        std::cout << "Button位置: (" << m_x << ", " << m_y << ") 大小: (" << m_width << ", " << m_height << ")" << std::endl;
        std::cout << "Button动画偏移: (" << m_animationOffsetX << ", " << m_animationOffsetY << ")" << std::endl;
        std::cout << "contains结果: " << contains(event.mouseX, event.mouseY) << std::endl;
    }
    
    bool wasHovered = m_isHovered;
    bool wasFocused = m_isFocused;
    
    switch (event.type) {
        case UIEvent::MOUSE_MOVE:
            m_isHovered = contains(event.mouseX, event.mouseY);
            // 如果 hover 状态发生变化，触发重绘
            if (wasHovered != m_isHovered) {
                // 可以在这里添加 hover 状态变化的回调
                std::cout << "Button hover 状态: " << (m_isHovered ? "进入" : "离开") << std::endl;
            }
            return m_isHovered;
            
        case UIEvent::MOUSE_PRESS:
            if (m_isHovered && event.mouseButton == 0) { // 左键
                m_isPressed = true;
                m_isFocused = true; // 点击时获得焦点
                // 如果焦点状态发生变化，输出调试信息
                if (wasFocused != m_isFocused) {
                    std::cout << "Button focus 状态: 获得焦点" << std::endl;
                }
                return true;
            } else {
                // 点击其他区域时失去焦点
                if (m_isFocused) {
                    m_isFocused = false;
                    std::cout << "Button focus 状态: 失去焦点" << std::endl;
                }
            }
            break;
            
        case UIEvent::MOUSE_RELEASE:
            if (m_isPressed && event.mouseButton == 0) {
                m_isPressed = false;
                // 重新检查鼠标是否仍在按钮区域内
                m_isHovered = contains(event.mouseX, event.mouseY);
                if (m_isHovered && m_onClick) {
                    m_onClick(); // 触发点击回调
                }
                return true;
            }
            break;
    }
    
    return false;
}

NVGcolor UIButton::getCurrentBackgroundColor() const {
    if (m_isPressed) {
        return m_pressedColor;
    } else if (m_isHovered) {
        return m_hoverColor;
    } else if (m_isFocused) {
        return m_focusColor;
    }
    return m_backgroundColor;
}

void UIButton::renderText(NVGcontext* vg) {
    if (m_text.empty()) return;
    
    nvgFontFace(vg, "default");
    nvgFontSize(vg, m_fontSize);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgFillColor(vg, m_textColor);
    
    // 使用相对坐标（相对于按钮左上角）
    float centerX = m_width / 2.0f;
    float centerY = m_height / 2.0f;
    nvgText(vg, centerX, centerY, m_text.c_str(), nullptr);
}