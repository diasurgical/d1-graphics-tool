#include "palettewidget.h"

#include <algorithm>

#include <QColorDialog>
#include <QDebug>
#include <QMessageBox>
#include <QMimeData>

#include "config/config.h"
#include "mainwindow.h"
#include "ui_palettewidget.h"

enum class COLORFILTER_TYPE {
    NONE,
    USED,
    TILE,
    SUBTILE,
    FRAME,
    TRANSLATED,
};

Q_DECLARE_METATYPE(COLORFILTER_TYPE)

EditColorsCommand::EditColorsCommand(D1Pal *p, quint8 sci, quint8 eci, QColor nc, QColor ec)
    : pal(p)
    , startColorIndex(sci)
    , endColorIndex(eci)
    , newColor(nc)
    , endColor(ec)
{
    // Get the initial color values before doing any modification
    for (int i = startColorIndex; i <= endColorIndex; i++)
        initialColors.append(this->pal->getColor(i));
}

void EditColorsCommand::undo()
{
    if (this->pal.isNull()) {
        this->setObsolete(true);
        return;
    }

    for (int i = startColorIndex; i <= endColorIndex; i++)
        this->pal->setColor(i, this->initialColors.at(i - this->startColorIndex));

    emit this->modified();
}

void EditColorsCommand::redo()
{
    if (this->pal.isNull()) {
        this->setObsolete(true);
        return;
    }

    float step = 1.0f / (endColorIndex - startColorIndex + 1);

    for (int i = startColorIndex; i <= endColorIndex; i++) {
        float factor = (i - startColorIndex) * step;

        QColor color(
            this->newColor.red() * (1 - factor) + this->endColor.red() * factor,
            this->newColor.green() * (1 - factor) + this->endColor.green() * factor,
            this->newColor.blue() * (1 - factor) + this->endColor.blue() * factor,
            255);

        this->pal->setColor(i, color);
    }

    emit this->modified();
}

EditTranslationsCommand::EditTranslationsCommand(D1Trn *t, quint8 sci, quint8 eci, QList<quint8> nt)
    : trn(t)
    , startColorIndex(sci)
    , endColorIndex(eci)
    , newTranslations(nt)
{
    // Get the initial color values before doing any modification
    for (int i = startColorIndex; i <= endColorIndex; i++)
        initialTranslations.append(this->trn->getTranslation(i));
}

void EditTranslationsCommand::undo()
{
    if (this->trn.isNull()) {
        this->setObsolete(true);
        return;
    }

    for (int i = startColorIndex; i <= endColorIndex; i++)
        this->trn->setTranslation(i, this->initialTranslations.at(i - this->startColorIndex));

    emit this->modified();
}

void EditTranslationsCommand::redo()
{
    if (this->trn.isNull()) {
        this->setObsolete(true);
        return;
    }

    for (int i = startColorIndex; i <= endColorIndex; i++)
        this->trn->setTranslation(i, this->newTranslations.at(i - this->startColorIndex));

    emit this->modified();
}

ClearTranslationsCommand::ClearTranslationsCommand(D1Trn *t, quint8 sci, quint8 eci)
    : trn(t)
    , startColorIndex(sci)
    , endColorIndex(eci)
{
    // Get the initial color values before doing any modification
    for (int i = startColorIndex; i <= endColorIndex; i++)
        initialTranslations.append(this->trn->getTranslation(i));
}

void ClearTranslationsCommand::undo()
{
    if (this->trn.isNull()) {
        this->setObsolete(true);
        return;
    }

    for (int i = startColorIndex; i <= endColorIndex; i++)
        this->trn->setTranslation(i, this->initialTranslations.at(i - this->startColorIndex));

    emit this->modified();
}

void ClearTranslationsCommand::redo()
{
    if (this->trn.isNull()) {
        this->setObsolete(true);
        return;
    }

    for (int i = startColorIndex; i <= endColorIndex; i++)
        this->trn->setTranslation(i, i);

    emit this->modified();
}

PaletteScene::PaletteScene(QWidget *v)
    : QGraphicsScene(0, 0, PALETTE_WIDTH, PALETTE_WIDTH)
    , view(v)
{
}

static int getColorIndexFromCoordinates(QPointF coordinates)
{
    // if (position.x() < 0 || position.x() >= PALETTE_WIDTH
    //    || position.y() < 0 || position.y() >= PALETTE_WIDTH)
    //    return -1;

    int index = 0;

    int w = PALETTE_WIDTH / PALETTE_COLORS_PER_LINE;

    int ix = coordinates.x() / w;
    int iy = coordinates.y() / w;

    index = iy * PALETTE_COLORS_PER_LINE + ix;

    return index;
}

void PaletteScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    QPointF pos = event->scenePos();

    qDebug() << "Clicked: " << pos.x() << "," << pos.y();

    // Check if selected color has changed
    int colorIndex = getColorIndexFromCoordinates(pos);

    // if (colorIndex >= 0)
    ((PaletteWidget *)this->view)->startColorSelection(colorIndex);
}

void PaletteScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }

    QPointF pos = event->scenePos();

    // Check if selected color has changed
    int colorIndex = getColorIndexFromCoordinates(pos);

    // if (colorIndex >= 0)
    ((PaletteWidget *)this->view)->changeColorSelection(colorIndex);
}

void PaletteScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    ((PaletteWidget *)this->view)->finishColorSelection();
}

void PaletteScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    this->dragMoveEvent(event);
}

void PaletteScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    bool isTrn = ((PaletteWidget *)this->view)->isTrnWidget();
    const char *ext = isTrn ? ".trn" : ".pal";
    for (const QUrl &url : event->mimeData()->urls()) {
        if (url.toLocalFile().toLower().endsWith(ext)) {
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
}

void PaletteScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    event->acceptProposedAction();

    QStringList filePaths;
    bool isTrn = ((PaletteWidget *)this->view)->isTrnWidget();
    const char *ext = isTrn ? ".trn" : ".pal";
    for (const QUrl &url : event->mimeData()->urls()) {
        if (url.toLocalFile().toLower().endsWith(ext)) {
            filePaths.append(url.toLocalFile());
        }
    }
    // try to insert pal/trn files
    ((MainWindow *)this->view->window())->openPalFiles(filePaths, (PaletteWidget *)this->view);
}

void PaletteScene::contextMenuEvent(QContextMenuEvent *event)
{
    ((PaletteWidget *)this->view)->ShowContextMenu(event->globalPos());
}

QPushButton *PaletteWidget::addButton(QStyle::StandardPixmap type, QString tooltip, void (PaletteWidget::*callback)(void))
{
    QPushButton *button = new QPushButton(this->style()->standardIcon(type), "", nullptr);
    constexpr int iconSize = 16;
    button->setToolTip(tooltip);
    button->setIconSize(QSize(iconSize, iconSize));
    button->setMinimumSize(iconSize, iconSize);
    button->setMaximumSize(iconSize, iconSize);
    ((QBoxLayout *)ui->groupHeader->layout())->addWidget(button, Qt::AlignLeft);

    QObject::connect(button, &QPushButton::clicked, this, callback);
    return button;
}

PaletteWidget::PaletteWidget(std::shared_ptr<UndoStack> us, QString title)
    : QWidget(nullptr)
    , undoStack(std::move(us))
    , ui(new Ui::PaletteWidget())
    , scene(new PaletteScene(this))
{
    // Load selection border color from JSON config file
    this->reloadConfig();

    ui->setupUi(this);
    ui->graphicsView->setScene(this->scene);
    ui->groupLabel->setText(title);

    this->addButton(QStyle::SP_FileDialogNewFolder, "New", &PaletteWidget::on_newPushButtonClicked); // use SP_FileIcon ?
    this->addButton(QStyle::SP_DialogOpenButton, "Open", &PaletteWidget::on_openPushButtonClicked);
    this->addButton(QStyle::SP_DialogSaveButton, "Save", &PaletteWidget::on_savePushButtonClicked);
    this->addButton(QStyle::SP_DialogSaveButton, "Save As", &PaletteWidget::on_saveAsPushButtonClicked);
    this->addButton(QStyle::SP_DialogCloseButton, "Close", &PaletteWidget::on_closePushButtonClicked); // use SP_DialogDiscardButton ?

    // When there is a modification to the PAL or TRNs then UI must be refreshed
    QObject::connect(this, &PaletteWidget::modified, this, &PaletteWidget::refresh);

    // setup context menu
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowContextMenu(const QPoint &)));
}

PaletteWidget::~PaletteWidget()
{
    delete ui;
    delete scene;

    for (auto &pair : m_palettes_map) {
        delete pair.second.second;
    }
}

void PaletteWidget::setPal(const QString &path)
{
    m_pal = m_palettes_map[path].second;

    emit this->modified();
}

void PaletteWidget::setTrn(const QString &path)
{
    this->m_trn = dynamic_cast<D1Trn *>(m_palettes_map[path].second);

    emit this->modified();
}

bool PaletteWidget::isTrnWidget()
{
    return (m_paletteType == PaletteType::Translation || m_paletteType == PaletteType::UniqTranslation);
}

void PaletteWidget::initialize(D1Pal *p, CelView *c, D1PalHits *ph, PaletteType palType)
{
    this->m_paletteType = palType;
    this->m_pal = p;
    this->m_trn = nullptr;
    this->celView = c;
    this->levelCelView = nullptr;
    this->palHits = ph;

    this->initializeUi();
}

void PaletteWidget::initialize(D1Pal *p, LevelCelView *lc, D1PalHits *ph, PaletteType palType)
{
    this->m_paletteType = palType;
    this->m_pal = p;
    this->m_trn = nullptr;
    this->celView = nullptr;
    this->levelCelView = lc;
    this->palHits = ph;

    this->initializeUi();
}

