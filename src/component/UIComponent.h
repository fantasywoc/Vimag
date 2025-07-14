#pragma once
#include <nanovg.h>
#include <string>
#include <functional>
#include <memory>
#include <iostream>
#include "UIEvent.h"
#include "../animation/UIAnimation.h"

/**
 * @class UIComponent
 * @brief UI组件基类
 * @description 所有UI控件的抽象基类，定义通用接口和属性
 */
class UIComponent {
protected:
    // 动画相关属性
    float m_animationOpacity = 1.0f;
    float m_animationScaleX = 1.0f;
    float m_animationScaleY = 1.0f;
    float m_animationRotation = 0.0f;
    float m_animationOffsetX = 0.0f;
    float m_animationOffsetY = 0.0f;
    
public:
    // 构造函数
    UIComponent(float x = 0, float y = 0, float width = 0, float height = 0);
    virtual ~UIComponent() = default;
    
    // 纯虚函数 - 子类必须实现
    virtual void render(NVGcontext* vg) = 0;
    virtual void update(double deltaTime) = 0;
    virtual bool handleEvent(const UIEvent& event) = 0;
    
    // 通用接口
    virtual void setPosition(float x, float y);
    virtual void setSize(float width, float height);
    virtual void setBounds(float x, float y, float width, float height);
    virtual bool contains(float px, float py) const;
    
    // 属性访问器
    float getX() const { return m_x; }
    float getY() const { return m_y; }
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }
    
    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }
    
    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }
    
    // 添加display属性的访问器
    bool isDisplay() const { return m_display; }
    void setDisplay(bool display) { m_display = display; }
    
    // 样式设置
    void setBackgroundColor(NVGcolor color) { m_backgroundColor = color; }
    void setBorderColor(NVGcolor color) { m_borderColor = color; }
    void setBorderWidth(float width) { m_borderWidth = width; }
    void setCornerRadius(float radius) { m_cornerRadius = radius; }
    
    // 动画接口
    void fadeIn(float duration = 0.3f, UIAnimation::EasingType easing = UIAnimation::EASE_OUT);
    void fadeOut(float duration = 0.3f, UIAnimation::EasingType easing = UIAnimation::EASE_IN);
    void moveTo(float x, float y, float duration = 0.3f);
    void scaleTo(float scaleX, float scaleY, float duration = 0.3f);
    void rotateTo(float angle, float duration = 0.3f);
    
    // 动画属性设置器（由动画系统调用）
    void setAnimationOpacity(float opacity) { m_animationOpacity = opacity; }
    void setAnimationScale(float scaleX, float scaleY) { m_animationScaleX = scaleX; m_animationScaleY = scaleY; }
    void setAnimationRotation(float rotation) { m_animationRotation = rotation; }
    void setAnimationOffset(float offsetX, float offsetY) { m_animationOffsetX = offsetX; m_animationOffsetY = offsetY; }
    
    // 添加单独的动画属性设置器
    void setAnimationOffsetX(float offsetX) { m_animationOffsetX = offsetX; }
    void setAnimationOffsetY(float offsetY) { m_animationOffsetY = offsetY; }
    void setAnimationScaleX(float scaleX) { m_animationScaleX = scaleX; }
    void setAnimationScaleY(float scaleY) { m_animationScaleY = scaleY; }
    
    // 动画属性获取器（由动画系统调用）
    float getAnimationOpacity() const { return m_animationOpacity; }
    float getAnimationOffsetX() const { return m_animationOffsetX; }
    float getAnimationOffsetY() const { return m_animationOffsetY; }
    float getAnimationScaleX() const { return m_animationScaleX; }
    float getAnimationScaleY() const { return m_animationScaleY; }
    float getAnimationRotation() const { return m_animationRotation; }
    bool isAnimating() const{ return m_animationOpacity != 1.0f || m_animationScaleX != 1.0f || m_animationScaleY != 1.0f || m_animationRotation != 0.0f || m_animationOffsetX != 0.0f || m_animationOffsetY != 0.0f;};  
protected:
    // 基础属性
    float m_x, m_y, m_width, m_height;
    bool m_visible = true;
    bool m_enabled = true;
    bool m_display = true;  // 新增display属性，默认为true
    
    // 样式属性
    NVGcolor m_backgroundColor = nvgRGB(200, 200, 200);
    NVGcolor m_borderColor = nvgRGB(100, 100, 100);
    float m_borderWidth = 0.0f;
    float m_cornerRadius = 0.0f;
    
    // 辅助渲染方法
    void renderBackground(NVGcontext* vg);
    void renderBorder(NVGcontext* vg);
};

// 删除从这里开始的所有内容（第70-86行）：
// /**
//  * @brief UI事件结构体
//  */
// struct UIEvent {
//     enum Type {
//         MOUSE_MOVE,
//         MOUSE_PRESS,
//         MOUSE_RELEASE,
//         KEY_PRESS,
//         KEY_RELEASE
//     };
//     
//     Type type;
//     float mouseX, mouseY;
//     int mouseButton;
//     int keyCode;
// };