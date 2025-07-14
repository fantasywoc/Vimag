#include "UITextInput.h"
#include <algorithm>
#include <cstring>

UITextInput::UITextInput(float x, float y, float width, float height, const std::string& placeholder)
    : UIComponent(x, y, width, height), m_placeholder(placeholder) {
    m_backgroundColor = nvgRGB(40, 40, 40);
    m_borderColor = nvgRGB(80, 80, 80);
    m_borderWidth = 1.0f;
    m_cornerRadius = 4.0f;
    m_lastCursorBlink = std::chrono::steady_clock::now();
}

void UITextInput::render(NVGcontext* vg) {
    if (!m_visible) return;
    
    nvgSave(vg);
    
    // 先移动到组件位置
    nvgTranslate(vg, m_x + m_animationOffsetX, m_y + m_animationOffsetY);
    
    // 缩放变换（以组件中心为原点）
    if (m_animationScaleX != 1.0f || m_animationScaleY != 1.0f) {
        nvgTranslate(vg, m_width/2, m_height/2);
        nvgScale(vg, m_animationScaleX, m_animationScaleY);
        nvgTranslate(vg, -m_width/2, -m_height/2);
    }
    
    // 应用透明度
    nvgGlobalAlpha(vg, m_animationOpacity);
    
    // 绘制背景（使用相对坐标）
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0, 0, m_width, m_height, m_cornerRadius);
    nvgFillColor(vg, m_backgroundColor);
    nvgFill(vg);
    
    // 绘制边框
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0, 0, m_width, m_height, m_cornerRadius);
    nvgStrokeColor(vg, m_isFocused ? m_focusedBorderColor : m_borderColor);
    nvgStrokeWidth(vg, m_borderWidth);
    nvgStroke(vg);
    
    // 设置裁剪区域（使用相对坐标）
    nvgSave(vg);
    nvgIntersectScissor(vg, m_padding, 0, m_width - 2 * m_padding, m_height);
    
    // 绘制选择区域
    if (m_hasSelection && m_isFocused) {
        renderSelection(vg);
    }
    
    // 绘制文本
    renderText(vg);
    
    // 绘制光标
    if (m_isFocused && m_cursorVisible && !m_hasSelection) {
        renderCursor(vg);
    }
    
    nvgRestore(vg);
    nvgRestore(vg);
}

void UITextInput::update(double deltaTime) {
    updateCursorBlink();
}

bool UITextInput::handleEvent(const UIEvent& event) {
    if (!m_visible || !m_enabled) return false;
    
    switch (event.type) {
        case UIEvent::MOUSE_MOVE: {
            bool wasHovered = m_isHovered;
            m_isHovered = contains(event.mouseX, event.mouseY);
            
            // 处理鼠标拖拽选择文本
            if (m_isDragging && m_isFocused) {
                float relativeX = event.mouseX - (m_x + m_padding);
                size_t newPos = getCharIndexAtPosition(nullptr, relativeX); // 需要传入vg上下文
                
                if (newPos != m_cursorPos) {
                    m_selectionEnd = newPos;
                    m_hasSelection = (m_selectionStart != m_selectionEnd);
                    m_cursorPos = newPos;
                }
                return true;
            }
            
            return m_isHovered != wasHovered;
        }
        
        case UIEvent::MOUSE_PRESS: {
            if (contains(event.mouseX, event.mouseY)) {
                if (!m_isFocused) {
                    setFocused(true);
                }
                
                // 设置光标位置并开始选择
                float relativeX = event.mouseX - (m_x + m_padding);
                size_t clickPos = getCharIndexAtPosition(nullptr, relativeX); // 需要传入vg上下文
                
                m_cursorPos = clickPos;
                m_selectionStart = clickPos;
                m_selectionEnd = clickPos;
                m_hasSelection = false;
                m_isDragging = true;
                
                return true;
            } else {
                if (m_isFocused) {
                    setFocused(false);
                }
            }
            break;
        }
        
        case UIEvent::MOUSE_RELEASE: {
            if (m_isDragging) {
                m_isDragging = false;
                return true;
            }
            break;
        }
        
        case UIEvent::KEY_PRESS: {
            if (m_isFocused) {
                handleKeyInput(event.keyCode, false, false);
                return true;
            }
            break;
        }
        // 在 handleEvent 方法的 switch 语句中添加
        case UIEvent::CHAR_INPUT: {
            if (m_isFocused && !m_readOnly) {
                handleCharInput(event.character);
                return true;
            }
            break;
        }
    }
    
    return false;
}

void UITextInput::setText(const std::string& text) {
    if (m_text != text) {
        m_text = text.substr(0, m_maxLength);
        m_cursorPos = std::min(m_cursorPos, m_text.length());
        clearSelection();
        
        if (m_onTextChanged) {
            m_onTextChanged(m_text);
        }
    }
}