void PaletteWidget::initialize(D1Pal *p, D1Trn *t, CelView *c, D1PalHits *ph, PaletteType palType)
{
    this->m_paletteType = palType;
    this->m_pal = p;
    this->m_trn = t;
    this->celView = c;
    this->levelCelView = nullptr;
    this->palHits = ph;

    this->initializeUi();
}

void PaletteWidget::initialize(D1Pal *p, D1Trn *t, LevelCelView *lc, D1PalHits *ph, PaletteType palType)
{
    this->m_paletteType = palType;
    this->m_pal = p;
    this->m_trn = t;
    this->celView = nullptr;
    this->levelCelView = lc;
    this->palHits = ph;

    this->initializeUi();
}

void PaletteWidget::initializeUi()
{
    bool trnMode = this->m_paletteType == PaletteType::Translation || this->m_paletteType == PaletteType::UniqTranslation;

    this->ui->monsterTrnPushButton->setVisible(trnMode);
    this->ui->translationClearPushButton->setVisible(trnMode);
    this->ui->translationPickPushButton->setVisible(trnMode);
    this->ui->colorLineEdit->setReadOnly(trnMode);
    this->ui->colorPickPushButton->setVisible(!trnMode);
    this->ui->colorClearPushButton->setVisible(!trnMode);
    this->ui->translationIndexLineEdit->setVisible(trnMode);
    this->ui->translationLabel->setVisible(trnMode);

    this->initializePathComboBox();
    this->initializeDisplayComboBox();

    this->refreshColorLineEdit();
    this->refreshIndexLineEdit();
    if (trnMode)
        this->refreshTranslationIndexLineEdit();

    this->displayColors();
    this->displaySelection();
}

void PaletteWidget::initializePathComboBox()
{
    if (m_paletteType == PaletteType::Palette) {
        this->m_palettes_map[D1Pal::DEFAULT_PATH].first = D1Pal::DEFAULT_NAME;
        this->m_palettes_map[D1Pal::DEFAULT_PATH].second = m_pal;
    } else {
        this->m_palettes_map[D1Trn::IDENTITY_PATH].first = D1Trn::IDENTITY_NAME;
        this->m_palettes_map[D1Trn::IDENTITY_PATH].second = m_trn;
    }

    this->refreshPathComboBox();
}

void PaletteWidget::initializeDisplayComboBox()
{
    ui->displayComboBox->addItem("Show all colors", QVariant((int)COLORFILTER_TYPE::NONE));

    if (m_paletteType == PaletteType::Palette) {
        ui->displayComboBox->addItem("Show all frames hits", QVariant((int)COLORFILTER_TYPE::USED));
        if (this->levelCelView != nullptr) {
            ui->displayComboBox->addItem("Show current tile hits", QVariant((int)COLORFILTER_TYPE::TILE));
            ui->displayComboBox->addItem("Show current sub-tile hits", QVariant((int)COLORFILTER_TYPE::SUBTILE));
        }
        ui->displayComboBox->addItem("Show current frame hits", QVariant((int)COLORFILTER_TYPE::FRAME));
    } else {
        ui->displayComboBox->addItem("Show translated colors", QVariant((int)COLORFILTER_TYPE::TRANSLATED));
    }
}

void PaletteWidget::reloadConfig()
{
    this->paletteDefaultColor = QColor(
        Config::value("PaletteDefaultColor").toString());

    this->selectionBorderColor = QColor(
        Config::value("PaletteSelectionBorderColor").toString());
}

void PaletteWidget::selectColor(quint8 index)
{
    this->selectedFirstColorIndex = index;
    this->selectedLastColorIndex = index;

    this->temporarilyDisplayingAllColors = false;

    this->refresh();
}

void PaletteWidget::checkTranslationsSelection(QList<quint8> indexes)
{
    if (!this->pickingTranslationColor)
        return;

    quint8 selectionLength = this->selectedLastColorIndex - this->selectedFirstColorIndex + 1;
    if (selectionLength != indexes.length()) {
        QMessageBox::warning(this, "Warning", "Source and target selection length do not match.");
        return;
    }

    // Build color editing command and connect it to the current palette widget
    // to update the PAL/TRN and CEL views when undo/redo is performed
    std::unique_ptr<EditTranslationsCommand> command = std::make_unique<EditTranslationsCommand>(
        this->m_trn, this->selectedFirstColorIndex, this->selectedLastColorIndex, indexes);
    QObject::connect(command.get(), &EditTranslationsCommand::modified, this, &PaletteWidget::modify);

    this->undoStack->push(std::move(command));

    this->pickingTranslationColor = false;
    this->clearInfo();

    emit this->clearRootInformation();
    emit this->clearRootBorder();
}

void PaletteWidget::addPath(const QString &path, const QString &name, D1Pal *pal)
{
    this->m_palettes_map[path] = std::make_pair(name, pal);
}

