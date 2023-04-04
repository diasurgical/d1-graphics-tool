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

    if (!hasSubtile) {
        this->ui->sol0->setChecked(false);
        this->ui->sol1->setChecked(false);
        this->ui->sol2->setChecked(false);
        this->ui->sol3->setChecked(false);
        this->ui->sol4->setChecked(false);
        this->ui->sol5->setChecked(false);
        this->ui->sol7->setChecked(false);

        this->onUpdate = false;
        return;
    }

    int subtileIdx = this->levelCelView->getCurrentSubtileIndex();
    quint8 sol = this->sol->getSubtileProperties(subtileIdx);

    this->ui->sol0->setChecked((sol & 1 << 0) != 0);
    this->ui->sol1->setChecked((sol & 1 << 1) != 0);
    this->ui->sol2->setChecked((sol & 1 << 2) != 0);
    this->ui->sol3->setChecked((sol & 1 << 3) != 0);
    this->ui->sol4->setChecked((sol & 1 << 4) != 0);
    this->ui->sol5->setChecked((sol & 1 << 5) != 0);
    this->ui->sol7->setChecked((sol & 1 << 7) != 0);

    this->onUpdate = false;
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
