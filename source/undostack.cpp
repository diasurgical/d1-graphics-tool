#include "undostack.h"

void UndoStack::push(std::unique_ptr<Command> cmd)
{
    try {
        cmd->redo();
    } catch (...) {
        /* TODO: here we we will start to implement proper
         * exception handling, but first we need to implement stuff
         * that will use it
         */
    }

    // Erase any command that was set to obsolete
    std::erase_if(m_cmds, [](const auto &cmd) { return cmd->isObsolete(); });

    // Drop any command that's after currently undo'd index
    if (m_cmds.begin() + (m_undoPos + 1) <= m_cmds.end())
        m_cmds.erase(m_cmds.begin() + (m_undoPos + 1), m_cmds.end());

    m_cmds.push_back(std::move(cmd));
    m_canUndo = true;
    m_canRedo = false;
    m_undoPos = m_cmds.size() - 1;
}

void UndoStack::undo()
{
    // Erase any command that was previously set as obsolete
    std::erase_if(m_cmds, [](const auto &cmd) { return cmd->isObsolete(); });

    if (m_undoPos == 0)
        m_canUndo = false;

    m_cmds[m_undoPos]->undo();

    m_canRedo = true;
    m_undoPos--;
}

void UndoStack::redo()
{
    // erase any command that was previously set as obsolete
    std::erase_if(m_cmds, [](const auto &cmd) { return cmd->isObsolete(); });

    m_cmds[m_undoPos + 1]->redo();

    m_undoPos++;
    m_canUndo = true;

    if (m_undoPos + 1 > m_cmds.size() - 1)
        m_canRedo = false;
}

bool UndoStack::canRedo() const
{
    return m_canRedo;
}

bool UndoStack::canUndo() const
{
    return m_canUndo;
}

void UndoStack::clear()
{
    m_undoPos = 0;
    m_canUndo = m_canRedo = false;
    m_cmds.clear();
}
