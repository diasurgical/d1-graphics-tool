#include "leveltabtilewidget.h"

#include "d1amp.h"
#include "d1min.h"
#include "d1til.h"
#include "levelcelview.h"
#include "ui_leveltabtilewidget.h"

LevelTabTileWidget::LevelTabTileWidget()
    : QWidget(nullptr)
    , ui(new Ui::LevelTabTileWidget)
{
    ui->setupUi(this);
}

LevelTabTileWidget::~LevelTabTileWidget()
{
    delete ui;
}

void LevelTabTileWidget::initialize(LevelCelView *v, D1Til *t, D1Min *m, D1Amp *a)
{
    this->levelCelView = v;
    this->til = t;
    this->min = m;
    this->amp = a;
}

void LevelTabTileWidget::update()
{
    this->onUpdate = true;

    bool hasTile = this->til->getTileCount() != 0;

    this->ui->ampTypeComboBox->setEnabled(hasTile);

    this->ui->amp0->setEnabled(hasTile);
    this->ui->amp1->setEnabled(hasTile);
    this->ui->amp2->setEnabled(hasTile);
    this->ui->amp3->setEnabled(hasTile);
    this->ui->amp4->setEnabled(hasTile);
    this->ui->amp5->setEnabled(hasTile);
    this->ui->amp6->setEnabled(hasTile);
    this->ui->amp7->setEnabled(hasTile);

    if (!hasTile) {
        this->ui->ampTypeComboBox->setCurrentIndex(-1);

        this->ui->amp0->setChecked(false);
        this->ui->amp1->setChecked(false);
        this->ui->amp2->setChecked(false);
        this->ui->amp3->setChecked(false);
        this->ui->amp4->setChecked(false);
        this->ui->amp5->setChecked(false);
        this->ui->amp6->setChecked(false);
        this->ui->amp7->setChecked(false);

        this->onUpdate = false;
        return;
    }

    int tileIdx = this->levelCelView->getCurrentTileIndex();
    quint8 ampType = this->amp->getTileType(tileIdx);
    quint8 ampProperty = this->amp->getTileProperties(tileIdx);

    // update the combo box of the amp-type
    this->ui->ampTypeComboBox->setCurrentIndex(ampType);
    // update the checkboxes
    this->ui->amp0->setChecked((ampProperty & 1 << 0) != 0);
    this->ui->amp1->setChecked((ampProperty & 1 << 1) != 0);
    this->ui->amp2->setChecked((ampProperty & 1 << 2) != 0);
    this->ui->amp3->setChecked((ampProperty & 1 << 3) != 0);
    this->ui->amp4->setChecked((ampProperty & 1 << 4) != 0);
    this->ui->amp5->setChecked((ampProperty & 1 << 5) != 0);
    this->ui->amp6->setChecked((ampProperty & 1 << 6) != 0);
    this->ui->amp7->setChecked((ampProperty & 1 << 7) != 0);

    this->onUpdate = false;
}

void LevelTabTileWidget::updateAmpType()
{
    int tileIdx = this->levelCelView->getCurrentTileIndex();
    quint8 index = this->readAmpType();

    this->amp->setTileType(tileIdx, index);
}

void LevelTabTileWidget::updateAmpProperty()
{
    int tileIdx = this->levelCelView->getCurrentTileIndex();
    quint8 flags = this->readAmpProperty();

    this->amp->setTileProperties(tileIdx, flags);
}

quint8 LevelTabTileWidget::readAmpProperty()
{
    quint8 flags = 0;
    if (this->ui->amp0->checkState())
        flags |= 1 << 0;
    if (this->ui->amp1->checkState())
        flags |= 1 << 1;
    if (this->ui->amp2->checkState())
        flags |= 1 << 2;
    if (this->ui->amp3->checkState())
        flags |= 1 << 3;
    if (this->ui->amp4->checkState())
        flags |= 1 << 4;
    if (this->ui->amp5->checkState())
        flags |= 1 << 5;
    if (this->ui->amp6->checkState())
        flags |= 1 << 6;
    if (this->ui->amp7->checkState())
        flags |= 1 << 7;
    return flags;
}

quint8 LevelTabTileWidget::readAmpType()
{
    return this->ui->ampTypeComboBox->currentIndex();
}

void LevelTabTileWidget::on_ampTypeComboBox_activated(int index)
{
    if (!this->onUpdate) {
        this->updateAmpType();
    }
}

void LevelTabTileWidget::on_amp0_clicked()
{
    this->updateAmpProperty();
}

void LevelTabTileWidget::on_amp1_clicked()
{
    this->updateAmpProperty();
}

void LevelTabTileWidget::on_amp2_clicked()
{
    this->updateAmpProperty();
}

void LevelTabTileWidget::on_amp3_clicked()
{
    this->updateAmpProperty();
}

void LevelTabTileWidget::on_amp4_clicked()
{
    this->updateAmpProperty();
}

void LevelTabTileWidget::on_amp5_clicked()
{
    this->updateAmpProperty();
}

void LevelTabTileWidget::on_amp6_clicked()
{
    this->updateAmpProperty();
}

void LevelTabTileWidget::on_amp7_clicked()
{
    this->updateAmpProperty();
}
