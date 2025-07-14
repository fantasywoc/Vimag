#pragma once
#include <vector>
#include <memory>

class UIComponent;

/**
 * @class UILayout
 * @brief 布局管理器基类
 */
class UILayout {
public:
    virtual ~UILayout() = default;
    virtual void updateLayout(const std::vector<std::shared_ptr<UIComponent>>& children, 
                            float containerX, float containerY,
                            float containerWidth, float containerHeight) = 0;
};