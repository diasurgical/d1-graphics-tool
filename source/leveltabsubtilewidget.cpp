#include "leveltabsubtilewidget.h"

#include "levelcelview.h"
#include "ui_leveltabsubtilewidget.h"

LevelTabSubTileWidget::LevelTabSubTileWidget()
    : QWidget(nullptr)
    , ui(new Ui::LevelTabSubTileWidget)
{
    ui->setupUi(this);
}

LevelTabSubTileWidget::~LevelTabSubTileWidget()
{
    delete ui;
}

void LevelTabSubTileWidget::initialize(LevelCelView *v, D1Gfx *g, D1Min *m, D1Sol *s)
{
    this->levelCelView = v;
    this->gfx = g;
    this->min = m;
    this->sol = s;
}

void LevelTabSubTileWidget::update()
{
    this->onUpdate = true;

    bool hasSubtile = this->min->getSubtileCount() != 0;

    this->ui->sol0->setEnabled(hasSubtile);
    this->ui->sol1->setEnabled(hasSubtile);
    this->ui->sol2->setEnabled(hasSubtile);
    this->ui->sol3->setEnabled(hasSubtile);
    this->ui->sol4->setEnabled(hasSubtile);
    this->ui->sol5->setEnabled(hasSubtile);
    this->ui->sol7->setEnabled(hasSubtile);

    this->ui->framesComboBox->setEnabled(hasSubtile);

    if (!hasSubtile) {
        this->ui->sol0->setChecked(false);
        this->ui->sol1->setChecked(false);
        this->ui->sol2->setChecked(false);
        this->ui->sol3->setChecked(false);
        this->ui->sol4->setChecked(false);
        this->ui->sol5->setChecked(false);
        this->ui->sol7->setChecked(false);

        this->ui->framesComboBox->setCurrentIndex(-1);
        this->ui->framesComboBox->setEnabled(false);
        this->ui->framesPrevButton->setEnabled(false);
        this->ui->framesNextButton->setEnabled(false);

        this->onUpdate = false;
        return;
    }

    int subtileIdx = this->levelCelView->getCurrentSubtileIndex();
    quint8 sol = this->sol->getSubtileProperties(subtileIdx);
    QList<quint16> &frames = this->min->getCelFrameIndices(subtileIdx);

    this->ui->sol0->setChecked((sol & 1 << 0) != 0);
    this->ui->sol1->setChecked((sol & 1 << 1) != 0);
    this->ui->sol2->setChecked((sol & 1 << 2) != 0);
    this->ui->sol3->setChecked((sol & 1 << 3) != 0);
    this->ui->sol4->setChecked((sol & 1 << 4) != 0);
    this->ui->sol5->setChecked((sol & 1 << 5) != 0);
    this->ui->sol7->setChecked((sol & 1 << 7) != 0);
    // update combo box of the frames
    while (this->ui->framesComboBox->count() > frames.count())
        this->ui->framesComboBox->removeItem(0);
    int i = 0;
    while (this->ui->framesComboBox->count() < frames.count())
        this->ui->framesComboBox->insertItem(0, QString::number(++i));
    for (i = 0; i < frames.count(); i++) {
        this->ui->framesComboBox->setItemText(i, QString::number(frames[i]));
    }
    if (this->lastSubtileIndex != subtileIdx) {
        this->lastSubtileIndex = subtileIdx;
        this->lastFrameEntryIndex = 0;
        this->ui->framesComboBox->setCurrentIndex(0);
    }
    this->updateFramesSelection(this->lastFrameEntryIndex);

    this->onUpdate = false;
}

void LevelTabSubTileWidget::updateFramesSelection(int index)
{
    this->lastFrameEntryIndex = index;
    int frameIdx = this->ui->framesComboBox->currentText().toInt();

    this->ui->framesPrevButton->setEnabled(frameIdx > 0);
    this->ui->framesNextButton->setEnabled(frameIdx < this->gfx->getFrameCount() - 1);
}