void PaletteWidget::removePath(const QString &path)
{
    if (this->m_palettes_map.contains(path))
        this->m_palettes_map.erase(path);
}

void PaletteWidget::selectPath(const QString &path)
{
    this->ui->pathComboBox->setCurrentIndex(this->ui->pathComboBox->findData(path));
    this->ui->pathComboBox->setToolTip(path);

    emit this->pathSelected(path);
    emit this->modified();
}

QString PaletteWidget::getWidgetsDefaultPath() const
{
    if (m_paletteType == PaletteType::Palette) {
        return D1Pal::DEFAULT_PATH;
    }

    return D1Trn::IDENTITY_PATH;
}

QString PaletteWidget::getSelectedPath() const
{
    QString path = this->ui->pathComboBox->currentText();
    for (const auto &[key, valuePair] : m_palettes_map) {
        if (valuePair.first == this->ui->pathComboBox->currentText())
            return key;
    }
}

static QRectF getColorCoordinates(quint8 index)
{
    int ix = index % PALETTE_COLORS_PER_LINE;
    int iy = index / PALETTE_COLORS_PER_LINE;

    int w = PALETTE_WIDTH / PALETTE_COLORS_PER_LINE;

    QRectF coordinates(ix * w, iy * w, w, w);

    return coordinates;
}

void PaletteWidget::ShowContextMenu(const QPoint &pos)
{
    QMenu contextMenu(tr("Context menu"), this);
    contextMenu.setToolTipsVisible(true);

    QAction action0("Undo", this);
    QObject::connect(&action0, SIGNAL(triggered()), this, SLOT(actionUndo_triggered()));
    action0.setEnabled(this->undoStack->canUndo());
    contextMenu.addAction(&action0);

    QAction action1("Redo", this);
    QObject::connect(&action1, SIGNAL(triggered()), this, SLOT(actionRedo_triggered()));
    action1.setEnabled(this->undoStack->canRedo());
    contextMenu.addAction(&action1);

    contextMenu.exec(mapToGlobal(pos));
}

void PaletteWidget::startColorSelection(int colorIndex)
{
    this->selectedFirstColorIndex = colorIndex;
    this->selectedLastColorIndex = colorIndex;
}

void PaletteWidget::changeColorSelection(int colorIndex)
{
    this->selectedLastColorIndex = colorIndex;

    this->refreshIndexLineEdit();

    this->displayColors();
    this->displaySelection();
}

void PaletteWidget::finishColorSelection()
{
    // If second selected color has an index less than the first one swap them
    if (this->selectedFirstColorIndex > this->selectedLastColorIndex) {
        std::swap(this->selectedFirstColorIndex, this->selectedLastColorIndex);
    }

    if (m_paletteType != PaletteType::Palette && this->pickingTranslationColor) {
        this->clearInfo();
        emit this->clearRootInformation();
        emit this->clearRootBorder();
        this->pickingTranslationColor = false;
    }

    this->temporarilyDisplayingAllColors = false;

    // emit selected colors
    // if ((!this->isTrn && !this->pal.isNull()) || (this->isTrn && !this->m_trn.isNull())) {
    QList<quint8> indexes;
    for (int i = this->selectedFirstColorIndex; i <= this->selectedLastColorIndex; i++)
        indexes.append(i);
    emit this->colorsSelected(indexes);
    // }

    this->refresh();
}

void PaletteWidget::setTrnPalette(D1Pal *pal)
{
    this->m_trn->setPalette(pal);
    refresh();
}

void PaletteWidget::save()
{
    PaletteFileInfo fileInfo = paletteFileInfo();

    QString selectedPath = getSelectedPath();
    if (selectedPath == getWidgetsDefaultPath()) {
        newOrSaveAsFile(PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_SAVEAS);
    } else {
        if (!(m_paletteType != PaletteType::Palette ? m_trn->save(selectedPath) : m_pal->save(selectedPath))) {
            QMessageBox::critical(this, "Error", QString("Could not save %1 file.").arg(fileInfo.suffix.toUpper()));
        }
    }
}

PaletteFileInfo PaletteWidget::paletteFileInfo() const
{
    if (m_paletteType != PaletteType::Palette) {
        return { "translation", "trn" };
    }

    return { "palette", "pal" };
}

void PaletteWidget::performSave(const QString &palFilePath, const PaletteFileInfo &fileInfo)
{
    bool opResult = false;
    if (m_paletteType == PaletteType::Palette) {
        opResult = this->m_pal->save(palFilePath);
    } else {
        opResult = this->m_trn->save(palFilePath);
    }

    if (!opResult) {
        QMessageBox::critical(this, "Error", QString("Could not save %1 file.").arg(fileInfo.suffix.toUpper()));
    }
}

