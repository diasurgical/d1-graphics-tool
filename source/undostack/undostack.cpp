#include "undostack.h"
#include <QDebug>

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

    eraseRedundantCmds();

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

    /* If current processed command has a macroID higher than 0, then it means it's a macro.
     * So we need to start going through commands backwards in a loop
     */
    unsigned int macroID = m_cmds[m_undoPos]->macroID();
    if (macroID > 0) {
        emit initializeWidget(m_macros[macroID - 1].userdata(), OperationType::Undo);

        while (m_cmds[m_undoPos]->macroID() > 0) {
            bool result = false;
            m_cmds[m_undoPos]->undo();
            m_undoPos--;
            emit updateWidget(result);
            if (result || m_undoPos < 0 || m_cmds[m_undoPos + 1]->macroID() != m_cmds[m_undoPos]->macroID()) {
                break;
            }
        }
    } else {
        m_cmds[m_undoPos]->undo();
        m_undoPos--;
    }

    if (m_undoPos < 0)
        m_canUndo = false;

    m_canRedo = true;
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

    unsigned int macroID = m_cmds[m_undoPos + 1]->macroID();
    if (macroID > 0) {
        emit initializeWidget(m_macros[macroID - 1].userdata(), OperationType::Redo);

        while (m_cmds[m_undoPos + 1]->macroID() > 0) {
            bool result = false;
            m_cmds[m_undoPos + 1]->redo();
            m_undoPos++;
            emit updateWidget(result);
            if (result || m_undoPos + 1 >= m_cmds.size() || m_cmds[m_undoPos + 1]->macroID() != m_cmds[m_undoPos]->macroID()) {
                break;
            }
        }
    } else {
        m_cmds[m_undoPos + 1]->redo();
        m_undoPos++;
    }

    m_canUndo = true;

    if (m_undoPos + 1 >= m_cmds.size())
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
    m_undoPos = -1;
    m_canUndo = m_canRedo = false;
    m_cmds.clear();
    m_macros.clear();
}

/**
 * @brief Adds a macro to undo stack
 *
 * This function is being called whenever someone wants to insert
 * a macro in the undo stack. It calls redo() on all commands contained
 * in the macro, and updates undo stack position accordingly, it also
 * sets macroIDs depending on the macros' vector size.
 *
 */
void UndoStack::addMacro(UndoMacroFactory &macroFactory)
{
    eraseRedundantCmds();

    emit initializeWidget(macroFactory.userdata(), OperationType::Redo);
    m_macros.emplace_back(std::move(macroFactory.userdata()), std::make_pair<int, int>(m_cmds.size(), (m_cmds.size() + macroFactory.cmds().size()) - 1));

    for (auto &cmd : macroFactory.cmds()) {
        bool result = false;
        cmd->redo();
        m_undoPos++;

        emit updateWidget(result);
        if (result) {
            m_canRedo = true;
            break;
        }
    }

    // For each command that will be inserted set a macroID so it is located in the same span
    std::for_each(macroFactory.cmds().begin(), macroFactory.cmds().end(), [&](const std::unique_ptr<Command> &cmd) {
        cmd->setMacroID(m_macros.size());
    });

    m_cmds.insert(m_cmds.end(), std::make_move_iterator(macroFactory.cmds().begin()), std::make_move_iterator(macroFactory.cmds().end()));
    m_canUndo = true;
}

/**
 * @brief Erases redundant commands and macros from the undo stack
 *
 * This function erases redundant commands and macros from both vectors
 * in undostack. Redundant in this case means commands which will no longer
 * be available, i.e. command being obsolete (having obsolete flag set to true), or
 * all commands + macros after currently pushed command - upon insertion undo stack removes all
 * commands + macros that are after current undo stack position (were undo'd) and are possible
 * to redo.
 *
 */
void UndoStack::eraseRedundantCmds()
{
    // Erase any command that was set to obsolete
    std::erase_if(m_cmds, [](const auto &cmd) { return cmd->isObsolete(); });

    if (m_cmds.begin() + (m_undoPos + 1) < m_cmds.end()) {
        // Drop any command that's after currently undo'd index
        m_cmds.erase(m_cmds.begin() + (m_undoPos + 1), m_cmds.end());

        // Drop any macro that's after currently undo'd index
        std::erase_if(m_macros, [&](const auto &macro) { return macro.beginIndex() > m_undoPos; });

        // If undoPos is currently on a macro, then update it's ending index because we could have removed some of
        // it's commands
        unsigned int macroID = m_cmds[m_undoPos]->macroID();
        if (macroID > 0)
            m_macros[macroID - 1].setLastIndex(m_undoPos);
    }
}
