#include <QImageReader>

#include <stdexcept>

#include "framecmds.h"
#include "mainwindow.h"

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

AddFrameCommand::AddFrameCommand(IMAGE_FILE_MODE mode, int index, const QString imagefilePath, QUndoCommand *parent)
    : startingIndex(index)
    , mode(mode)
{
    QImageReader reader = QImageReader(imagefilePath);
    int numImages = 0;

    // FIXME: this loop should have some sort of a progress bar, we support
    // status bar, but if user loads a .gif which could contain up to hundreds
    // of frames, loading might take quite a bit
    while (true) {
        QImage image = reader.read();
        if (image.isNull()) {
            break;
        }

        images.emplace_back(image);
        numImages++;
    }

    if (mode != IMAGE_FILE_MODE::AUTO && numImages == 0) {
        throw std::exception();
    }

    endingIndex = startingIndex + numImages;
}

void AddFrameCommand::undo()
{
    emit this->undoAdded(startingIndex, endingIndex);
}

void AddFrameCommand::redo()
{
    emit this->added(startingIndex, images, mode);
}