void PaletteWidget::newOrSaveAsFile(const PWIDGET_CALLBACK_TYPE action)
{
    PaletteFileInfo fileInfo = paletteFileInfo();

    QString actionStr;
    if (action == PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_SAVEAS) {
        actionStr = QString("Save %1 File as...").arg(fileInfo.name);
    } else {
        actionStr = QString("New %1 File").arg(fileInfo.name);
    }

    auto *mw = dynamic_cast<MainWindow *>(this->window());
    QString palFilePath = mw->fileDialog(FILE_DIALOG_MODE::SAVE_CONF,
        actionStr.toStdString().c_str(),
        (QString("%2 Files (*.%1 *.%2)").arg(fileInfo.suffix, fileInfo.suffix.toUpper())).toStdString().c_str());

    if (palFilePath.isEmpty()) {
        return;
    }

    QFileInfo palFileInfo(palFilePath);
    QString path = palFileInfo.absoluteFilePath();
    QString name = palFileInfo.fileName();

    if (palFileInfo.suffix().isEmpty()) {
        path += "." + fileInfo.suffix;
        name += "." + fileInfo.suffix;
    } else if (palFileInfo.suffix() != fileInfo.suffix) {
        QMessageBox::critical(nullptr, "Error", QString("Only %1 is supported for %2").arg(fileInfo.suffix, fileInfo.name));
        return;
    }

    // For save operation we need to save first and then load, but for new file op
    // we need to propagate `newPal` with default information first by loading and
    // then save
    if (action == PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_SAVEAS) {
        performSave(palFilePath, fileInfo);
    }

    D1Pal *newPal;
    switch (m_paletteType) {
    case PaletteType::Palette:
        newPal = new D1Pal();
        break;
    case PaletteType::Translation:
        newPal = new D1Trn(mw->uniqTrnWidget()->trn()->getResultingPalette());
        break;
    case PaletteType::UniqTranslation:
        newPal = new D1Trn(mw->paletteWidget()->pal());
        break;
    }

    QString loadedFilePath;
    if (action == PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_NEW) {
        loadedFilePath = newPal->getDefaultPath();
    } else {
        loadedFilePath = path;
    }

    if (!newPal->load(loadedFilePath)) {
        delete newPal;
        QMessageBox::critical(this, "Error", QString("Could not load %1 file.").arg(fileInfo.suffix.toUpper()));
        return;
    }

    // For new operation we need to load first and then save, as the file has to be
    // propagated with default information first
    if (action == PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_NEW) {
        performSave(path, fileInfo);
    }

    if (this->m_palettes_map.contains(name))
        delete this->m_palettes_map[name].second;
    addPath(path, name, newPal);
    selectPath(path);
}

bool PaletteWidget::loadPalette(const QString &filepath)
{
    PaletteFileInfo fileInfo = paletteFileInfo();

    auto *mw = dynamic_cast<MainWindow *>(this->window());
    QFileInfo palFileInfo(filepath);
    // QString path = trnFileInfo.absoluteFilePath();
    const QString &path = filepath;
    QString name = palFileInfo.fileName();

    D1Pal *newPal;
    switch (m_paletteType) {
    case PaletteType::Palette:
        newPal = new D1Pal();
        break;
    case PaletteType::Translation:
        newPal = new D1Trn(mw->uniqTrnWidget()->trn()->getResultingPalette());
        break;
    case PaletteType::UniqTranslation:
        newPal = new D1Trn(mw->paletteWidget()->pal());
        break;
    }

    if (!newPal->load(path)) {
        delete newPal;
        QMessageBox::critical(this, "Error", QString("Could not load %1 file.").arg(fileInfo.suffix.toUpper()));
        return false;
    }

    if (this->m_palettes_map.contains(name))
        delete this->m_palettes_map[name].second;
    addPath(path, name, newPal);
    return true;
}

void PaletteWidget::openPalette()
{
    PaletteFileInfo fileInfo = paletteFileInfo();

    auto *mw = dynamic_cast<MainWindow *>(this->window());
    QString paletteFilePath = mw->fileDialog(FILE_DIALOG_MODE::OPEN,
        QString("Load %1 File").arg(fileInfo.name).toStdString().c_str(),
        QString("%2 Files (*.%1 *.%2)").arg(fileInfo.suffix, fileInfo.suffix.toUpper()).toStdString().c_str());

    if (!paletteFilePath.isEmpty() && loadPalette(paletteFilePath)) {
        selectPath(paletteFilePath);
    }
}

bool PaletteWidget::isOkToQuit()
{
    for (const auto &pair : m_palettes_map) {
        D1Pal *pal = pair.second.second;
        if (!mw::QuestionDiscardChanges(pal->isModified(), pal->getFilePath())) {
            return false;
        }
    }

    return true;
}

void PaletteWidget::closePalette()
{
    QString selectedPath = getSelectedPath();
    if (selectedPath == getWidgetsDefaultPath())
        return;

    if (this->m_palettes_map.contains(selectedPath)) {
        delete this->m_palettes_map[selectedPath].second;
        this->m_palettes_map.erase(selectedPath);
    }

    removePath(selectedPath);
    selectPath(getWidgetsDefaultPath());
}

