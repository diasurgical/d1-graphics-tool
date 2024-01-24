#include "command.h"

/**
 * @brief Sets if a command is obsolete or not
 *
 * This function sets if a command is obsolete or not.
 * This flag will be used in UndoStack to check if a
 * command should be popped whenever there will be undo/redo
 * operation used, or user pushes any command onto the stack.
 */
void Command::setObsolete(bool isObsolete)
{
    m_isObsolete = isObsolete;
}

/**
 * @brief Returns if a command is obsolete or not
 */
bool Command::isObsolete() const
{
    return m_isObsolete;
}

void Command::setMacroID(unsigned int macroID)
{
    m_macroID = macroID;
}

unsigned int Command::macroID() const
{
    return m_macroID;
}
