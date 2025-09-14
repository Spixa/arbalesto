#pragma once

#include <stack>
#include <optional>
#include <cstdint>

enum class UIWidget : uint32_t {
    CHAT = 1,
    INVENTORY,
    SETTINGS
};

class FocusStack {
public:
    using WidgetId = UIWidget;

    void push(WidgetId id) {
        stack.push(id);
    }

    void pop(WidgetId id) {
        if (!stack.empty() && stack.top() == id) {
            stack.pop();
        } else {
            // remove from anywhere else logic
            std::stack<WidgetId> tmp;

            // remake the stack without this widget
            while (!stack.empty()) {
                if (stack.top() != id) {
                    tmp.push(stack.top());
                }
                stack.pop();
            }

            // rewrite the stack
            while (!tmp.empty()) {
                stack.push(tmp.top());
                tmp.pop();
            }
        }
    }


    bool shouldWorldFocus() const {
        return stack.empty();
    }

    std::optional<WidgetId> topmost() const {
        if (stack.empty()) return std::nullopt;

        return stack.top();
    }

private:
    std::stack<WidgetId> stack;
};