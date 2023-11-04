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
