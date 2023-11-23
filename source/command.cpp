#include "command.h"

void Command::setObsolete(bool isObsolete)
{
    m_isObsolete = isObsolete;
}

bool Command::isObsolete() const
{
    return m_isObsolete;
}
