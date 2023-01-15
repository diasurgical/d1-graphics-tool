#include "leveltabframewidget.h"

#include "d1gfx.h"
#include "levelcelview.h"
#include "ui_leveltabframewidget.h"

LevelTabFrameWidget::LevelTabFrameWidget()
    : QWidget(nullptr)
    , ui(new Ui::LevelTabFrameWidget)
{
    ui->setupUi(this);
}

LevelTabFrameWidget::~LevelTabFrameWidget()
{
    delete ui;
}

void LevelTabFrameWidget::initialize(LevelCelView *v, D1Gfx *g)
{
    this->levelCelView = v;
    this->gfx = g;
}

void LevelTabFrameWidget::update()
{
    int frameIdx = this->levelCelView->getCurrentFrameIndex();
    D1GfxFrame *frame = this->gfx->getFrame(frameIdx);

    bool hasFrame = frame != nullptr;

    this->ui->frameTypeComboBox->setEnabled(hasFrame);

    if (!hasFrame) {
        this->ui->frameTypeComboBox->setCurrentIndex(-1);
        return;
    }

    D1CEL_FRAME_TYPE frameType = frame->getFrameType();
    this->ui->frameTypeComboBox->setCurrentIndex((int)frameType);

    this->validate();
}

static bool prepareMsgTransparent(QString &msg, int x, int y)
{
    msg = QString("Invalid (transparent) pixel at (") + QString::number(x) + ":" + QString::number(y) + ")";
    return false;
}

static bool prepareMsgNonTransparent(QString &msg, int x, int y)
{
    msg = QString("Invalid (non-transparent) pixel at (") + QString::number(x) + ":" + QString::number(y) + ")";
    return false;
}

static bool validSquare(const D1GfxFrame *frame, QString &msg, int *limit)
{
    for (int y = 0; y < MICRO_HEIGHT; y++) {
        for (int x = 0; x < MICRO_WIDTH; x++) {
            if (frame->getPixel(x, y).isTransparent() && --*limit < 0) {
                return prepareMsgTransparent(msg, x, y);
            }
        }
    }
    return true;
}

static bool validBottomLeftTriangle(const D1GfxFrame *frame, QString &msg, int *limit)
{
    for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
        for (int x = 0; x < MICRO_WIDTH; x++) {
            if (frame->getPixel(x, y).isTransparent()) {
                if (x >= (y * 2 - MICRO_WIDTH) && --*limit < 0) {
                    return prepareMsgTransparent(msg, x, y);
                }
            } else {
                if (x < (y * 2 - MICRO_WIDTH) && --*limit < 0) {
                    return prepareMsgNonTransparent(msg, x, y);
                }
            }
        }
    }
    return true;
}

static bool validBottomRightTriangle(const D1GfxFrame *frame, QString &msg, int *limit)
{
    for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
        for (int x = 0; x < MICRO_WIDTH; x++) {
            if (frame->getPixel(x, y).isTransparent()) {
                if (x < (2 * MICRO_WIDTH - y * 2) && --*limit < 0) {
                    return prepareMsgTransparent(msg, x, y);
                }
            } else {
                if (x >= (2 * MICRO_WIDTH - y * 2) && --*limit < 0) {
                    return prepareMsgNonTransparent(msg, x, y);
                }
            }
        }
    }
    return true;
}

static bool validLeftTriangle(const D1GfxFrame *frame, QString &msg, int *limit)
{
    if (!validBottomLeftTriangle(frame, msg, limit)) {
        return false;
    }
    for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
        for (int x = 0; x < MICRO_WIDTH; x++) {
            if (frame->getPixel(x, y).isTransparent()) {
                if (x >= (MICRO_WIDTH - y * 2) && --*limit < 0) {
                    return prepareMsgTransparent(msg, x, y);
                }
            } else {
                if (x < (MICRO_WIDTH - y * 2) && --*limit < 0) {
                    return prepareMsgNonTransparent(msg, x, y);
                }
            }
        }
    }
    return true;
}

static bool validRightTriangle(const D1GfxFrame *frame, QString &msg, int *limit)
{
    if (!validBottomRightTriangle(frame, msg, limit)) {
        return false;
    }
    for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
        for (int x = 0; x < MICRO_WIDTH; x++) {
            if (frame->getPixel(x, y).isTransparent()) {
                if (x < y * 2 && --*limit < 0) {
                    return prepareMsgTransparent(msg, x, y);
                }
            } else {
                if (x >= y * 2 && --*limit < 0) {
                    return prepareMsgNonTransparent(msg, x, y);
                }
            }
        }
    }
    return true;
}

static bool validTopHalfSquare(const D1GfxFrame *frame, QString &msg, int *limit)
{
    for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
        for (int x = 0; x < MICRO_WIDTH; x++) {
            if (frame->getPixel(x, y).isTransparent() && --*limit < 0) {
                return prepareMsgTransparent(msg, x, y);
            }
        }
    }
    return true;
}

static bool validLeftTrapezoid(const D1GfxFrame *frame, QString &msg, int *limit)
{
    return validBottomLeftTriangle(frame, msg, limit) && validTopHalfSquare(frame, msg, limit);
}

static bool validRightTrapezoid(const D1GfxFrame *frame, QString &msg, int *limit)
{
    return validBottomRightTriangle(frame, msg, limit) && validTopHalfSquare(frame, msg, limit);
}

