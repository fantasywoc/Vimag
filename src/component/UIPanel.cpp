#include "UIPanel.h"
#include "FlexLayout.h"
#include <algorithm>
#include <iostream>  // 添加这行

UIPanel::UIPanel(float x, float y, float width, float height)
    : UIComponent(x, y, width, height) {
}

void UIPanel::render(NVGcontext* vg) {
    if (!m_visible || !m_display) return;  // 同时检查visible和display属性
    
    nvgSave(vg);
    
    // 应用动画变换 - 修正变换顺序
    // 1. 先移动到组件位置（包含动画偏移）
    nvgTranslate(vg, m_x + m_animationOffsetX, m_y + m_animationOffsetY);
    
    // 2. 移动到组件中心进行旋转和缩放
    nvgTranslate(vg, m_width * 0.5f, m_height * 0.5f);
    nvgRotate(vg, m_animationRotation);
    nvgScale(vg, m_animationScaleX, m_animationScaleY);
    
    // 3. 移回到组件左上角
    nvgTranslate(vg, -m_width * 0.5f, -m_height * 0.5f);
    
    // 应用透明度
    nvgGlobalAlpha(vg, m_animationOpacity);
    
    // 渲染面板背景 - 使用相对坐标
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0, 0, m_width, m_height, m_cornerRadius);
    nvgFillColor(vg, m_backgroundColor);
    nvgFill(vg);
    
    // 渲染边框（如果需要）
    if (m_borderWidth > 0) {
        nvgStrokeColor(vg, m_borderColor);
        nvgStrokeWidth(vg, m_borderWidth);
        nvgStroke(vg);
    }
    
    // 渲染所有子组件
    for (auto& child : m_children) {
        if (child && child->isDisplay()) {
            child->render(vg);
        }
    }
    
    nvgRestore(vg);
}

void UIPanel::update(double deltaTime) {
    // 更新所有子组件
    for (auto& child : m_children) {
        if (child) {
            child->update(deltaTime);
        }
    }
}

bool UIPanel::handleEvent(const UIEvent& event) {
    if (!m_visible || !m_enabled) {
        return false;
    }
    
    // 在处理鼠标事件前强制更新布局
    if (event.type == UIEvent::MOUSE_PRESS || event.type == UIEvent::MOUSE_RELEASE || event.type == UIEvent::MOUSE_MOVE) {
        updateLayout();
    }
    
    // 添加调试输出
    if (event.type == UIEvent::MOUSE_PRESS) {
        // std::cout << "Panel处理事件: 面板位置(" << m_x << ", " << m_y << ") \n";
        // std::cout << " 原始鼠标坐标: (" << event.mouseX << ", " << event.mouseY << ") \n";
    }
    
    // 从后往前遍历子组件（后添加的在上层）
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if (*it) {
            // 创建相对坐标的事件副本 - 考虑动画偏移
            UIEvent localEvent = event;
            localEvent.mouseX = event.mouseX - (m_x + m_animationOffsetX);
            localEvent.mouseY = event.mouseY - (m_y + m_animationOffsetY);
            
            // // 添加调试输出
            // if (event.type == UIEvent::MOUSE_PRESS) {
            //     std::cout << " 转换后的鼠标坐标: (" << localEvent.mouseX << ", " << localEvent.mouseY << ") \n";
            // }
            
            if ((*it)->handleEvent(localEvent)) {
                return true; // 事件被子组件处理
            }
        }
    }
    
    // 检查事件是否在面板范围内 - 也要考虑动画偏移
    if (event.type == UIEvent::MOUSE_PRESS || event.type == UIEvent::MOUSE_MOVE) {
        float actualX = m_x + m_animationOffsetX;
        float actualY = m_y + m_animationOffsetY;
        return (event.mouseX >= actualX && event.mouseX <= actualX + m_width &&
                event.mouseY >= actualY && event.mouseY <= actualY + m_height);
    }
    
    return false;
}

void UIPanel::addChild(std::shared_ptr<UIComponent> child) {
    if (child) {
        m_children.push_back(child);  // 始终添加子组件
        updateLayout();  // 布局会自动处理display属性
    }
}

void UIPanel::removeChild(std::shared_ptr<UIComponent> child) {
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it != m_children.end()) {
        m_children.erase(it);
        updateLayout();
    }
}

void UIPanel::clearChildren() {
    m_children.clear();
}

void UIPanel::setLayout(std::unique_ptr<UILayout> layout) {
    m_layout = std::move(layout);
    updateLayout();
}

void UIPanel::updateLayout() {
    if (m_layout) {
        // 传递相对坐标 (0, 0) 而不是绝对坐标
        m_layout->updateLayout(m_children, 0, 0, m_width, m_height);
    }
}

void UIPanel::setLayout(FlexLayout::Direction direction, 
                       FlexLayout::XAlignment xAlign, 
                       FlexLayout::YAlignment yAlign,
                       float spacing, 
                       float padding) {
    m_layout = std::make_unique<FlexLayout>(direction, xAlign, yAlign, spacing, padding);
}

void UIPanel::setVerticalLayoutWithAlignment(FlexLayout::XAlignment xAlign, 
                                           FlexLayout::YAlignment yAlign,
                                           float spacing, 
                                           float padding) {
    setLayout(FlexLayout::VERTICAL, xAlign, yAlign, spacing, padding);
}

void UIPanel::setHorizontalLayoutWithAlignment(FlexLayout::XAlignment xAlign,
                                             FlexLayout::YAlignment yAlign,
                                             float spacing, 
                                             float padding) {
    setLayout(FlexLayout::HORIZONTAL, xAlign, yAlign, spacing, padding);
}

FlexLayout* UIPanel::getFlexLayout() const {
    return dynamic_cast<FlexLayout*>(m_layout.get());
}


void UIPanel::resetAllAnimationOffsets() {
    // 重置自身的动画偏移
    setAnimationOffsetX(0);
    setAnimationOffsetY(0);
    
    // 递归重置所有子组件的动画偏移
    for (auto& child : m_children) {
        if (child) {
            // 重置子组件自身的动画偏移
            child->setAnimationOffsetX(0);
            child->setAnimationOffsetY(0);
            
            // 如果子组件也是UIPanel，递归调用
            auto childPanel = std::dynamic_pointer_cast<UIPanel>(child);
            if (childPanel) {
                childPanel->resetAllAnimationOffsets();
            }
        }
    }
}