bool PaletteWidget::displayColor(int colorIndex)
{
    quint32 itemIndex;

    switch (this->palHits->getMode()) {
    case D1PALHITS_MODE::ALL_COLORS:
    case D1PALHITS_MODE::ALL_FRAMES:
        return this->palHits->getIndexHits(colorIndex, 0) != 0;
    case D1PALHITS_MODE::CURRENT_TILE:
        itemIndex = this->levelCelView->getCurrentTileIndex();
        break;
    case D1PALHITS_MODE::CURRENT_SUBTILE:
        itemIndex = this->levelCelView->getCurrentSubtileIndex();
        break;
    case D1PALHITS_MODE::CURRENT_FRAME:
        itemIndex = this->levelCelView != nullptr ? this->levelCelView->getCurrentFrameIndex() : this->celView->getCurrentFrameIndex();
        break;
    }

    return this->palHits->getIndexHits(colorIndex, itemIndex) != 0;
}

void PaletteWidget::displayColors()
{
    // delta
    const int dx = PALETTE_WIDTH / PALETTE_COLORS_PER_LINE;
    const int dy = PALETTE_WIDTH / PALETTE_COLORS_PER_LINE;

    // Color width
    const int w = PALETTE_WIDTH / PALETTE_COLORS_PER_LINE - 2 * PALETTE_COLOR_SPACING;
    const int bsw = PALETTE_COLOR_SPACING;

    // Removing existing items
    this->scene->clear();

    // Setting background color
    this->scene->setBackgroundBrush(Qt::white);

    QPen pen(Qt::NoPen);

    for (int i = 0; i < D1PAL_COLORS; i++) {
        if (!this->temporarilyDisplayingAllColors && !this->displayColor(i))
            continue;

        // Check translation display filter
        if (m_paletteType != PaletteType::Palette && ui->displayComboBox->currentData().value<COLORFILTER_TYPE>() == COLORFILTER_TYPE::TRANSLATED // "Show translated colors"
            && this->m_trn->getTranslation(i) == i)
            continue;

        int x = i % PALETTE_COLORS_PER_LINE;
        int y = i / PALETTE_COLORS_PER_LINE;

        QBrush brush = QBrush(m_paletteType != PaletteType::Palette ? this->m_trn->getResultingPalette()->getColor(i) : this->m_pal->getColor(i));
        this->scene->addRect(x * dx + bsw, y * dy + bsw, w, w, pen, brush);
    }
}

void PaletteWidget::displaySelection()
{
    QPen pen(this->selectionBorderColor);
    pen.setStyle(Qt::SolidLine);
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setWidth(PALETTE_SELECTION_WIDTH);

    int first = 0;
    int last = 0;
    // int length = 0;
    if (this->selectedFirstColorIndex <= this->selectedLastColorIndex) {
        first = this->selectedFirstColorIndex;
        last = this->selectedLastColorIndex;
    } else {
        // Swap first and last color if this is a backwards selection
        first = this->selectedLastColorIndex;
        last = this->selectedFirstColorIndex;
    }
    // length = last - first + 1;

    for (int i = first; i <= last; i++) {
        QRectF coordinates = getColorCoordinates(i);
        int a = PALETTE_SELECTION_WIDTH / 2;
        coordinates.adjust(a, a, -a, -a);

        // left line
        if (i == first && i + PALETTE_COLORS_PER_LINE <= last)
            this->scene->addLine(coordinates.bottomLeft().x(), coordinates.bottomLeft().y() + PALETTE_SELECTION_WIDTH,
                coordinates.topLeft().x(), coordinates.topLeft().y(), pen);
        else if (i == first || i % PALETTE_COLORS_PER_LINE == 0)
            this->scene->addLine(coordinates.bottomLeft().x(), coordinates.bottomLeft().y(),
                coordinates.topLeft().x(), coordinates.topLeft().y(), pen);

        // right line
        if (i == last && i - PALETTE_COLORS_PER_LINE >= first)
            this->scene->addLine(coordinates.topRight().x(), coordinates.topRight().y() - PALETTE_SELECTION_WIDTH,
                coordinates.bottomRight().x(), coordinates.bottomRight().y(), pen);
        else if (i == last || i % PALETTE_COLORS_PER_LINE == PALETTE_COLORS_PER_LINE - 1)
            this->scene->addLine(coordinates.topRight().x(), coordinates.topRight().y(),
                coordinates.bottomRight().x(), coordinates.bottomRight().y(), pen);

        // top line
        if (i - PALETTE_COLORS_PER_LINE < first)
            this->scene->addLine(coordinates.topLeft().x(), coordinates.topLeft().y(),
                coordinates.topRight().x(), coordinates.topRight().y(), pen);

        // bottom line
        if (i + PALETTE_COLORS_PER_LINE > last)
            this->scene->addLine(coordinates.bottomLeft().x(), coordinates.bottomLeft().y(),
                coordinates.bottomRight().x(), coordinates.bottomRight().y(), pen);
    }
}