static bool validEmpty(const D1GfxFrame *frame, QString &msg, int *limit)
{
    for (int y = 0; y < MICRO_HEIGHT; y++) {
        for (int x = 0; x < MICRO_WIDTH; x++) {
            if (!frame->getPixel(x, y).isTransparent() && --*limit < 0) {
                return prepareMsgNonTransparent(msg, x, y);
            }
        }
    }
    return true;
}

D1CEL_FRAME_TYPE LevelTabFrameWidget::altFrameType(D1GfxFrame *frame, int *limit)
{
    D1CEL_FRAME_TYPE frameType = D1CEL_FRAME_TYPE::TransparentSquare;
    QString tmp;
    int limitSquare = *limit, limitLeftTriangle = *limit, limitRightTriangle = *limit, limitLeftTrapezoid = *limit, limitRightTrapezoid = *limit, limitEmpty = *limit;

    if (frame->getWidth() == MICRO_WIDTH && frame->getHeight() == MICRO_HEIGHT) {
        if (validSquare(frame, tmp, &limitSquare)) {
            frameType = D1CEL_FRAME_TYPE::Square;
            *limit = limitSquare;
        } else if (validLeftTriangle(frame, tmp, &limitLeftTriangle)) {
            frameType = D1CEL_FRAME_TYPE::LeftTriangle;
            *limit = limitLeftTriangle;
        } else if (validRightTriangle(frame, tmp, &limitRightTriangle)) {
            frameType = D1CEL_FRAME_TYPE::RightTriangle;
            *limit = limitRightTriangle;
        } else if (validLeftTrapezoid(frame, tmp, &limitLeftTrapezoid)) {
            frameType = D1CEL_FRAME_TYPE::LeftTrapezoid;
            *limit = limitLeftTrapezoid;
        } else if (validRightTrapezoid(frame, tmp, &limitRightTrapezoid)) {
            frameType = D1CEL_FRAME_TYPE::RightTrapezoid;
            *limit = limitRightTrapezoid;
        } else if (limitEmpty > 0 && validEmpty(frame, tmp, &limitEmpty)) {
            frameType = D1CEL_FRAME_TYPE::Empty;
            *limit = limitEmpty;
        }
    }
    return frameType;
}

void LevelTabFrameWidget::selectFrameType(D1GfxFrame *frame)
{
    int limit = 0;
    D1CEL_FRAME_TYPE frameType = LevelTabFrameWidget::altFrameType(frame, &limit);
    frame->setFrameType(frameType);
}

void LevelTabFrameWidget::validate()
{
    int frameIdx = this->levelCelView->getCurrentFrameIndex();
    D1GfxFrame *frame = this->gfx->getFrame(frameIdx);

    QString error, warning, tmp;

    if (frame->getWidth() != MICRO_WIDTH) {
        error = "Invalid width. Must be 32px wide.";
    } else if (frame->getHeight() != MICRO_HEIGHT) {
        error = "Invalid height. Must be 32px wide.";
    } else {
        int limit = 0;
        switch (frame->getFrameType()) {
        case D1CEL_FRAME_TYPE::Square:
            validSquare(frame, error, &limit);
            break;
        case D1CEL_FRAME_TYPE::TransparentSquare:
            if (validSquare(frame, tmp, &limit)) {
                warning = "Suggested type: 'Square'";
                break;
            }
            limit = 0;
            if (validLeftTriangle(frame, tmp, &limit)) {
                warning = "Suggested type: 'Left Triangle'";
                break;
            }
            limit = 0;
            if (validRightTriangle(frame, tmp, &limit)) {
                warning = "Suggested type: 'Right Triangle'";
                break;
            }
            limit = 0;
            if (validLeftTrapezoid(frame, tmp, &limit)) {
                warning = "Suggested type: 'Left Trapezoid'";
                break;
            }
            limit = 0;
            if (validRightTrapezoid(frame, tmp, &limit)) {
                warning = "Suggested type: 'Right Trapezoid'";
                break;
            }
            break;
        case D1CEL_FRAME_TYPE::LeftTriangle:
            validLeftTriangle(frame, error, &limit);
            break;
        case D1CEL_FRAME_TYPE::RightTriangle:
            validRightTriangle(frame, error, &limit);
            break;
        case D1CEL_FRAME_TYPE::LeftTrapezoid:
            validLeftTrapezoid(frame, error, &limit);
            break;
        case D1CEL_FRAME_TYPE::RightTrapezoid:
            validRightTrapezoid(frame, error, &limit);
            break;
        }
    }

    if (!error.isEmpty()) {
        this->ui->frameTypeMsgLabel->setText(error);
        this->ui->frameTypeMsgLabel->setStyleSheet("color: rgb(255, 0, 0);");
    } else if (!warning.isEmpty()) {
        this->ui->frameTypeMsgLabel->setText(warning);
        this->ui->frameTypeMsgLabel->setStyleSheet("color: rgb(0, 255, 0);");
    } else {
        this->ui->frameTypeMsgLabel->setText("");
    }
}

void LevelTabFrameWidget::on_frameTypeComboBox_activated(int index)
{
    int frameIdx = this->levelCelView->getCurrentFrameIndex();
    D1GfxFrame *frame = this->gfx->getFrame(frameIdx);

    frame->setFrameType((D1CEL_FRAME_TYPE)index);

    this->validate();
}
