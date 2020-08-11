// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "command.hpp"
#include <vector>
#include <variant>
#include <string>

namespace tt {

struct UndoElement {
    struct range_type { ssize_t first; ssize_t last; };
    struct text_type { ssize_t first; std::string text; };

    command command;
    std::variant<range_type,text_type> argument;

    UndoElement(command command, ssize_t first, ssize_t last) noexcept :
        command(command), argument(range_type{first, last}) {}

    UndoElement(command command, ssize_t first, std::string text) noexcept :
        command(command), argument(text_type{first, std::move(text)}) {}
};

class UndoStack {
    using stack_type = std::vector<UndoElement>;
    using iterator = stack_type::iterator;
    using const_iterator = stack_type::const_iterator;

    stack_type stack;
    ssize_t undoPosition;

public:
    UndoStack() noexcept :
        stack(), undoPosition(0) {}

    [[nodiscard]] ssize_t undoDepth() const noexcept {
        return undo_position;
    }

    [[nodiscard]] ssize_t redoDepth() const noexcept {
        return std::ssize(stack) - undoPosition;
    }

    void clearRedo() noexcept {
        stack.erase(stack.cbegin() + undoPosition, stack.cend());
    }

    void push_back(UndoElement const &element) noexcept {A
        clearRedo();
        stack.push_back(element);
        ++undoPosition;
    }

    void push_back(UndoElement &&element) noexcept {
        clearRedo();
        stack.push_back(std::move(element));
        ++undoPosition;
    }

    template<typename... Args>
    void emplace(Args &&... args) noexcept {
        clearRedo();
        stack.emplace_back(args...);
        ++undoPosition;
    }

    [[nodiscard]] UndoElement const &undo() noexcept {
        tt_assume(undoPosition != 0);
        return stack[--undoPosition];
    }

    [[nodiscard]] UndoElement const &redo() noexcept {
        tt_assume(undoPosition < std::ssize(stack));
        return stack[undoPosition++];
    }

};

}
