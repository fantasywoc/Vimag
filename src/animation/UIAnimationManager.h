#pragma once
#include "UIAnimation.h"
#include <vector>
#include <memory>
#include <unordered_map>

class UIComponent;

class UIAnimationManager {
public:
    static UIAnimationManager& getInstance();
    
    void addAnimation(std::shared_ptr<UIAnimation> animation, UIComponent* target);
    void removeAnimation(UIComponent* target);
    void removeAllAnimations();
    void update(double deltaTime);
    
    // 便捷方法
    void fadeIn(UIComponent* target, float duration = 0.3f, UIAnimation::EasingType easing = UIAnimation::EASE_OUT);
    void fadeOut(UIComponent* target, float duration = 0.3f, UIAnimation::EasingType easing = UIAnimation::EASE_IN);
    void moveTo(UIComponent* target, float x, float y, float duration = 0.3f, UIAnimation::EasingType easing = UIAnimation::EASE_OUT);
    void scaleTo(UIComponent* target, float scaleX, float scaleY, float duration = 0.3f, UIAnimation::EasingType easing = UIAnimation::EASE_OUT, UIAnimation::ScaleOrigin origin = UIAnimation::TOP_LEFT);
    void rotateTo(UIComponent* target, float angle, float duration = 0.3f, UIAnimation::EasingType easing = UIAnimation::EASE_OUT, UIAnimation::RotateOrigin origin = UIAnimation::ROTATE_CENTER);
    
    // 查询方法
    bool hasAnimations(UIComponent* target) const;
    size_t getAnimationCount() const;
    
private:
    UIAnimationManager() = default;
    ~UIAnimationManager() = default;
    UIAnimationManager(const UIAnimationManager&) = delete;
    UIAnimationManager& operator=(const UIAnimationManager&) = delete;
    
    struct AnimationInfo {
        std::shared_ptr<UIAnimation> animation;
        UIComponent* target;
        bool isActive;
        
        AnimationInfo(std::shared_ptr<UIAnimation> anim, UIComponent* tgt) 
            : animation(anim), target(tgt), isActive(true) {}
    };
    
    std::vector<AnimationInfo> m_animations;
    bool m_isUpdating = false;  // 添加更新状态标志
    std::vector<UIComponent*> m_pendingRemovals;  // 待移除的组件列表
};