void PaletteWidget::temporarilyDisplayAllColors()
{
    this->temporarilyDisplayingAllColors = true;
    this->displayColors();
}

void PaletteWidget::displayInfo(QString info)
{
    this->ui->informationLabel->setText(info);
}

void PaletteWidget::clearInfo()
{
    this->ui->informationLabel->clear();
}

void PaletteWidget::displayBorder()
{
    this->ui->graphicsView->setStyleSheet("color: rgb(255, 0, 0);");
}

void PaletteWidget::clearBorder()
{
    this->ui->graphicsView->setStyleSheet("color: rgb(255, 255, 255);");
}

void PaletteWidget::refreshPathComboBox()
{
    this->ui->pathComboBox->clear();

    // Go through the hits of the CEL frame and add them to the subtile hits
    for (const auto &[key, pair] : m_palettes_map) {
        this->ui->pathComboBox->addItem(pair.first, key);
    }

    QString selectedPath;
    if (m_paletteType == PaletteType::Palette) {
        selectedPath = this->m_pal->getFilePath();
    } else {
        selectedPath = this->m_trn->getFilePath();
    }
    this->ui->pathComboBox->setCurrentIndex(this->ui->pathComboBox->findData(selectedPath));
    this->ui->pathComboBox->setToolTip(selectedPath);
}

void PaletteWidget::refreshColorLineEdit()
{
    if (this->selectedFirstColorIndex == this->selectedLastColorIndex) {
        QColor selectedColor = this->m_pal->getColor(this->selectedFirstColorIndex);
        this->ui->colorLineEdit->setText(selectedColor.name());
    } else {
        this->ui->colorLineEdit->setText("*");
    }
}

void PaletteWidget::refreshIndexLineEdit()
{
    int firstColorIndex = this->selectedFirstColorIndex;
    int lastColorIndex = this->selectedLastColorIndex;
    if (firstColorIndex == lastColorIndex) {
        this->ui->indexLineEdit->setText(QString::number(firstColorIndex));
    } else {
        // If second selected color has an index less than the first one swap them
        if (firstColorIndex < lastColorIndex) {
            std::swap(firstColorIndex, lastColorIndex);
        }
        this->ui->indexLineEdit->setText(QString::number(firstColorIndex) + "-" + QString::number(lastColorIndex));
    }
}

void PaletteWidget::refreshTranslationIndexLineEdit()
{
    if (this->m_trn == nullptr)
        return;

    if (this->selectedFirstColorIndex == this->selectedLastColorIndex) {
        this->ui->translationIndexLineEdit->setText(
            QString::number(this->m_trn->getTranslation(this->selectedFirstColorIndex)));
    } else {
        this->ui->translationIndexLineEdit->setText("*");
    }
}

void PaletteWidget::modify()
{
    emit this->modified();
}

void PaletteWidget::refresh()
{
    if (m_paletteType != PaletteType::Palette)
        this->m_trn->refreshResultingPalette();

    this->displayColors();
    this->displaySelection();
    this->refreshPathComboBox();
    this->refreshColorLineEdit();
    this->refreshIndexLineEdit();
    if (m_paletteType != PaletteType::Palette)
        this->refreshTranslationIndexLineEdit();

    emit refreshed();
}

void PaletteWidget::on_newPushButtonClicked()
{
    ((MainWindow *)this->window())->paletteWidget_callback(this, PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_NEW);
}

void PaletteWidget::on_openPushButtonClicked()
{
    ((MainWindow *)this->window())->paletteWidget_callback(this, PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_OPEN);
}

void PaletteWidget::on_savePushButtonClicked()
{
    ((MainWindow *)this->window())->paletteWidget_callback(this, PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_SAVE);
}

void PaletteWidget::on_saveAsPushButtonClicked()
{
    ((MainWindow *)this->window())->paletteWidget_callback(this, PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_SAVEAS);
}

void PaletteWidget::on_closePushButtonClicked()
{
    ((MainWindow *)this->window())->paletteWidget_callback(this, PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_CLOSE);
}

void PaletteWidget::actionUndo_triggered()
{
    this->undoStack->undo();
}

void PaletteWidget::actionRedo_triggered()
{
    this->undoStack->redo();
}

void PaletteWidget::on_pathComboBox_activated(int index)
{
    QString filePath = this->ui->pathComboBox->currentData().value<QString>();

    emit this->pathSelected(filePath);
    emit this->modified();
}

void PaletteWidget::on_displayComboBox_activated(int index)
{
    if (m_paletteType == PaletteType::Palette) {
        D1PALHITS_MODE mode = D1PALHITS_MODE::ALL_COLORS;
        switch (this->ui->displayComboBox->currentData().value<COLORFILTER_TYPE>()) {
        case COLORFILTER_TYPE::NONE:
            mode = D1PALHITS_MODE::ALL_COLORS;
            break;
        case COLORFILTER_TYPE::USED:
            mode = D1PALHITS_MODE::ALL_FRAMES;
            break;
        case COLORFILTER_TYPE::TILE:
            mode = D1PALHITS_MODE::CURRENT_TILE;
            break;
        case COLORFILTER_TYPE::SUBTILE:
            mode = D1PALHITS_MODE::CURRENT_SUBTILE;
            break;
        case COLORFILTER_TYPE::FRAME:
            mode = D1PALHITS_MODE::CURRENT_FRAME;
            break;
        }
        this->palHits->setMode(mode);
    }

    this->refresh();
}

