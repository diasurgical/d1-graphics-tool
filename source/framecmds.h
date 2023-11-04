#pragma once

#include "celview.h"
#include <QObject>
#include <QUndoCommand>

class RemoveFrameCommand : public QObject, public QUndoCommand {
    Q_OBJECT

public:
    explicit RemoveFrameCommand(D1Gfx *g, CelView *cv, QUndoCommand *parent = nullptr);
    ~RemoveFrameCommand() = default;

    void undo() override;
    void redo() override;

private:
    QPointer<D1Gfx> gfx;
    QPointer<CelView> celview;
    QImage img;
    int currentFrameIndex = 0;
};
