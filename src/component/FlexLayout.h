#pragma once
#include "UILayout.h"

class FlexLayout : public UILayout {
public:
    enum Direction {
        HORIZONTAL,  // 横向布局
        VERTICAL     // 纵向布局
    };
    
    // X轴对齐方式
    enum XAlignment {
        X_START,    // 靠左
        X_CENTER,   // 居中
        X_END       // 靠右
    };
    
    // Y轴对齐方式
    enum YAlignment {
        Y_START,        // 靠上
        Y_CENTER,       // 居中
        Y_END,          // 靠下
        Y_SPACE_BETWEEN, // 两端对齐
        Y_SPACE_AROUND   // 环绕对齐
    };
    
    FlexLayout(Direction direction = HORIZONTAL, 
               XAlignment xAlignment = X_START,
               YAlignment yAlignment = Y_START,
               float spacing = 10.0f,
               float padding = 10.0f)
        : m_direction(direction)
        , m_xAlignment(xAlignment)
        , m_yAlignment(yAlignment)
        , m_spacing(spacing)
        , m_padding(padding) {}
    
    void updateLayout(const std::vector<std::shared_ptr<UIComponent>>& children,
                     float containerX, float containerY,
                     float containerWidth, float containerHeight) override;
    
    // 设置布局属性
    void setDirection(Direction direction) { m_direction = direction; }
    void setXAlignment(XAlignment xAlignment) { m_xAlignment = xAlignment; }
    void setYAlignment(YAlignment yAlignment) { m_yAlignment = yAlignment; }
    void setSpacing(float spacing) { m_spacing = spacing; }
    void setPadding(float padding) { m_padding = padding; }
    
private:
    Direction m_direction;
    XAlignment m_xAlignment;
    YAlignment m_yAlignment;
    float m_spacing;
    float m_padding;
    
    void layoutHorizontal(const std::vector<std::shared_ptr<UIComponent>>& children,
                         float containerX, float containerY,
                         float containerWidth, float containerHeight);
    void layoutVertical(const std::vector<std::shared_ptr<UIComponent>>& children,
                       float containerX, float containerY,
                       float containerWidth, float containerHeight);
    
    // 辅助函数
    float calculateXPosition(float containerX, float containerWidth, float childWidth);
    float calculateYPosition(float containerY, float containerHeight, float childHeight);
};