void UITextInput::setFocused(bool focused) {
    if (m_isFocused != focused) {
        m_isFocused = focused;
        if (!focused) {
            clearSelection();
        }
        m_lastCursorBlink = std::chrono::steady_clock::now();
        m_cursorVisible = true;
        
        if (m_onFocusChanged) {
            m_onFocusChanged(focused);
        }
    }
}

void UITextInput::selectAll() {
    if (!m_text.empty()) {
        m_selectionStart = 0;
        m_selectionEnd = m_text.length();
        m_hasSelection = true;
        m_cursorPos = m_selectionEnd;
    }
}

void UITextInput::clearSelection() {
    m_hasSelection = false;
    m_selectionStart = 0;
    m_selectionEnd = 0;
}

void UITextInput::deleteSelection() {
    if (m_hasSelection) {
        size_t start = std::min(m_selectionStart, m_selectionEnd);
        size_t end = std::max(m_selectionStart, m_selectionEnd);
        
        m_text.erase(start, end - start);
        m_cursorPos = start;
        clearSelection();
        
        if (m_onTextChanged) {
            m_onTextChanged(m_text);
        }
    }
}

void UITextInput::insertText(const std::string& text) {
    if (m_readOnly) return;
    
    if (m_hasSelection) {
        deleteSelection();
    }
    
    if (m_text.length() + text.length() <= m_maxLength) {
        m_text.insert(m_cursorPos, text);
        m_cursorPos += text.length();
        
        if (m_onTextChanged) {
            m_onTextChanged(m_text);
        }
    }
}

void UITextInput::renderText(NVGcontext* vg) {
    nvgFontFace(vg, "default");  // 添加字体设置
    nvgFontSize(vg, m_fontSize);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    
    // 使用相对坐标
    float textY = m_height * 0.5f;
    float textX = m_padding;
    
    if (m_text.empty() && !m_isFocused) {
        // 显示占位符
        nvgFillColor(vg, m_placeholderColor);
        nvgText(vg, textX, textY, m_placeholder.c_str(), nullptr);
    } else {
        std::string displayText = getDisplayText();
        
        if (m_hasSelection && m_isFocused) {
            // 有选中文本时分段渲染
            size_t start = std::min(m_selectionStart, m_selectionEnd);
            size_t end = std::max(m_selectionStart, m_selectionEnd);
            
            // 渲染选中前的文本
            if (start > 0) {
                nvgFillColor(vg, m_textColor);
                std::string beforeText = displayText.substr(0, start);
                nvgText(vg, textX, textY, beforeText.c_str(), nullptr);
            }
            
            // 渲染选中的文本（使用反色）
            if (end > start) {
                float beforeWidth = start > 0 ? nvgTextBounds(vg, 0, 0, displayText.substr(0, start).c_str(), nullptr, nullptr) : 0;
                
                // 计算反色
                NVGcolor invertedColor;
                invertedColor.r = 1.0f - m_textColor.r;
                invertedColor.g = 1.0f - m_textColor.g;
                invertedColor.b = 1.0f - m_textColor.b;
                invertedColor.a = m_textColor.a;
                
                nvgFillColor(vg, invertedColor);
                std::string selectedText = displayText.substr(start, end - start);
                nvgText(vg, textX + beforeWidth, textY, selectedText.c_str(), nullptr);
            }
            
            // 渲染选中后的文本
            if (end < displayText.length()) {
                float beforeWidth = nvgTextBounds(vg, 0, 0, displayText.substr(0, end).c_str(), nullptr, nullptr);
                
                nvgFillColor(vg, m_textColor);
                std::string afterText = displayText.substr(end);
                nvgText(vg, textX + beforeWidth, textY, afterText.c_str(), nullptr);
            }
        } else {
            // 没有选中文本时正常渲染
            nvgFillColor(vg, m_textColor);
            nvgText(vg, textX, textY, displayText.c_str(), nullptr);
        }
    }
}

void UITextInput::renderCursor(NVGcontext* vg) {
    // 使用相对坐标
    float cursorX = m_padding + getCharPositionX(vg, m_cursorPos);
    float cursorY1 = m_padding;
    float cursorY2 = m_height - m_padding;
    
    nvgBeginPath(vg);
    nvgMoveTo(vg, cursorX, cursorY1);
    nvgLineTo(vg, cursorX, cursorY2);
    nvgStrokeColor(vg, m_cursorColor);
    nvgStrokeWidth(vg, 1.0f);
    nvgStroke(vg);
}

