#include "framecmds.h"

RemoveFrameCommand::RemoveFrameCommand(int currentFrameIndex, const QImage img, QUndoCommand *parent)
    : frameIndexToRevert(currentFrameIndex)
    , imgToRevert(img)
{
}

void RemoveFrameCommand::undo()
{
    emit this->inserted(frameIndexToRevert, imgToRevert);
}

void RemoveFrameCommand::redo()
{
    // emit this signal which will call LevelCelView/CelView::removeCurrentFrame
    emit this->removed(frameIndexToRevert);
}
