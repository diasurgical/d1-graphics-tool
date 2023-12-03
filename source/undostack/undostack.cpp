#include "undostack.h"

/**
 * @brief Pushes new commands onto the commands stack (Undostack)
 *
 * This function pushes new commands onto the undo stack, and redos
 * the command that got currently pushed, essentially triggering the command.
 * It also erases any commands that had isObsolete flag set.
 * Push also removes any commands that were currently undo'd on the stack.
 */
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

/**
 * @brief Undos command that is currently the first one on the stack
 *
 * This function undos the command that is currently on the stack, setting
 * redo option to be available at the same time. It also pops any commands that
 * have the isObsolete flag set.
 */
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

/**
 * @brief Redos command that is currently the first one on the stack
 *
 * This function redos the command that is currently on the stack, setting
 * undo option to be available at the same time. It also pops any commands that
 * have the isObsolete flag set.
 */
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

/**
 * @brief Returns if stack can currently redo
 */
bool UndoStack::canRedo() const
{
    return m_canRedo;
}

/**
 * @brief Returns if stack can currently undo
 */
bool UndoStack::canUndo() const
{
    return m_canUndo;
}

/**
 * @brief Sets undo stack to a starting position
 *
 * This function clears undo stack to a starting position. To be
 * exact it sets it's position to 0, clears any commands and disables
 * flags that inform if stack can currently undo or redo.
 */
void UndoStack::clear()
{
    m_undoPos = 0;
    m_canUndo = m_canRedo = false;
    m_cmds.clear();
}
