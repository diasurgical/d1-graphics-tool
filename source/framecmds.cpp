#include "framecmds.h"

RemoveFrameCommand::RemoveFrameCommand(D1Gfx *g, CelView *cv, QUndoCommand *parent)
    : gfx(g)
    , celview(cv)
    , QUndoCommand(parent)
{
}

void RemoveFrameCommand::undo()
{
}

void RemoveFrameCommand::redo()
{
}
