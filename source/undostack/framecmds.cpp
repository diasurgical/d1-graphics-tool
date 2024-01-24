#include <QImageReader>

#include <stdexcept>

#include "framecmds.h"
#include "mainwindow.h"

RemoveFrameCommand::RemoveFrameCommand(int currentFrameIndex, const QImage img)
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

ReplaceFrameCommand::ReplaceFrameCommand(int currentFrameIndex, const QImage imgToReplace, const QImage imgToRestore)
    : frameIndexToReplace(currentFrameIndex)
    , imgToReplace(imgToReplace)
    , imgToRestore(imgToRestore)
{
}

void ReplaceFrameCommand::undo()
{
    emit this->undoReplaced(frameIndexToReplace, imgToRestore);
}

void ReplaceFrameCommand::redo()
{
    // emit this signal which will call LevelCelView/CelView::replaceCurrentFrame
    emit this->replaced(frameIndexToReplace, imgToReplace);
}

AddFrameCommand::AddFrameCommand(int index, QImage &img, IMAGE_FILE_MODE mode)
    : m_index(index)
    , m_image(std::move(img))
    , m_mode(mode)
{
}

void AddFrameCommand::undo()
{
    emit this->undoAdded(m_index);
}

void AddFrameCommand::redo()
{
    emit this->added(m_index, m_image, m_mode);
}
