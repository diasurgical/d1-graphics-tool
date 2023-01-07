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

static bool validSquare(const D1GfxFrame *frame, QString &msg)
{
    for (int y = 0; y < MICRO_HEIGHT; y++) {
        for (int x = 0; x < MICRO_WIDTH; x++) {
            if (frame->getPixel(x, y).isTransparent()) {
                return prepareMsgTransparent(msg, x, y);
            }
        }
    }
    return true;
}

static bool validBottomLeftTriangle(const D1GfxFrame *frame, QString &msg)
{
    for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
        for (int x = 0; x < MICRO_WIDTH; x++) {
            if (frame->getPixel(x, y).isTransparent()) {
                if (x >= (y * 2 - MICRO_WIDTH)) {
                    return prepareMsgTransparent(msg, x, y);
                }
            } else {
                if (x < (y * 2 - MICRO_WIDTH)) {
                    return prepareMsgNonTransparent(msg, x, y);
                }
            }
        }
    }
    return true;
}

static bool validBottomRightTriangle(const D1GfxFrame *frame, QString &msg)
{
    for (int y = MICRO_HEIGHT / 2; y < MICRO_HEIGHT; y++) {
        for (int x = 0; x < MICRO_WIDTH; x++) {
            if (frame->getPixel(x, y).isTransparent()) {
                if (x < (2 * MICRO_WIDTH - y * 2)) {
                    return prepareMsgTransparent(msg, x, y);
                }
            } else {
                if (x >= (2 * MICRO_WIDTH - y * 2)) {
                    return prepareMsgNonTransparent(msg, x, y);
                }
            }
        }
    }
    return true;
}

static bool validLeftTriangle(const D1GfxFrame *frame, QString &msg)
{
    if (!validBottomLeftTriangle(frame, msg)) {
        return false;
    }
    for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
        for (int x = 0; x < MICRO_WIDTH; x++) {
            if (frame->getPixel(x, y).isTransparent()) {
                if (x >= (MICRO_WIDTH - y * 2)) {
                    return prepareMsgTransparent(msg, x, y);
                }
            } else {
                if (x < (MICRO_WIDTH - y * 2)) {
                    return prepareMsgNonTransparent(msg, x, y);
                }
            }
        }
    }
    return true;
}

static bool validRightTriangle(const D1GfxFrame *frame, QString &msg)
{
    if (!validBottomRightTriangle(frame, msg)) {
        return false;
    }
    for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
        for (int x = 0; x < MICRO_WIDTH; x++) {
            if (frame->getPixel(x, y).isTransparent()) {
                if (x < y * 2) {
                    return prepareMsgTransparent(msg, x, y);
                }
            } else {
                if (x >= y * 2) {
                    return prepareMsgNonTransparent(msg, x, y);
                }
            }
        }
    }
    return true;
}

static bool validTopHalfSquare(const D1GfxFrame *frame, QString &msg)
{
    for (int y = 0; y < MICRO_HEIGHT / 2; y++) {
        for (int x = 0; x < MICRO_WIDTH; x++) {
            if (frame->getPixel(x, y).isTransparent()) {
                return prepareMsgTransparent(msg, x, y);
            }
        }
    }
    return true;
}

static bool validLeftTrapezoid(const D1GfxFrame *frame, QString &msg)
{
    return validBottomLeftTriangle(frame, msg) && validTopHalfSquare(frame, msg);
}

static bool validRightTrapezoid(const D1GfxFrame *frame, QString &msg)
{
    return validBottomRightTriangle(frame, msg) && validTopHalfSquare(frame, msg);
}

void LevelTabFrameWidget::selectFrameType(D1GfxFrame *frame)
{
    D1CEL_FRAME_TYPE frameType = D1CEL_FRAME_TYPE::TransparentSquare;
    QString tmp;

    if (frame->getWidth() == MICRO_WIDTH && frame->getHeight() == MICRO_HEIGHT) {
        if (validSquare(frame, tmp)) {
            frameType = D1CEL_FRAME_TYPE::Square;
        } else if (validLeftTriangle(frame, tmp)) {
            frameType = D1CEL_FRAME_TYPE::LeftTriangle;
        } else if (validRightTriangle(frame, tmp)) {
            frameType = D1CEL_FRAME_TYPE::RightTriangle;
        } else if (validLeftTrapezoid(frame, tmp)) {
            frameType = D1CEL_FRAME_TYPE::LeftTrapezoid;
        } else if (validRightTrapezoid(frame, tmp)) {
            frameType = D1CEL_FRAME_TYPE::RightTrapezoid;
        }
    }

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
        switch (frame->getFrameType()) {
        case D1CEL_FRAME_TYPE::Square:
            validSquare(frame, error);
            break;
        case D1CEL_FRAME_TYPE::TransparentSquare:
            if (validSquare(frame, tmp)) {
                warning = "Suggested type: 'Square'";
                break;
            }
            if (validLeftTriangle(frame, tmp)) {
                warning = "Suggested type: 'Left Triangle'";
                break;
            }
            if (validRightTriangle(frame, tmp)) {
                warning = "Suggested type: 'Right Triangle'";
                break;
            }
            if (validLeftTrapezoid(frame, tmp)) {
                warning = "Suggested type: 'Left Trapezoid'";
                break;
            }
            if (validRightTrapezoid(frame, tmp)) {
                warning = "Suggested type: 'Right Trapezoid'";
                break;
            }
            break;
        case D1CEL_FRAME_TYPE::LeftTriangle:
            validLeftTriangle(frame, error);
            break;
        case D1CEL_FRAME_TYPE::RightTriangle:
            validRightTriangle(frame, error);
            break;
        case D1CEL_FRAME_TYPE::LeftTrapezoid:
            validLeftTrapezoid(frame, error);
            break;
        case D1CEL_FRAME_TYPE::RightTrapezoid:
            validRightTrapezoid(frame, error);
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
