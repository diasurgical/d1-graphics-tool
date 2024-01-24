#pragma once

#include "command.h"
#include "undomacro.h"

#include <QObject>
#include <array>
#include <memory>
#include <utility>
#include <vector>

enum OperationType {
    Undo,
    Redo
};

class UndoStack : public QObject {
    Q_OBJECT

public:
    UndoStack() = default;
    ~UndoStack() = default;

    void push(std::unique_ptr<Command> cmd);

    void undo();
    void redo();
    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;

    void clear();
    void addMacro(UndoMacroFactory &macroFactory);

signals:
    void updateWidget(bool &userCancelled);
    void initializeWidget(std::unique_ptr<UserData> &userData, enum OperationType opType);

private:
    bool m_canUndo = false;
    bool m_canRedo = false;
    int8_t m_undoPos { -1 };
    std::vector<std::unique_ptr<Command>> m_cmds; // holds all the commands on the stack
    std::vector<UndoMacro> m_macros;

    void eraseRedundantCmds();
};
