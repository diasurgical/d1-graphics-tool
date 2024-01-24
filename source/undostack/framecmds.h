#pragma once

#include "command.h"
#include "views/celview.h"

#include <QObject>

class RemoveFrameCommand : public QObject, public Command {
    Q_OBJECT

public:
    explicit RemoveFrameCommand(int currentFrameIndex, const QImage img);
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

class ReplaceFrameCommand : public QObject, public Command {
    Q_OBJECT

public:
    explicit ReplaceFrameCommand(int currentFrameIndex, const QImage imgToReplace, const QImage imgToRestore);
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

class AddFrameCommand : public QObject, public Command {
    Q_OBJECT

public:
    explicit AddFrameCommand(int index, QImage &image, IMAGE_FILE_MODE mode);
    ~AddFrameCommand() = default;

    void undo() override;
    void redo() override;

signals:
    void undoAdded(int index);
    void added(int index, const QImage &image, IMAGE_FILE_MODE mode);

private:
    QImage m_image;
    int m_index = 0;
    IMAGE_FILE_MODE m_mode;
};