void LevelTabSubTileWidget::updateSolProperty()
{
    int subTileIdx = this->levelCelView->getCurrentSubtileIndex();
    quint8 flags = this->readSol();

    this->sol->setSubtileProperties(subTileIdx, flags);
}

quint8 LevelTabSubTileWidget::readSol()
{
    quint8 flags = 0;
    if (this->ui->sol0->checkState())
        flags |= 1 << 0;
    if (this->ui->sol1->checkState())
        flags |= 1 << 1;
    if (this->ui->sol2->checkState())
        flags |= 1 << 2;
    if (this->ui->sol3->checkState())
        flags |= 1 << 3;
    if (this->ui->sol4->checkState())
        flags |= 1 << 4;
    if (this->ui->sol5->checkState())
        flags |= 1 << 5;
    if (this->ui->sol7->checkState())
        flags |= 1 << 7;
    return flags;
}

void LevelTabSubTileWidget::on_sol0_clicked()
{
    this->updateSolProperty();
}

void LevelTabSubTileWidget::on_sol1_clicked()
{
    this->updateSolProperty();
}

void LevelTabSubTileWidget::on_sol2_clicked()
{
    this->updateSolProperty();
}

void LevelTabSubTileWidget::on_sol3_clicked()
{
    this->updateSolProperty();
}

void LevelTabSubTileWidget::on_sol4_clicked()
{
    this->updateSolProperty();
}

void LevelTabSubTileWidget::on_sol5_clicked()
{
    this->updateSolProperty();
}

void LevelTabSubTileWidget::on_sol7_clicked()
{
    this->updateSolProperty();
}

void LevelTabSubTileWidget::on_framesPrevButton_clicked()
{
    int index = this->ui->framesComboBox->currentIndex();
    int subtileIdx = this->levelCelView->getCurrentSubtileIndex();
    QList<quint16> &frameIndices = this->min->getCelFrameIndices(subtileIdx);
    int frameIdx = frameIndices[index] - 1;

    if (frameIdx > this->gfx->getFrameCount() - 1) {
        frameIdx = this->gfx->getFrameCount() - 1;
    }

    frameIndices[index] = frameIdx;

    // this->ui->subtilesComboBox->setItemText(index, QString::number(frameIdx));
    // this->updateFramesSelection(index);

    this->levelCelView->displayFrame();
}

void LevelTabSubTileWidget::on_framesComboBox_activated(int index)
{
    if (!this->onUpdate) {
        this->updateFramesSelection(index);
    }
}

void LevelTabSubTileWidget::on_framesComboBox_currentTextChanged(const QString &arg1)
{
    int index = this->lastFrameEntryIndex;
    int frameIdx = this->ui->framesComboBox->currentText().toInt();

    if (this->onUpdate || this->ui->framesComboBox->currentIndex() != index)
        return; // on update or side effect of combobox activated -> ignore

    if (frameIdx >= 0 && frameIdx < this->gfx->getFrameCount()) {
        int subtileIdx = this->levelCelView->getCurrentSubtileIndex();

        this->min->getCelFrameIndices(subtileIdx)[index] = frameIdx;

        // this->ui->subtilesComboBox->setItemText(index, QString::number(frameIdx));
        // this->updateFramesSelection(index);

        this->levelCelView->displayFrame();
    }
}

void LevelTabSubTileWidget::on_framesNextButton_clicked()
{
    int index = this->ui->framesComboBox->currentIndex();
    int subtileIdx = this->levelCelView->getCurrentSubtileIndex();
    QList<quint16> &frameIndices = this->min->getCelFrameIndices(subtileIdx);
    int frameIdx = frameIndices[index] + 1;

    if (frameIdx < 0) {
        frameIdx = 0;
    }

    frameIndices[index] = frameIdx;

    // this->ui->subtilesComboBox->setItemText(index, QString::number(frameIdx));
    // this->updateFramesSelection(index);

    this->levelCelView->displayFrame();
}
