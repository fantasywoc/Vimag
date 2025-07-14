#include "UIAnimation.h"
#include "UIEasing.h"
#include <algorithm>
#include <cmath>

UIAnimation::UIAnimation(AnimationType type, float duration, EasingType easing)
    : m_type(type)
    , m_duration(duration)
    , m_currentTime(0.0f)
    , m_easing(easing)
    , m_delay(0.0f)
    , m_progress(0.0f)
    , m_startValue(0.0f)
    , m_endValue(1.0f)
    , m_isRunning(false)
    , m_isFinished(false)
    , m_isPaused(false)
    , m_hasStarted(false)
    , m_repeatCount(0)
    , m_currentRepeat(0)
    , m_reverse(false)
    , m_isReversing(false)
    , m_scaleOrigin(CENTER)
    , m_rotateOrigin(ROTATE_CENTER)
{
}

void UIAnimation::start() {
    if (m_onStart) {
        m_onStart();
    }
    
    m_isRunning = true;
    m_isFinished = false;
    m_isPaused = false;
    m_hasStarted = true;
    m_currentTime = 0.0f;
    m_currentRepeat = 0;
    m_progress = 0.0f;
}

void UIAnimation::pause() {
    if (m_isRunning) {
        m_isPaused = true;
        m_isRunning = false;
    }
}

void UIAnimation::resume() {
    if (m_isPaused && !m_isFinished) {
        m_isPaused = false;
        m_isRunning = true;
    }
}

void UIAnimation::stop() {
    m_isRunning = false;
    m_isFinished = true;
    m_isPaused = false;
    m_currentTime = 0.0f;
    m_currentRepeat = 0;
    m_progress = 0.0f;
}

void UIAnimation::update(double deltaTime) {
    if (!m_isRunning || m_isFinished || m_duration <= 0.0f) {
        return;
    }
    
    // 处理延迟
    if (m_delay > 0.0f) {
        m_delay -= static_cast<float>(deltaTime);
        if (m_delay > 0.0f) {
            return;
        }
        deltaTime = -m_delay; // 使用剩余时间
        m_delay = 0.0f;
    }
    
    m_currentTime += static_cast<float>(deltaTime);
    
    // 计算进度
    m_progress = std::clamp(m_currentTime / m_duration, 0.0f, 1.0f);
    
    // 应用缓动函数
    float easedProgress = applyEasing(m_progress);
    
    // 如果是反向播放
    if (m_isReversing) {
        easedProgress = 1.0f - easedProgress;
    }
    
    // 触发更新回调
    if (m_onUpdate) {
        m_onUpdate(easedProgress);
    }
    
    // 检查是否完成一次循环
    if (m_progress >= 1.0f) {
        // 检查重复
        if (m_repeatCount == -1 || m_currentRepeat < m_repeatCount) {
            m_currentRepeat++;
            m_currentTime = 0.0f;
            m_progress = 0.0f;
            
            // 如果是反向播放，切换方向
            if (m_reverse) {
                m_isReversing = !m_isReversing;
            }
        } else {
            // 动画完成
            m_isRunning = false;
            m_isFinished = true;
            
            if (m_onComplete) {
                m_onComplete();
            }
        }
    }
}

float UIAnimation::getCurrentValue() const {
    if (!m_hasStarted) {
        return m_startValue;
    }
    
    if (m_isFinished) {
        return m_endValue;
    }
    
    float easedProgress = applyEasing(m_progress);
    if (m_isReversing) {
        easedProgress = 1.0f - easedProgress;
    }
    
    return m_startValue + (m_endValue - m_startValue) * easedProgress;
}

void UIAnimation::setValues(float startValue, float endValue) {
    m_startValue = startValue;
    m_endValue = endValue;
}

float UIAnimation::applyEasing(float t) const {
    switch (m_easing) {
        case LINEAR:
            return UIEasing::linear(t);
        case EASE_IN:
            return UIEasing::easeInQuad(t);
        case EASE_OUT:
            return UIEasing::easeOutQuad(t);
        case EASE_IN_OUT:
            return UIEasing::easeInOutQuad(t);
        case BOUNCE:
            return UIEasing::bounce(t);
        case ELASTIC:
            return UIEasing::elastic(t);
        default:
            return t;
    }
}