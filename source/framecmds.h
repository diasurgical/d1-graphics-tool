#pragma once

#include "celview.h"
#include <QObject>
#include <QUndoCommand>

class RemoveFrameCommand : public QObject, public QUndoCommand {
    Q_OBJECT

public:
    explicit RemoveFrameCommand(int currentFrameIndex, const QImage img, QUndoCommand *parent = nullptr);
    ~RemoveFrameCommand() = default;

    void undo() override;
    void redo() override;

signals:
    void removed(int idxToRemove);
    void inserted(int idxToRestore, const QImage imgToRestore);

private:
    QImage imgToRevert;
    int frameIndexToRevert = 0;
};

class ReplaceFrameCommand : public QObject, public QUndoCommand {
    Q_OBJECT

public:
    explicit ReplaceFrameCommand(int currentFrameIndex, const QImage imgToReplace, const QImage imgToRestore, QUndoCommand *parent = nullptr);
    ~ReplaceFrameCommand() = default;

    void undo() override;
    void redo() override;

signals:
    void undoReplaced(int idxToRemove, const QImage imgToRestore);
    void replaced(int idxToReplace, const QImage imgToReplace);

private:
    QImage imgToReplace;
    QImage imgToRestore;
    int frameIndexToReplace = 0;
};

class AddFrameCommand : public QObject, public QUndoCommand {
    Q_OBJECT

public:
    explicit AddFrameCommand(IMAGE_FILE_MODE mode, int index, const QString imagefilePath, QUndoCommand *parent = nullptr);
    ~AddFrameCommand() = default;

    void undo() override;
    void redo() override;

signals:
    void undoAdded(int startingIndex, int endingIndex);
    void added(int startingIndex, const std::vector<QImage> &images, IMAGE_FILE_MODE mode);

private:
    std::vector<QImage> images;
    int startingIndex = 0;
    int endingIndex = 0;
    IMAGE_FILE_MODE mode;
};