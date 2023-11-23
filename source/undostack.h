#pragma once

#include "command.h"

#include <array>
#include <memory>
#include <vector>

class UndoStack {
public:
    UndoStack() = default;
    ~UndoStack() = default;

    void push(std::unique_ptr<Command> cmd);

    void undo();
    void redo();
    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;

    void clear();

private:
    bool m_canUndo = false;
    bool m_canRedo = false;
    int8_t m_undoPos = 0;
    std::vector<std::unique_ptr<Command>> m_cmds; // holds all the commands on the stack
};