void PaletteWidget::on_colorLineEdit_returnPressed()
{
    QColor color = QColor(ui->colorLineEdit->text());

    // Build color editing command and connect it to the current palette widget
    // to update the PAL/TRN and CEL views when undo/redo is performed
    std::unique_ptr<EditColorsCommand> command = std::make_unique<EditColorsCommand>(
        this->m_pal, this->selectedFirstColorIndex, this->selectedLastColorIndex, color, color);
    QObject::connect(command.get(), &EditColorsCommand::modified, this, &PaletteWidget::modify);

    this->undoStack->push(std::move(command));

    // Release focus to allow keyboard shortcuts to work as expected
    this->ui->colorLineEdit->clearFocus();
}

void PaletteWidget::on_colorPickPushButton_clicked()
{
    QColor color = this->m_pal->getColor(this->selectedFirstColorIndex);
    color = QColorDialog::getColor(color);
    if (!color.isValid())
        return;

    QColor colorEnd;
    if (this->selectedFirstColorIndex == this->selectedLastColorIndex) {
        colorEnd = color;
    } else {
        colorEnd = this->m_pal->getColor(this->selectedLastColorIndex);
        colorEnd = QColorDialog::getColor(colorEnd);
        if (!colorEnd.isValid())
            return;
    }

    // Build color editing command and connect it to the current palette widget
    // to update the PAL/TRN and CEL views when undo/redo is performed
    std::unique_ptr<EditColorsCommand> command = std::make_unique<EditColorsCommand>(
        this->m_pal, this->selectedFirstColorIndex, this->selectedLastColorIndex, color, colorEnd);
    QObject::connect(command.get(), &EditColorsCommand::modified, this, &PaletteWidget::modify);

    this->undoStack->push(std::move(command));
}

void PaletteWidget::on_colorClearPushButton_clicked()
{
    // Build color editing command and connect it to the current palette widget
    // to update the PAL/TRN and CEL views when undo/redo is performed
    std::unique_ptr<EditColorsCommand> command = std::make_unique<EditColorsCommand>(
        this->m_pal, this->selectedFirstColorIndex, this->selectedLastColorIndex, this->paletteDefaultColor, this->paletteDefaultColor);
    QObject::connect(command.get(), &EditColorsCommand::modified, this, &PaletteWidget::modify);

    this->undoStack->push(std::move(command));
}

void PaletteWidget::on_translationIndexLineEdit_returnPressed()
{
    quint8 index = ui->translationIndexLineEdit->text().toUInt();

    // New translations
    QList<quint8> newTranslations;
    for (int i = this->selectedFirstColorIndex; i <= this->selectedLastColorIndex; i++)
        newTranslations.append(index);

    // Build translation editing command and connect it to the current palette widget
    // to update the PAL/TRN and CEL views when undo/redo is performed
    std::unique_ptr<EditTranslationsCommand> command = std::make_unique<EditTranslationsCommand>(
        this->m_trn, this->selectedFirstColorIndex, this->selectedLastColorIndex, newTranslations);
    QObject::connect(command.get(), &EditTranslationsCommand::modified, this, &PaletteWidget::modify);

    this->undoStack->push(std::move(command));

    // Release focus to allow keyboard shortcuts to work as expected
    this->ui->translationIndexLineEdit->clearFocus();
}

void PaletteWidget::on_translationPickPushButton_clicked()
{
    this->pickingTranslationColor = true;

    emit this->displayAllRootColors();
    emit this->displayRootInformation("<- Select translation");
    emit this->displayRootBorder();
}

void PaletteWidget::on_translationClearPushButton_clicked()
{
    // Build translation clearing command and connect it to the current palette widget
    // to update the PAL/TRN and CEL views when undo/redo is performed
    std::unique_ptr<ClearTranslationsCommand> command = std::make_unique<ClearTranslationsCommand>(
        this->m_trn, this->selectedFirstColorIndex, this->selectedLastColorIndex);
    QObject::connect(command.get(), &ClearTranslationsCommand::modified, this, &PaletteWidget::modify);

    this->undoStack->push(std::move(command));
}

void PaletteWidget::on_monsterTrnPushButton_clicked()
{
    bool trnModified = false;

    for (int i = 0; i < D1PAL_COLORS; i++) {
        if (this->m_trn->getTranslation(i) == 0xFF) {
            this->m_trn->setTranslation(i, 0);
            trnModified = true;
        }
    }

    if (trnModified) {
        emit this->modified();
    }
}
