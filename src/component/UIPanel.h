#pragma once
#include "UIComponent.h"
#include "UILayout.h"
#include "FlexLayout.h"
#include <vector>
#include <memory>

class UIPanel : public UIComponent {
public:
    UIPanel(float x, float y, float width, float height);
    
    // 重写基类虚函数
    void render(NVGcontext* vg) override;
    void update(double deltaTime) override;
    bool handleEvent(const UIEvent& event) override;
    
    // 子组件管理
    void addChild(std::shared_ptr<UIComponent> child);
    void removeChild(std::shared_ptr<UIComponent> child);
    void clearChildren();
    
    // 布局功能
    void setLayout(std::unique_ptr<UILayout> layout);
    void updateLayout();
    
    // 设置布局 - 新的独立对齐方式
    void setLayout(FlexLayout::Direction direction, 
                   FlexLayout::XAlignment xAlign, 
                   FlexLayout::YAlignment yAlign,
                   float spacing = 10.0f, 
                   float padding = 10.0f);
    
    // 便捷方法
    void setVerticalLayoutWithAlignment(FlexLayout::XAlignment xAlign, 
                                       FlexLayout::YAlignment yAlign = FlexLayout::Y_START,
                                       float spacing = 10.0f, 
                                       float padding = 10.0f);
    
    void setHorizontalLayoutWithAlignment(FlexLayout::XAlignment xAlign,
                                         FlexLayout::YAlignment yAlign,
                                         float spacing = 10.0f, 
                                         float padding = 10.0f);
    
    // 获取当前布局
    FlexLayout* getFlexLayout() const;
    
    // 添加这个方法来访问子组件
    const std::vector<std::shared_ptr<UIComponent>>& getChildren() const { return m_children; }
    
    // 添加递归重置动画偏移的方法
    void resetAllAnimationOffsets();
    
private:
    std::vector<std::shared_ptr<UIComponent>> m_children;
    std::unique_ptr<UILayout> m_layout;
};