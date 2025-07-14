#pragma once
#include <functional>
#include <memory>

class UIAnimation {
public:
    enum AnimationType {
        FADE,
        MOVE,
        SCALE,
        ROTATE,
        CUSTOM
    };
    
    enum EasingType {
        LINEAR,
        EASE_IN,
        EASE_OUT,
        EASE_IN_OUT,
        BOUNCE,
        ELASTIC
    };
    
    // 添加缩放原点枚举
    enum ScaleOrigin {
        TOP_LEFT,
        CENTER,
        TOP_RIGHT  // 添加缺失的枚举值
    };
    
    // 添加旋转原点枚举
    enum RotateOrigin {
        ROTATE_TOP_LEFT,
        ROTATE_CENTER,
        ROTATE_TOP_RIGHT  // 添加缺失的枚举值
    };
    
    UIAnimation(AnimationType type, float duration, EasingType easing = LINEAR);
    ~UIAnimation() = default;
    
    void start();
    void pause();
    void resume();
    void stop();
    void update(double deltaTime);
    
    bool isRunning() const { return m_isRunning; }
    bool isFinished() const { return m_isFinished; }
    bool isPaused() const { return m_isPaused; }
    
    float getProgress() const { return m_progress; }
    float getCurrentValue() const;
    
    void setValues(float startValue, float endValue);
    void setDuration(float duration) { m_duration = duration; }
    void setEasing(EasingType easing) { m_easing = easing; }
    void setDelay(float delay) { m_delay = delay; }
    void setRepeatCount(int count) { m_repeatCount = count; }
    void setReverse(bool reverse) { m_reverse = reverse; }
    
    // 设置缩放和旋转原点
    void setScaleOrigin(ScaleOrigin origin) { m_scaleOrigin = origin; }
    void setRotateOrigin(RotateOrigin origin) { m_rotateOrigin = origin; }
    
    void setOnUpdate(std::function<void(float)> callback) { m_onUpdate = callback; }
    void setOnComplete(std::function<void()> callback) { m_onComplete = callback; }
    void setOnStart(std::function<void()> callback) { m_onStart = callback; }
    
private:
    float applyEasing(float t) const;
    
    AnimationType m_type;
    EasingType m_easing;
    ScaleOrigin m_scaleOrigin;
    RotateOrigin m_rotateOrigin;
    
    float m_duration;
    float m_delay;
    float m_currentTime;
    float m_progress;
    
    float m_startValue;
    float m_endValue;
    
    bool m_isRunning;
    bool m_isFinished;
    bool m_isPaused;
    bool m_hasStarted;
    
    int m_repeatCount;
    int m_currentRepeat;
    bool m_reverse;
    bool m_isReversing;
    
    std::function<void(float)> m_onUpdate;
    std::function<void()> m_onComplete;
    std::function<void()> m_onStart;
};