void UITextInput::renderSelection(NVGcontext* vg) {
    if (!m_hasSelection) return;
    
    size_t start = std::min(m_selectionStart, m_selectionEnd);
    size_t end = std::max(m_selectionStart, m_selectionEnd);
    
    // 使用相对坐标
    float startX = m_padding + getCharPositionX(vg, start);
    float endX = m_padding + getCharPositionX(vg, end);
    
    nvgBeginPath(vg);
    nvgRect(vg, startX, m_padding, endX - startX, m_height - 2 * m_padding);
    nvgFillColor(vg, m_selectionColor);
    nvgFill(vg);
}

void UITextInput::updateCursorBlink() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastCursorBlink).count() / 1000.0;
    
    if (elapsed >= CURSOR_BLINK_INTERVAL) {
        m_cursorVisible = !m_cursorVisible;
        m_lastCursorBlink = now;
    }
}

size_t UITextInput::getCharIndexAtPosition(NVGcontext* vg, float x) {
    if (!vg) {
        // 简化计算，每个字符约8像素宽
        if (x <= 0) return 0;
        size_t pos = (size_t)(x / 8);
        return std::min(pos, m_text.length());
    }
    
    nvgFontSize(vg, m_fontSize);
    std::string displayText = getDisplayText();
    
    float totalWidth = 0;
    for (size_t i = 0; i <= displayText.length(); ++i) {
        float charMidpoint = totalWidth;
        if (i < displayText.length()) {
            float charWidth = nvgTextBounds(vg, 0, 0, &displayText[i], &displayText[i + 1], nullptr);
            charMidpoint += charWidth * 0.5f;
        }
        
        if (x <= charMidpoint) {
            return i;
        }
        
        if (i < displayText.length()) {
            float charWidth = nvgTextBounds(vg, 0, 0, &displayText[i], &displayText[i + 1], nullptr);
            totalWidth += charWidth;
        }
    }
    
    return displayText.length();
}

float UITextInput::getCharPositionX(NVGcontext* vg, size_t index) {
    if (!vg || index == 0) return 0;
    
    nvgFontSize(vg, m_fontSize);
    std::string displayText = getDisplayText();
    
    if (index >= displayText.length()) {
        return nvgTextBounds(vg, 0, 0, displayText.c_str(), nullptr, nullptr);
    }
    
    std::string substr = displayText.substr(0, index);
    return nvgTextBounds(vg, 0, 0, substr.c_str(), nullptr, nullptr);
}

void UITextInput::moveCursor(int delta, bool selecting) {
    size_t newPos = m_cursorPos;
    
    if (delta < 0 && newPos > 0) {
        newPos = std::max(0, (int)newPos + delta);
    } else if (delta > 0 && newPos < m_text.length()) {
        newPos = std::min(m_text.length(), newPos + delta);
    }
    
    if (selecting) {
        if (!m_hasSelection) {
            m_selectionStart = m_cursorPos;
            m_hasSelection = true;
        }
        m_selectionEnd = newPos;
    } else {
        clearSelection();
    }
    
    m_cursorPos = newPos;
    m_lastCursorBlink = std::chrono::steady_clock::now();
    m_cursorVisible = true;
}

void UITextInput::handleKeyInput(int key, bool shift, bool ctrl) {
    // 使用GLFW按键常量
    switch (key) {
        case 263: // GLFW_KEY_LEFT
            moveCursor(-1, shift);
            break;
        case 262: // GLFW_KEY_RIGHT
            moveCursor(1, shift);
            break;
        case 268: // GLFW_KEY_HOME
            m_cursorPos = 0;
            if (!shift) clearSelection();
            break;
        case 269: // GLFW_KEY_END
            m_cursorPos = m_text.length();
            if (!shift) clearSelection();
            break;
        case 259: // GLFW_KEY_BACKSPACE
            if (m_hasSelection) {
                deleteSelection();
            } else if (m_cursorPos > 0) {
                m_text.erase(m_cursorPos - 1, 1);
                m_cursorPos--;
                if (m_onTextChanged) m_onTextChanged(m_text);
            }
            break;
        case 261: // GLFW_KEY_DELETE
            if (m_hasSelection) {
                deleteSelection();
            } else if (m_cursorPos < m_text.length()) {
                m_text.erase(m_cursorPos, 1);
                if (m_onTextChanged) m_onTextChanged(m_text);
            }
            break;
        case 257: // GLFW_KEY_ENTER
            if (m_onEnterPressed) {
                m_onEnterPressed(m_text);
            }
            break;
        case 65: // GLFW_KEY_A
            if (ctrl) {
                selectAll();
            }
            break;
    }
}

void UITextInput::handleCharInput(unsigned int codepoint) {
    if (codepoint >= 32 && codepoint < 127) { // 可打印ASCII字符
        char ch = static_cast<char>(codepoint);
        insertText(std::string(1, ch));
    }
}

std::string UITextInput::getDisplayText() const {
    if (m_isPassword && !m_text.empty()) {
        return std::string(m_text.length(), '*');
    }
    return m_text;
}