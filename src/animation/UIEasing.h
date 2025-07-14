#pragma once
#include <cmath>

class UIEasing {
public:
    // 线性缓动
    static float linear(float t) { return t; }
    
    // 二次缓动
    static float easeInQuad(float t) { return t * t; }
    static float easeOutQuad(float t) { return t * (2 - t); }
    static float easeInOutQuad(float t) {
        return t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
    }
    
    // 三次缓动
    static float easeInCubic(float t) { return t * t * t; }
    static float easeOutCubic(float t) { return (--t) * t * t + 1; }
    static float easeInOutCubic(float t) {
        return t < 0.5f ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
    }
    
    // 四次缓动
    static float easeInQuart(float t) { return t * t * t * t; }
    static float easeOutQuart(float t) { return 1 - (--t) * t * t * t; }
    static float easeInOutQuart(float t) {
        return t < 0.5f ? 8 * t * t * t * t : 1 - 8 * (--t) * t * t * t;
    }
    
    // 弹跳缓动
    static float bounce(float t) {
        if (t < 1/2.75f) {
            return 7.5625f * t * t;
        } else if (t < 2/2.75f) {
            return 7.5625f * (t -= 1.5f/2.75f) * t + 0.75f;
        } else if (t < 2.5f/2.75f) {
            return 7.5625f * (t -= 2.25f/2.75f) * t + 0.9375f;
        } else {
            return 7.5625f * (t -= 2.625f/2.75f) * t + 0.984375f;
        }
    }
    
    // 弹性缓动
    static float elastic(float t) {
        if (t == 0) return 0;
        if (t == 1) return 1;
        
        float p = 0.3f;
        float s = p / 4;
        return -(std::pow(2, 10 * (t -= 1)) * std::sin((t - s) * (2 * 3.14159f) / p));
    }
    
    // 回弹缓动
    static float back(float t) {
        float s = 1.70158f;
        return t * t * ((s + 1) * t - s);
    }
    
    // 圆形缓动
    static float easeInCirc(float t) {
        return 1 - std::sqrt(1 - t * t);
    }
    
    static float easeOutCirc(float t) {
        return std::sqrt(1 - (t - 1) * (t - 1));
    }
    
    static float easeInOutCirc(float t) {
        if (t < 0.5f) {
            return (1 - std::sqrt(1 - 4 * t * t)) / 2;
        } else {
            return (std::sqrt(1 - (-2 * t + 2) * (-2 * t + 2)) + 1) / 2;
        }
    }
};