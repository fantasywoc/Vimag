#include "UIAnimationManager.h"
#include "../component/UIComponent.h"
#include <algorithm>
#include <iostream>

UIAnimationManager& UIAnimationManager::getInstance() {
    static UIAnimationManager instance;
    return instance;
}

void UIAnimationManager::addAnimation(std::shared_ptr<UIAnimation> animation, UIComponent* target) {
    if (!animation || !target) {
        return;
    }
    
    m_animations.emplace_back(animation, target);
    animation->start();
}

void UIAnimationManager::removeAnimation(UIComponent* target) {
    if (!target) {
        return;
    }
    
    // 如果正在更新，将移除操作延迟
    if (m_isUpdating) {
        m_pendingRemovals.push_back(target);
        return;
    }
    
    // 立即移除
    m_animations.erase(
        std::remove_if(m_animations.begin(), m_animations.end(),
            [target](const AnimationInfo& info) {
                return info.target == target;
            }),
        m_animations.end()
    );
}

void UIAnimationManager::removeAllAnimations() {
    if (m_isUpdating) {
        // 如果正在更新，标记所有动画为待移除
        for (auto& info : m_animations) {
            if (info.target) {
                m_pendingRemovals.push_back(info.target);
            }
        }
    } else {
        m_animations.clear();
    }
}

void UIAnimationManager::update(double deltaTime) {
    m_isUpdating = true;
    
    // 创建当前帧要更新的动画副本，避免在遍历过程中修改
    std::vector<AnimationInfo> currentAnimations = m_animations;
    
    // 更新所有动画
    for (size_t i = 0; i < currentAnimations.size(); ++i) {
        auto& info = currentAnimations[i];
        
        // 检查原始动画是否仍然存在且活跃
        auto it = std::find_if(m_animations.begin(), m_animations.end(),
            [&info](const AnimationInfo& original) {
                return original.animation == info.animation && 
                       original.target == info.target && 
                       original.isActive;
            });
        
        if (it != m_animations.end() && info.isActive && info.animation) {
            info.animation->update(deltaTime);
            
            // 检查动画是否完成
            if (info.animation->isFinished()) {
                it->isActive = false;
            }
        }
    }
    
    m_isUpdating = false;
    
    // 处理延迟的移除操作
    for (UIComponent* target : m_pendingRemovals) {
        m_animations.erase(
            std::remove_if(m_animations.begin(), m_animations.end(),
                [target](const AnimationInfo& info) {
                    return info.target == target;
                }),
            m_animations.end()
        );
    }
    m_pendingRemovals.clear();
    
    // 移除已完成的动画
    m_animations.erase(
        std::remove_if(m_animations.begin(), m_animations.end(),
            [](const AnimationInfo& info) {
                return !info.isActive;
            }),
        m_animations.end()
    );
}

void UIAnimationManager::fadeIn(UIComponent* target, float duration, UIAnimation::EasingType easing) {
    if (!target) return;
    
    auto animation = std::make_shared<UIAnimation>(UIAnimation::FADE, duration, easing);
    animation->setValues(target->getAnimationOpacity(), 1.0f);
    animation->setOnUpdate([target](float value) {
        target->setAnimationOpacity(value);
    });
    
    addAnimation(animation, target);
}

void UIAnimationManager::fadeOut(UIComponent* target, float duration, UIAnimation::EasingType easing) {
    if (!target) return;
    
    auto animation = std::make_shared<UIAnimation>(UIAnimation::FADE, duration, easing);
    animation->setValues(target->getAnimationOpacity(), 0.0f);
    animation->setOnUpdate([target](float value) {
        target->setAnimationOpacity(value);
    });
    
    addAnimation(animation, target);
}

void UIAnimationManager::moveTo(UIComponent* target, float x, float y, float duration, UIAnimation::EasingType easing) {
    if (!target) return;
    
    float startX = target->getX() + target->getAnimationOffsetX();
    float startY = target->getY() + target->getAnimationOffsetY();
    
    auto animation = std::make_shared<UIAnimation>(UIAnimation::MOVE, duration, easing);
    animation->setValues(0.0f, 1.0f);
    animation->setOnUpdate([target, startX, startY, x, y](float progress) {
        float currentX = startX + (x - startX) * progress;
        float currentY = startY + (y - startY) * progress;
        
        // 添加子像素对齐以减少撕裂
        currentX = std::round(currentX);
        currentY = std::round(currentY);
        
        target->setAnimationOffsetX(currentX - target->getX());
        target->setAnimationOffsetY(currentY - target->getY());
    });
    
    addAnimation(animation, target);
}

void UIAnimationManager::scaleTo(UIComponent* target, float scaleX, float scaleY, float duration, UIAnimation::EasingType easing, UIAnimation::ScaleOrigin origin) {
    if (!target) return;
    
    float startScaleX = target->getAnimationScaleX();
    float startScaleY = target->getAnimationScaleY();
    
    auto animation = std::make_shared<UIAnimation>(UIAnimation::SCALE, duration, easing);
    animation->setValues(0.0f, 1.0f);
    animation->setScaleOrigin(origin);
    animation->setOnUpdate([target, startScaleX, startScaleY, scaleX, scaleY, origin](float progress) {
        float currentScaleX = startScaleX + (scaleX - startScaleX) * progress;
        float currentScaleY = startScaleY + (scaleY - startScaleY) * progress;
        target->setAnimationScaleX(currentScaleX);
        target->setAnimationScaleY(currentScaleY);
        
        // 根据缩放原点调整位置
        if (origin == UIAnimation::CENTER) {
            float offsetX = target->getWidth() * (1.0f - currentScaleX) * 0.5f;
            float offsetY = target->getHeight() * (1.0f - currentScaleY) * 0.5f;
            target->setAnimationOffsetX(offsetX);
            target->setAnimationOffsetY(offsetY);
        }
    });
    
    addAnimation(animation, target);
}

void UIAnimationManager::rotateTo(UIComponent* target, float angle, float duration, UIAnimation::EasingType easing, UIAnimation::RotateOrigin origin) {
    if (!target) return;
    
    // 将角度转换为弧度
    angle = angle * 3.14159265359f / 180.0f;
    
    float startAngle = target->getAnimationRotation();
    
    auto animation = std::make_shared<UIAnimation>(UIAnimation::ROTATE, duration, easing);
    animation->setValues(0.0f, 1.0f);
    animation->setRotateOrigin(origin);
    animation->setOnUpdate([target, startAngle, angle](float progress) {
        float currentAngle = startAngle + (angle - startAngle) * progress;
        target->setAnimationRotation(currentAngle);
    });
    
    addAnimation(animation, target);
}

bool UIAnimationManager::hasAnimations(UIComponent* target) const {
    return std::any_of(m_animations.begin(), m_animations.end(),
        [target](const AnimationInfo& info) {
            return info.target == target && info.isActive;
        });
}

size_t UIAnimationManager::getAnimationCount() const {
    return std::count_if(m_animations.begin(), m_animations.end(),
        [](const AnimationInfo& info) {
            return info.isActive;
        });
}