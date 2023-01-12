#include "mainwindow.h"

#include <QDragEnterEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QImageReader>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QMimeData>
#include <QMimeDatabase>
#include <QMimeType>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStringList>
#include <QTextStream>
#include <QTime>
#include <QUndoCommand>
#include <QUndoStack>

#include "d1cel.h"
#include "d1celtileset.h"
#include "d1cl2.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // QCoreApplication::setAttribute( Qt::AA_EnableHighDpiScaling, true );

    this->loadConfiguration();
    this->lastFilePath = this->configuration->value("LastFilePath").toString();

    ui->setupUi(this);

    // initialize 'new' submenu of 'File'
    this->newMenu = new QMenu("New");
    this->newMenu->addAction("CEL gfx", this, SLOT(on_actionNew_CEL_triggered()));
    this->newMenu->addAction("CL2 gfx", this, SLOT(on_actionNew_CL2_triggered()));
    this->newMenu->addAction("Tileset", this, SLOT(on_actionNew_Tileset_triggered()));
    QAction *firstFileAction = (QAction *)this->ui->menuFile->actions()[0];
    this->ui->menuFile->insertMenu(firstFileAction, this->newMenu);

    // Initialize 'Undo/Redo' of 'Edit
    this->undoStack = new QUndoStack(this);
    this->undoAction = undoStack->createUndoAction(this, "Undo");
    this->undoAction->setShortcuts(QKeySequence::Undo);
    this->redoAction = undoStack->createRedoAction(this, "Redo");
    this->redoAction->setShortcuts(QKeySequence::Redo);
    this->ui->menuEdit->addAction(this->undoAction);
    this->ui->menuEdit->addAction(this->redoAction);
    this->ui->menuEdit->addSeparator();

    // Initialize 'Frame' submenu of 'Edit'
    this->frameMenu = new QMenu("Frame");
    this->frameMenu->setToolTipsVisible(true);
    this->frameMenu->addAction("Insert", this, SLOT(on_actionInsert_Frame_triggered()))->setToolTip("Add new frames before the current one");
    this->frameMenu->addAction("Add", this, SLOT(on_actionAdd_Frame_triggered()))->setToolTip("Add new frames at the end");
    this->frameMenu->addAction("Replace", this, SLOT(on_actionReplace_Frame_triggered()))->setToolTip("Replace the current frame");
    this->frameMenu->addAction("Delete", this, SLOT(on_actionDel_Frame_triggered()))->setToolTip("Delete the current frame");
    this->ui->menuEdit->addMenu(this->frameMenu);

    // Initialize 'Subtile' submenu of 'Edit'
    this->subtileMenu = new QMenu("Subtile");
    this->subtileMenu->setToolTipsVisible(true);
    this->subtileMenu->addAction("Create", this, SLOT(on_actionCreate_Subtile_triggered()))->setToolTip("Create a new subtile");
    this->subtileMenu->addAction("Insert", this, SLOT(on_actionInsert_Subtile_triggered()))->setToolTip("Add new subtiles before the current one");
    this->subtileMenu->addAction("Add", this, SLOT(on_actionAdd_Subtile_triggered()))->setToolTip("Add new subtiles at the end");
    this->subtileMenu->addAction("Clone", this, SLOT(on_actionClone_Subtile_triggered()))->setToolTip("Add new subtiles at the end based on the current one");
    this->subtileMenu->addAction("Replace", this, SLOT(on_actionReplace_Subtile_triggered()))->setToolTip("Replace the current subtile");
    this->subtileMenu->addAction("Delete", this, SLOT(on_actionDel_Subtile_triggered()))->setToolTip("Delete the current subtile");
    this->ui->menuEdit->addMenu(this->subtileMenu);

    // Initialize 'Tile' submenu of 'Edit'
    this->tileMenu = new QMenu("Tile");
    this->tileMenu->setToolTipsVisible(true);
    this->tileMenu->addAction("Create", this, SLOT(on_actionCreate_Tile_triggered()))->setToolTip("Create a new tile");
    this->tileMenu->addAction("Insert", this, SLOT(on_actionInsert_Tile_triggered()))->setToolTip("Add new tiles before the current one");
    this->tileMenu->addAction("Add", this, SLOT(on_actionAdd_Tile_triggered()))->setToolTip("Add new tiles at the end");
    this->tileMenu->addAction("Clone", this, SLOT(on_actionClone_Tile_triggered()))->setToolTip("Add new tile at the end based on the current one");
    this->tileMenu->addAction("Replace", this, SLOT(on_actionReplace_Tile_triggered()))->setToolTip("Replace the current tile");
    this->tileMenu->addAction("Delete", this, SLOT(on_actionDel_Tile_triggered()))->setToolTip("Delete the current tile");
    this->ui->menuEdit->addMenu(this->tileMenu);

    this->on_actionClose_triggered();
    setAcceptDrops(true);
}

MainWindow::~MainWindow()
{
    // close modal windows
    this->on_actionClose_triggered();
    // store last path
    this->configuration->insert("LastFilePath", this->lastFilePath);
    this->settingsDialog->storeConfiguration(configuration);
    // cleanup memory
    delete ui;
    delete configuration;
    delete openAsDialog;
    delete saveAsDialog;
    delete settingsDialog;
    delete exportDialog;
    delete this->newMenu;
    delete this->undoStack;
    delete this->undoAction;
    delete this->redoAction;
    delete this->frameMenu;
    delete this->subtileMenu;
    delete this->tileMenu;
    delete this->palHits;
}

void MainWindow::setPal(QString path)
{
    this->pal = this->pals[path];
    this->trn1->setPalette(this->pal);
    this->trn1->refreshResultingPalette();
    this->trn2->refreshResultingPalette();

    this->palWidget->setPal(this->pal);
}

void MainWindow::setTrn1(QString path)
{
    this->trn1 = this->trn1s[path];
    this->trn1->setPalette(this->pal);
    this->trn1->refreshResultingPalette();
    this->trn2->setPalette(this->trn1->getResultingPalette());
    this->trn2->refreshResultingPalette();

    this->trn1Widget->setTrn(this->trn1);
}

void MainWindow::setTrn2(QString path)
{
    this->trn2 = this->trn2s[path];
    this->trn2->setPalette(this->trn1->getResultingPalette());
    this->trn2->refreshResultingPalette();

    this->gfx->setPalette(this->trn2->getResultingPalette());

    this->trn2Widget->setTrn(this->trn2);
}

QString MainWindow::getLastFilePath()
{
    return this->lastFilePath;
}

void MainWindow::loadConfiguration()
{
    QString jsonFilePath = QCoreApplication::applicationDirPath() + "/D1GraphicsTool.config.json";
    bool configurationModified = false;

    // If configuration file exists load it otherwise create it
    if (QFile::exists(jsonFilePath)) {
        QFile loadJson(jsonFilePath);
        loadJson.open(QIODevice::ReadOnly);
        QJsonDocument loadJsonDoc = QJsonDocument::fromJson(loadJson.readAll());
        delete this->configuration;
        this->configuration = new QJsonObject(loadJsonDoc.object());
        loadJson.close();

        if (!this->configuration->contains("LastFilePath")) {
            this->configuration->insert("LastFilePath", jsonFilePath);
            configurationModified = true;
        }
        if (!this->configuration->contains("PaletteDefaultColor")) {
            this->configuration->insert("PaletteDefaultColor", "#FF00FF");
            configurationModified = true;
        }
        if (!this->configuration->contains("PaletteSelectionBorderColor")) {
            this->configuration->insert("PaletteSelectionBorderColor", "#FF0000");
            configurationModified = true;
        }
    } else {
        this->configuration->insert("LastFilePath", jsonFilePath);
        this->configuration->insert("PaletteDefaultColor", "#FF00FF");
        this->configuration->insert("PaletteSelectionBorderColor", "#FF0000");
        configurationModified = true;
    }

    if (configurationModified) {
        QFile saveJson(jsonFilePath);
        saveJson.open(QIODevice::WriteOnly);
        QJsonDocument saveDoc(*this->configuration);
        saveJson.write(saveDoc.toJson());
        saveJson.close();
    }
}

void MainWindow::updateWindow()
{
    // rebuild palette hits
    this->palHits->buildPalHits();
    this->palWidget->refresh();
    this->undoStack->clear();
    // update menu options
    bool hasFrame = this->gfx->getFrameCount() != 0;
    this->frameMenu->actions()[2]->setEnabled(hasFrame); // replace frame
    this->frameMenu->actions()[3]->setEnabled(hasFrame); // delete frame
    if (this->levelCelView != nullptr) {
        bool hasSubtile = this->min->getSubtileCount() != 0;
        this->subtileMenu->actions()[3]->setEnabled(hasSubtile); // replace subtile
        this->subtileMenu->actions()[4]->setEnabled(hasSubtile); // delete subtile
        this->tileMenu->actions()[0]->setEnabled(hasSubtile);    // create tile
        bool hasTile = this->til->getTileCount() != 0;
        this->tileMenu->actions()[3]->setEnabled(hasTile); // replace tile
        this->tileMenu->actions()[4]->setEnabled(hasTile); // delete tile
    }
}

bool MainWindow::loadPal(QString palFilePath)
{
    QFileInfo palFileInfo(palFilePath);
    // QString path = palFileInfo.absoluteFilePath();
    QString &path = palFilePath;
    QString name = palFileInfo.fileName();

    D1Pal *newPal = new D1Pal();
    if (!newPal->load(path)) {
        delete newPal;
        QMessageBox::critical(this, "Error", "Could not load PAL file.");
        return false;
    }

    if (this->pals.contains(path))
        delete this->pals[path];
    this->pals[path] = newPal;
    this->palWidget->addPath(path, name);
    return true;
}

bool MainWindow::loadTrn1(QString trnFilePath)
{
    QFileInfo trnFileInfo(trnFilePath);
    // QString path = trnFileInfo.absoluteFilePath();
    QString &path = trnFilePath;
    QString name = trnFileInfo.fileName();

    D1Trn *newTrn = new D1Trn(this->pal);
    if (!newTrn->load(path)) {
        delete newTrn;
        QMessageBox::critical(this, "Error", "Could not load TRN file.");
        return false;
    }

    if (this->trn1s.contains(path))
        delete this->trn1s[path];
    this->trn1s[path] = newTrn;
    this->trn1Widget->addPath(path, name);
    return true;
}

bool MainWindow::loadTrn2(QString trnFilePath)
{
    QFileInfo trnFileInfo(trnFilePath);
    // QString path = trnFileInfo.absoluteFilePath();
    QString &path = trnFilePath;
    QString name = trnFileInfo.fileName();

    D1Trn *newTrn = new D1Trn(this->pal);
    if (!newTrn->load(path)) {
        delete newTrn;
        QMessageBox::critical(this, "Error", "Could not load TRN file.");
        return false;
    }

    if (this->trn2s.contains(path))
        delete this->trn2s[path];
    this->trn2s[path] = newTrn;
    this->trn2Widget->addPath(path, name);
    return true;
}

void MainWindow::paletteWidget_callback(PaletteWidget *widget, PWIDGET_CALLBACK_TYPE type)
{
    if (widget == this->palWidget) {
        switch (type) {
        case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_NEW:
            this->on_actionNew_PAL_triggered();
            break;
        case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_OPEN:
            this->on_actionOpen_PAL_triggered();
            break;
        case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_SAVE:
            this->on_actionSave_PAL_triggered();
            break;
        case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_SAVEAS:
            this->on_actionSave_PAL_as_triggered();
            break;
        case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_CLOSE:
            this->on_actionClose_PAL_triggered();
            break;
        }
    } else if (widget == this->trn1Widget) {
        switch (type) {
        case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_NEW:
            this->on_actionNew_Translation_1_triggered();
            break;
        case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_OPEN:
            this->on_actionOpen_Translation_1_triggered();
            break;
        case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_SAVE:
            this->on_actionSave_Translation_1_triggered();
            break;
        case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_SAVEAS:
            this->on_actionSave_Translation_1_as_triggered();
            break;
        case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_CLOSE:
            this->on_actionClose_Translation_1_triggered();
            break;
        }
    } else if (widget == this->trn2Widget) {
        switch (type) {
        case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_NEW:
            this->on_actionNew_Translation_2_triggered();
            break;
        case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_OPEN:
            this->on_actionOpen_Translation_2_triggered();
            break;
        case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_SAVE:
            this->on_actionSave_Translation_2_triggered();
            break;
        case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_SAVEAS:
            this->on_actionSave_Translation_2_as_triggered();
            break;
        case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_CLOSE:
            this->on_actionClose_Translation_2_triggered();
            break;
        }
    }
}

void MainWindow::initPaletteCycle()
{
    for (int i = 0; i < 32; i++)
        this->origCyclePalette[i] = this->pal->getColor(i);
}

void MainWindow::resetPaletteCycle()
{
    for (int i = 0; i < 32; i++)
        this->pal->setColor(i, this->origCyclePalette[i]);

    this->palWidget->modify();
}

void MainWindow::nextPaletteCycle(D1PAL_CYCLE_TYPE type)
{
    this->pal->cycleColors(type);
    this->palWidget->modify();
}

static QString prepareFilePath(QString filePath, const char *filter)
{
    if (!filePath.isEmpty()) {
        // filter file-name unless it matches the filter
        QString pattern = QString(filter);
        pattern = pattern.mid(pattern.lastIndexOf('(', pattern.length() - 1) + 1, -1);
        pattern.chop(1);
        QStringList patterns = pattern.split(QRegularExpression(" "), Qt::SkipEmptyParts);
        bool match = false;
        for (int i = 0; i < patterns.size(); i++) {
            pattern = patterns.at(i);
            // convert filter to regular expression
            for (int n = 0; n < pattern.size(); n++) {
                if (pattern[n] == '*') {
                    // convert * to .*
                    pattern.insert(n, '.');
                    n++;
                } else if (pattern[n] == '.') {
                    // convert . to \.
                    pattern.insert(n, '\\');
                    n++;
                }
            }
            QRegularExpression re(pattern);
            QRegularExpressionMatch qmatch = re.match(filePath);
            match |= qmatch.hasMatch();
        }
        if (!match) {
            QFileInfo fi(filePath);
            filePath = fi.absolutePath();
        }
    }
    return filePath;
}

QString MainWindow::fileDialog(FILE_DIALOG_MODE mode, const char *title, const char *filter)
{
    QString filePath = prepareFilePath(this->lastFilePath, filter);

    if (mode == FILE_DIALOG_MODE::OPEN) {
        filePath = QFileDialog::getOpenFileName(this, title, filePath, filter);
    } else {
        filePath = QFileDialog::getSaveFileName(this, title, filePath, filter, nullptr, mode == FILE_DIALOG_MODE::SAVE_NO_CONF ? QFileDialog::DontConfirmOverwrite : QFileDialog::Options());
    }

    if (!filePath.isEmpty()) {
        this->lastFilePath = filePath;
    }
    return filePath;
}

QStringList MainWindow::filesDialog(const char *title, const char *filter)
{
    QString filePath = prepareFilePath(this->lastFilePath, filter);

    QStringList filePaths = QFileDialog::getOpenFileNames(this, title, filePath, filter);

    if (!filePaths.isEmpty()) {
        this->lastFilePath = filePaths[0];
    }
    return filePaths;
}

bool MainWindow::hasImageUrl(const QMimeData *mimeData)
{
    const QByteArrayList supportedMimeTypes = QImageReader::supportedMimeTypes();
    QMimeDatabase mimeDB;

    for (const QUrl &url : mimeData->urls()) {
        QMimeType mimeType = mimeDB.mimeTypeForFile(url.toLocalFile());
        for (const QByteArray &mimeTypeName : supportedMimeTypes) {
            if (mimeType.inherits(mimeTypeName)) {
                return true;
            }
        }
    }
    return false;
}

void MainWindow::on_actionNew_CEL_triggered()
{
    OpenAsParam params;
    params.isTileset = OPEN_TILESET_TYPE::FALSE;
    params.clipped = OPEN_CLIPPED_TYPE::FALSE;
    this->openFile(params);
}

void MainWindow::on_actionNew_CL2_triggered()
{
    OpenAsParam params;
    params.isTileset = OPEN_TILESET_TYPE::FALSE;
    params.clipped = OPEN_CLIPPED_TYPE::TRUE;
    this->openFile(params);
}

void MainWindow::on_actionNew_Tileset_triggered()
{
    OpenAsParam params;
    params.isTileset = OPEN_TILESET_TYPE::TRUE;
    this->openFile(params);
}

void MainWindow::on_actionOpen_triggered()
{
    QString openFilePath = this->fileDialog(FILE_DIALOG_MODE::OPEN, "Open Graphics", "CEL/CL2/CLX Files (*.cel *.CEL *.cl2 *.CL2 *.clx *.CLX)");

    if (!openFilePath.isEmpty()) {
        OpenAsParam params;
        params.celFilePath = openFilePath;
        this->openFile(params);
    }

    // QMessageBox::information( this, "Debug", celFilePath );
    // QMessageBox::information( this, "Debug", QString::number(cel->getFrameCount()));

    // QTime timer = QTime();
    // timer.start();
    // QMessageBox::information( this, "time", QString::number(timer.elapsed()) );
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    this->dragMoveEvent(event);
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    for (const QUrl &url : event->mimeData()->urls()) {
        QString path = url.toLocalFile().toLower();
        if (path.endsWith(".cel") || path.endsWith(".cl2") || path.endsWith(".clx")) {
            event->acceptProposedAction();
            return;
        }
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();

    OpenAsParam params;
    for (const QUrl &url : event->mimeData()->urls()) {
        params.celFilePath = url.toLocalFile();
        this->openFile(params);
    }
}

void MainWindow::openFile(const OpenAsParam &params)
{
    QString openFilePath = params.celFilePath;

    // Check file extension
    if (!openFilePath.isEmpty()
        && !openFilePath.toLower().endsWith(".cel")
        && !openFilePath.toLower().endsWith(".cl2")
        && !openFilePath.endsWith(".clx")) {
        return;
    }

    this->on_actionClose_triggered();

    this->ui->statusBar->showMessage("Loading...");
    this->ui->statusBar->repaint();

    // Loading default.pal
    D1Pal *newPal = new D1Pal();
    newPal->load(D1Pal::DEFAULT_PATH);
    this->pals[D1Pal::DEFAULT_PATH] = newPal;
    this->pal = newPal;

    // Loading default null.trn
    D1Trn *newTrn = new D1Trn(this->pal);
    newTrn->load(D1Trn::IDENTITY_PATH);
    this->trn1s[D1Trn::IDENTITY_PATH] = newTrn;
    this->trn1 = newTrn;
    newTrn = new D1Trn(this->trn1->getResultingPalette());
    newTrn->load(D1Trn::IDENTITY_PATH);
    this->trn2s[D1Trn::IDENTITY_PATH] = newTrn;
    this->trn2 = newTrn;

    QFileInfo celFileInfo = QFileInfo(openFilePath);

    // If a SOL, MIN and TIL files exists then build a LevelCelView
    QString basePath = celFileInfo.absolutePath() + "/" + celFileInfo.completeBaseName();
    QString tilFilePath = params.tilFilePath;
    QString minFilePath = params.minFilePath;
    QString solFilePath = params.solFilePath;
    if (!openFilePath.isEmpty() && tilFilePath.isEmpty()) {
        tilFilePath = basePath + ".til";
    }
    if (!openFilePath.isEmpty() && minFilePath.isEmpty()) {
        minFilePath = basePath + ".min";
    }
    if (!openFilePath.isEmpty() && solFilePath.isEmpty()) {
        solFilePath = basePath + ".sol";
    }

    bool isTileset = params.isTileset == OPEN_TILESET_TYPE::TRUE;
    if (params.isTileset == OPEN_TILESET_TYPE::AUTODETECT) {
        isTileset = openFilePath.toLower().endsWith(".cel") && QFileInfo::exists(tilFilePath) && QFileInfo::exists(minFilePath) && QFileInfo::exists(solFilePath);
    }

    this->gfx = new D1Gfx();
    this->gfx->setPalette(this->trn2->getResultingPalette());
    if (isTileset) {
        // Loading SOL
        this->sol = new D1Sol();
        if (!this->sol->load(solFilePath)) {
            QMessageBox::critical(this, "Error", "Failed loading SOL file: " + minFilePath);
            return;
        }

        // Loading MIN
        this->min = new D1Min();
        std::map<unsigned, D1CEL_FRAME_TYPE> celFrameTypes;
        if (!this->min->load(minFilePath, this->gfx, this->sol, celFrameTypes, params)) {
            QMessageBox::critical(this, "Error", "Failed loading MIN file: " + minFilePath);
            return;
        }

        // Loading TIL
        this->til = new D1Til();
        if (!this->til->load(tilFilePath, this->min)) {
            QMessageBox::critical(this, "Error", "Failed loading TIL file: " + tilFilePath);
            return;
        }

        // Loading AMP
        this->amp = new D1Amp();
        QString ampFilePath = params.ampFilePath;
        if (!openFilePath.isEmpty() && ampFilePath.isEmpty()) {
            ampFilePath = basePath + ".amp";
        }
        if (!this->amp->load(ampFilePath, this->til->getTileCount(), params)) {
            QMessageBox::critical(this, "Error", "Failed loading AMP file: " + ampFilePath);
            return;
        }

        // Loading CEL
        if (!D1CelTileset::load(*this->gfx, celFrameTypes, openFilePath, params)) {
            QMessageBox::critical(this, "Error", "Failed loading level CEL file: " + openFilePath);
            return;
        }
    } else if (openFilePath.toLower().endsWith(".cel")) {
        if (!D1Cel::load(*this->gfx, openFilePath, params)) {
            QMessageBox::critical(this, "Error", "Failed loading CEL file: " + openFilePath);
            return;
        }
    } else if (openFilePath.toLower().endsWith(".cl2")) {
        if (!D1Cl2::load(*this->gfx, openFilePath, false, params)) {
            QMessageBox::critical(this, "Error", "Failed loading CL2 file: " + openFilePath);
            return;
        }
    } else if (openFilePath.toLower().endsWith(".clx")) {
        if (!D1Cl2::load(*this->gfx, openFilePath, true, params)) {
            QMessageBox::critical(this, "Error", "Failed loading CLX file: " + openFilePath);
            return;
        }
    } else {
        // openFilePath.isEmpty()
        this->gfx->setType(params.clipped == OPEN_CLIPPED_TYPE::TRUE ? D1CEL_TYPE::V2_MONO_GROUP : D1CEL_TYPE::V1_REGULAR);
    }

    // Add palette widgets for PAL and TRNs
    this->palWidget = new PaletteWidget(this->configuration, this->undoStack, "Palette");
    this->trn2Widget = new PaletteWidget(this->configuration, this->undoStack, "Translation");
    this->trn1Widget = new PaletteWidget(this->configuration, this->undoStack, "Unique translation");
    this->ui->palFrame->layout()->addWidget(this->palWidget);
    this->ui->palFrame->layout()->addWidget(this->trn2Widget);
    this->ui->palFrame->layout()->addWidget(this->trn1Widget);

    // Configuration update triggers refresh of the palette widgets
    QObject::connect(this->settingsDialog, &SettingsDialog::configurationSaved, this->palWidget, &PaletteWidget::reloadConfig);
    QObject::connect(this->settingsDialog, &SettingsDialog::configurationSaved, this->trn1Widget, &PaletteWidget::reloadConfig);
    QObject::connect(this->settingsDialog, &SettingsDialog::configurationSaved, this->trn2Widget, &PaletteWidget::reloadConfig);
    QObject::connect(this->settingsDialog, &SettingsDialog::configurationSaved, this->palWidget, &PaletteWidget::refresh);

    // Palette and translation file selection
    // When a .pal or .trn file is selected in the PaletteWidget update the pal or trn
    QObject::connect(this->palWidget, &PaletteWidget::pathSelected, this, &MainWindow::setPal);
    QObject::connect(this->trn1Widget, &PaletteWidget::pathSelected, this, &MainWindow::setTrn1);
    QObject::connect(this->trn2Widget, &PaletteWidget::pathSelected, this, &MainWindow::setTrn2);

    // Refresh PAL/TRN view chain
    QObject::connect(this->palWidget, &PaletteWidget::refreshed, this->trn1Widget, &PaletteWidget::refresh);
    QObject::connect(this->trn1Widget, &PaletteWidget::refreshed, this->trn2Widget, &PaletteWidget::refresh);

    // Translation color selection
    QObject::connect(this->palWidget, &PaletteWidget::colorsSelected, this->trn2Widget, &PaletteWidget::checkTranslationsSelection);
    QObject::connect(this->trn2Widget, &PaletteWidget::colorsSelected, this->trn1Widget, &PaletteWidget::checkTranslationsSelection);
    QObject::connect(this->trn2Widget, &PaletteWidget::displayAllRootColors, this->palWidget, &PaletteWidget::temporarilyDisplayAllColors);
    QObject::connect(this->trn1Widget, &PaletteWidget::displayAllRootColors, this->trn2Widget, &PaletteWidget::temporarilyDisplayAllColors);
    QObject::connect(this->trn2Widget, &PaletteWidget::displayRootInformation, this->palWidget, &PaletteWidget::displayInfo);
    QObject::connect(this->trn1Widget, &PaletteWidget::displayRootInformation, this->trn2Widget, &PaletteWidget::displayInfo);
    QObject::connect(this->trn2Widget, &PaletteWidget::displayRootBorder, this->palWidget, &PaletteWidget::displayBorder);
    QObject::connect(this->trn1Widget, &PaletteWidget::displayRootBorder, this->trn2Widget, &PaletteWidget::displayBorder);
    QObject::connect(this->trn2Widget, &PaletteWidget::clearRootInformation, this->palWidget, &PaletteWidget::clearInfo);
    QObject::connect(this->trn1Widget, &PaletteWidget::clearRootInformation, this->trn2Widget, &PaletteWidget::clearInfo);
    QObject::connect(this->trn2Widget, &PaletteWidget::clearRootBorder, this->palWidget, &PaletteWidget::clearBorder);
    QObject::connect(this->trn1Widget, &PaletteWidget::clearRootBorder, this->trn2Widget, &PaletteWidget::clearBorder);

    if (isTileset) {
        this->levelCelView = new LevelCelView();
        this->levelCelView->initialize(this->gfx, this->min, this->til, this->sol, this->amp);

        // Refresh CEL view if a PAL or TRN is modified
        QObject::connect(this->palWidget, &PaletteWidget::modified, this->levelCelView, &LevelCelView::displayFrame);
        QObject::connect(this->trn1Widget, &PaletteWidget::modified, this->levelCelView, &LevelCelView::displayFrame);
        QObject::connect(this->trn2Widget, &PaletteWidget::modified, this->levelCelView, &LevelCelView::displayFrame);

        // Select color when level CEL view clicked
        QObject::connect(this->levelCelView, &LevelCelView::colorIndexClicked, this->palWidget, &PaletteWidget::selectColor);
        QObject::connect(this->levelCelView, &LevelCelView::colorIndexClicked, this->trn1Widget, &PaletteWidget::selectColor);
        QObject::connect(this->levelCelView, &LevelCelView::colorIndexClicked, this->trn2Widget, &PaletteWidget::selectColor);

        // Refresh palette widgets when frame, subtile of tile is changed
        QObject::connect(this->levelCelView, &LevelCelView::frameRefreshed, this->palWidget, &PaletteWidget::refresh);

        // Initialize palette widgets
        this->palHits = new D1PalHits(this->gfx, this->min, this->til, this->sol);
        this->palWidget->initialize(this->pal, this->levelCelView, this->palHits);
        this->trn1Widget->initialize(this->pal, this->trn1, this->levelCelView, this->palHits);
        this->trn2Widget->initialize(this->trn1->getResultingPalette(), this->trn2, this->levelCelView, this->palHits);

        this->levelCelView->displayFrame();
    }
    // Otherwise build a CelView
    else {
        this->celView = new CelView();
        this->celView->initialize(this->gfx);

        // Refresh CEL view if a PAL or TRN is modified
        QObject::connect(this->palWidget, &PaletteWidget::modified, this->celView, &CelView::displayFrame);
        QObject::connect(this->trn1Widget, &PaletteWidget::modified, this->celView, &CelView::displayFrame);
        QObject::connect(this->trn2Widget, &PaletteWidget::modified, this->celView, &CelView::displayFrame);

        // Select color when CEL view clicked
        QObject::connect(this->celView, &CelView::colorIndexClicked, this->palWidget, &PaletteWidget::selectColor);
        QObject::connect(this->celView, &CelView::colorIndexClicked, this->trn1Widget, &PaletteWidget::selectColor);
        QObject::connect(this->celView, &CelView::colorIndexClicked, this->trn2Widget, &PaletteWidget::selectColor);

        // Refresh palette widgets when frame
        QObject::connect(this->celView, &CelView::frameRefreshed, this->palWidget, &PaletteWidget::refresh);

        // Initialize palette widgets
        this->palHits = new D1PalHits(this->gfx);
        this->palWidget->initialize(this->pal, this->celView, this->palHits);
        this->trn1Widget->initialize(this->pal, this->trn1, this->celView, this->palHits);
        this->trn2Widget->initialize(this->trn1->getResultingPalette(), this->trn2, this->celView, this->palHits);

        this->celView->displayFrame();
    }

    // Look for all palettes in the same folder as the CEL/CL2 file
    QDirIterator it(celFileInfo.absolutePath(), QStringList() << "*.pal", QDir::Files);
    QString firstPaletteFound = QString();
    while (it.hasNext()) {
        QString sPath = it.next();

        if (this->loadPal(sPath) && firstPaletteFound.isEmpty()) {
            firstPaletteFound = sPath;
        }
    }
    // Select the first palette found in the same folder as the CEL/CL2 if it exists
    if (!firstPaletteFound.isEmpty())
        this->palWidget->selectPath(firstPaletteFound);

    // Adding the CelView to the main frame
    this->ui->mainFrame->layout()->addWidget(isTileset ? (QWidget *)this->levelCelView : this->celView);

    // Adding the PalView to the pal frame
    // this->ui->palFrame->layout()->addWidget( this->palView );

    // update available menu entries
    this->ui->menuEdit->setEnabled(true);
    this->ui->menuPalette->setEnabled(true);
    this->ui->actionExport->setEnabled(true);
    this->ui->actionSave->setEnabled(true);
    this->ui->actionSaveAs->setEnabled(true);
    this->ui->actionClose->setEnabled(true);

    this->subtileMenu->setEnabled(isTileset);
    this->tileMenu->setEnabled(isTileset);

    this->ui->menuTileset->setEnabled(isTileset);

    this->updateWindow();

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::openImageFiles(IMAGE_FILE_MODE mode, QStringList filePaths, bool append)
{
    if (filePaths.isEmpty()) {
        return;
    }

    this->ui->statusBar->showMessage("Reading...");
    this->ui->statusBar->repaint();

    if (this->celView != nullptr) {
        this->celView->insertImageFiles(mode, filePaths, append);
    }
    if (this->levelCelView != nullptr) {
        this->levelCelView->insertImageFiles(mode, filePaths, append);
    }
    this->updateWindow();

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::openPalFiles(QStringList filePaths, PaletteWidget *widget)
{
    QString firstFound;

    this->ui->statusBar->showMessage("Reading...");
    this->ui->statusBar->repaint();

    if (widget == this->palWidget) {
        for (QString path : filePaths) {
            if (this->loadPal(path) && firstFound.isEmpty()) {
                firstFound = path;
            }
        }
        if (!firstFound.isEmpty()) {
            this->palWidget->selectPath(firstFound);
        }
    } else if (widget == this->trn1Widget) {
        for (QString path : filePaths) {
            if (this->loadTrn1(path) && firstFound.isEmpty()) {
                firstFound = path;
            }
        }
        if (!firstFound.isEmpty()) {
            this->trn1Widget->selectPath(firstFound);
        }
    } else if (widget == this->trn2Widget) {
        for (QString path : filePaths) {
            if (this->loadTrn2(path) && firstFound.isEmpty()) {
                firstFound = path;
            }
        }
        if (!firstFound.isEmpty()) {
            this->trn2Widget->selectPath(firstFound);
        }
    }

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::saveFile(const SaveAsParam &params)
{
    this->ui->statusBar->showMessage("Saving...");
    this->ui->statusBar->repaint();

    bool change = false;
    QString filePath = params.celFilePath.isEmpty() ? this->gfx->getFilePath() : params.celFilePath;
    if (this->gfx->getType() == D1CEL_TYPE::V1_LEVEL) {
        if (!filePath.toLower().endsWith("cel")) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(nullptr, "Confirmation", "Are you sure you want to save as " + filePath + "? Data conversion is not supported.", QMessageBox::Yes | QMessageBox::No);
            if (reply != QMessageBox::Yes) {
                // Clear loading message from status bar
                this->ui->statusBar->clearMessage();
                return;
            }
        }
        change = D1CelTileset::save(*this->gfx, params);
    } else {
        if (filePath.toLower().endsWith("cel")) {
            change = D1Cel::save(*this->gfx, params);
        } else if (filePath.toLower().endsWith("cl2")) {
            change = D1Cl2::save(*this->gfx, false, params);
        } else if (filePath.toLower().endsWith("clx")) {
            change = D1Cl2::save(*this->gfx, true, params);
        } else {
            QMessageBox::critical(this, "Error", "Not supported.");
            // Clear loading message from status bar
            this->ui->statusBar->clearMessage();
            return;
        }
    }

    if (this->min != nullptr) {
        change |= this->min->save(params);
    }
    if (this->til != nullptr) {
        change |= this->til->save(params);
    }
    if (this->sol != nullptr) {
        change |= this->sol->save(params);
    }
    if (this->amp != nullptr) {
        change |= this->amp->save(params);
    }

    if (change) {
        // update view
        if (this->celView != nullptr) {
            this->celView->initialize(this->gfx);
        }
        if (this->levelCelView != nullptr) {
            this->levelCelView->initialize(this->gfx, this->min, this->til, this->sol, this->amp);
        }
    }

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

static QString imageNameFilter()
{
    // get supported image file types
    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = QImageReader::supportedMimeTypes();
    for (const QByteArray &mimeTypeName : supportedMimeTypes) {
        mimeTypeFilters.append(mimeTypeName);
    }

    // compose filter for all supported types
    QMimeDatabase mimeDB;
    QStringList allSupportedFormats;
    for (const QString &mimeTypeFilter : mimeTypeFilters) {
        QMimeType mimeType = mimeDB.mimeTypeForName(mimeTypeFilter);
        if (mimeType.isValid()) {
            QStringList mimePatterns = mimeType.globPatterns();
            for (int i = 0; i < mimePatterns.count(); i++) {
                allSupportedFormats.append(mimePatterns[i]);
                allSupportedFormats.append(mimePatterns[i].toUpper());
            }
        }
    }
    QString allSupportedFormatsFilter = QString("Image files (%1)").arg(allSupportedFormats.join(' '));
    return allSupportedFormatsFilter;
}

void MainWindow::addFrames(bool append)
{
    QString filter = imageNameFilter();
    QStringList files = this->filesDialog("Select Image Files", filter.toLatin1().data());

    this->openImageFiles(IMAGE_FILE_MODE::FRAME, files, append);
}

void MainWindow::addSubtiles(bool append)
{
    QString filter = imageNameFilter();
    QStringList files = this->filesDialog("Select Image Files", filter.toLatin1().data());

    this->openImageFiles(IMAGE_FILE_MODE::SUBTILE, files, append);
}

void MainWindow::addTiles(bool append)
{
    QString filter = imageNameFilter();
    QStringList files = this->filesDialog("Select Image Files", filter.toLatin1().data());

    this->openImageFiles(IMAGE_FILE_MODE::TILE, files, append);
}

void MainWindow::on_actionOpenAs_triggered()
{
    this->openAsDialog->initialize(this->configuration);
    this->openAsDialog->show();
}

void MainWindow::on_actionSave_triggered()
{
    if (this->gfx->getFilePath().isEmpty()) {
        this->on_actionSaveAs_triggered();
        return;
    }
    SaveAsParam params;
    this->saveFile(params);
}

void MainWindow::on_actionSaveAs_triggered()
{
    this->saveAsDialog->initialize(this->configuration, this->gfx, this->min, this->til, this->sol, this->amp);
    this->saveAsDialog->show();
}

void MainWindow::on_actionClose_triggered()
{
    this->undoStack->clear();

    delete this->celView;
    delete this->levelCelView;
    delete this->palWidget;
    delete this->trn1Widget;
    delete this->trn2Widget;
    delete this->gfx;

    qDeleteAll(this->pals);
    this->pals.clear();

    qDeleteAll(this->trn1s);
    this->trn1s.clear();

    qDeleteAll(this->trn2s);
    this->trn2s.clear();

    delete this->min;
    delete this->til;
    delete this->sol;
    delete this->amp;
    delete this->palHits;

    // update available menu entries
    this->ui->menuEdit->setEnabled(false);
    this->ui->menuTileset->setEnabled(false);
    this->ui->menuPalette->setEnabled(false);
    this->ui->actionExport->setEnabled(false);
    this->ui->actionSave->setEnabled(false);
    this->ui->actionSaveAs->setEnabled(false);
    this->ui->actionClose->setEnabled(false);
}

void MainWindow::on_actionSettings_triggered()
{
    this->settingsDialog->initialize(this->configuration);
    this->settingsDialog->show();
}

void MainWindow::on_actionExport_triggered()
{
    this->exportDialog->initialize(this->configuration, this->gfx, this->min, this->til, this->sol, this->amp);
    this->exportDialog->show();
}

void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
}

void MainWindow::on_actionInsert_Frame_triggered()
{
    this->addFrames(false);
}

void MainWindow::on_actionAdd_Frame_triggered()
{
    this->addFrames(true);
}

void MainWindow::on_actionReplace_Frame_triggered()
{
    QString filter = imageNameFilter();
    QString imgFilePath = this->fileDialog(FILE_DIALOG_MODE::OPEN, "Replacement Image File", filter.toLatin1().data());

    if (imgFilePath.isEmpty()) {
        return;
    }

    this->ui->statusBar->showMessage("Reading...");
    this->ui->statusBar->repaint();

    if (this->celView != nullptr) {
        this->celView->replaceCurrentFrame(imgFilePath);
    }
    if (this->levelCelView != nullptr) {
        this->levelCelView->replaceCurrentFrame(imgFilePath);
    }
    this->updateWindow();

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::on_actionDel_Frame_triggered()
{
    if (this->celView != nullptr) {
        this->celView->removeCurrentFrame();
    }
    if (this->levelCelView != nullptr) {
        this->levelCelView->removeCurrentFrame();
    }
    this->updateWindow();
}

void MainWindow::on_actionCreate_Subtile_triggered()
{
    this->levelCelView->createSubtile();
    this->updateWindow();
}

void MainWindow::on_actionClone_Subtile_triggered()
{
    this->levelCelView->cloneSubtile();
    this->updateWindow();
}

void MainWindow::on_actionInsert_Subtile_triggered()
{
    this->addSubtiles(false);
}

void MainWindow::on_actionAdd_Subtile_triggered()
{
    this->addSubtiles(true);
}

void MainWindow::on_actionReplace_Subtile_triggered()
{
    QString filter = imageNameFilter();
    QString imgFilePath = this->fileDialog(FILE_DIALOG_MODE::OPEN, "Replacement Image File", filter.toLatin1().data());

    if (imgFilePath.isEmpty()) {
        return;
    }

    this->ui->statusBar->showMessage("Reading...");
    this->ui->statusBar->repaint();

    this->levelCelView->replaceCurrentSubtile(imgFilePath);

    this->updateWindow();

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::on_actionDel_Subtile_triggered()
{
    this->levelCelView->removeCurrentSubtile();
    this->updateWindow();
}

void MainWindow::on_actionCreate_Tile_triggered()
{
    this->levelCelView->createTile();
    this->updateWindow();
}

void MainWindow::on_actionClone_Tile_triggered()
{
    this->levelCelView->cloneTile();
    this->updateWindow();
}

void MainWindow::on_actionInsert_Tile_triggered()
{
    this->addTiles(false);
}

void MainWindow::on_actionAdd_Tile_triggered()
{
    this->addTiles(true);
}

void MainWindow::on_actionReplace_Tile_triggered()
{
    QString filter = imageNameFilter();
    QString imgFilePath = this->fileDialog(FILE_DIALOG_MODE::OPEN, "Replacement Image File", filter.toLatin1().data());

    if (imgFilePath.isEmpty()) {
        return;
    }

    this->ui->statusBar->showMessage("Reading...");
    this->ui->statusBar->repaint();

    this->levelCelView->replaceCurrentTile(imgFilePath);

    this->updateWindow();

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::on_actionDel_Tile_triggered()
{
    this->levelCelView->removeCurrentTile();
    this->updateWindow();
}

void MainWindow::on_actionReportUse_Tileset_triggered()
{
    this->levelCelView->reportUsage();
}

void MainWindow::on_actionResetFrameTypes_Tileset_triggered()
{
    this->ui->statusBar->showMessage("Processing...");
    this->ui->statusBar->repaint();

    this->levelCelView->resetFrameTypes();

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::on_actionCleanupFrames_Tileset_triggered()
{
    this->ui->statusBar->showMessage("Processing...");
    this->ui->statusBar->repaint();

    this->levelCelView->cleanupFrames();

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::on_actionCleanupSubtiles_Tileset_triggered()
{
    this->ui->statusBar->showMessage("Processing...");
    this->ui->statusBar->repaint();

    this->levelCelView->cleanupSubtiles();

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::on_actionCleanupTileset_Tileset_triggered()
{
    this->ui->statusBar->showMessage("Processing...");
    this->ui->statusBar->repaint();

    this->levelCelView->cleanupTileset();

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::on_actionCompressSubtiles_Tileset_triggered()
{
    this->ui->statusBar->showMessage("Processing...");
    this->ui->statusBar->repaint();

    this->levelCelView->compressSubtiles();

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::on_actionCompressTiles_Tileset_triggered()
{
    this->ui->statusBar->showMessage("Processing...");
    this->ui->statusBar->repaint();

    this->levelCelView->compressTiles();

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::on_actionCompressTileset_Tileset_triggered()
{
    this->ui->statusBar->showMessage("Processing...");
    this->ui->statusBar->repaint();

    this->levelCelView->compressTileset();

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::on_actionSortFrames_Tileset_triggered()
{
    this->ui->statusBar->showMessage("Processing...");
    this->ui->statusBar->repaint();

    this->levelCelView->sortFrames();

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::on_actionSortSubtiles_Tileset_triggered()
{
    this->ui->statusBar->showMessage("Processing...");
    this->ui->statusBar->repaint();

    this->levelCelView->sortSubtiles();

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::on_actionSortTileset_Tileset_triggered()
{
    this->ui->statusBar->showMessage("Processing...");
    this->ui->statusBar->repaint();

    this->levelCelView->sortTileset();

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::on_actionNew_PAL_triggered()
{
    QString palFilePath = this->fileDialog(FILE_DIALOG_MODE::SAVE_CONF, "New Palette File", "PAL Files (*.pal *.PAL)");

    if (palFilePath.isEmpty()) {
        return;
    }

    QFileInfo palFileInfo(palFilePath);
    QString path = palFileInfo.absoluteFilePath();
    QString name = palFileInfo.fileName();

    D1Pal *newPal = new D1Pal();
    if (!newPal->load(D1Pal::DEFAULT_PATH)) {
        delete newPal;
        QMessageBox::critical(this, "Error", "Could not load PAL file.");
        return;
    }
    if (!newPal->save(path)) {
        delete newPal;
        QMessageBox::critical(this, "Error", "Could not save PAL file.");
        return;
    }

    if (this->pals.contains(path))
        delete this->pals[path];
    this->pals[path] = newPal;
    this->palWidget->addPath(path, name);
    this->palWidget->selectPath(path);
}

void MainWindow::on_actionOpen_PAL_triggered()
{
    QString palFilePath = this->fileDialog(FILE_DIALOG_MODE::OPEN, "Load Palette File", "PAL Files (*.pal *.PAL)");

    if (!palFilePath.isEmpty() && this->loadPal(palFilePath)) {
        this->palWidget->selectPath(palFilePath);
    }
}

void MainWindow::on_actionSave_PAL_triggered()
{
    QString selectedPath = this->palWidget->getSelectedPath();
    if (selectedPath == D1Pal::DEFAULT_PATH) {
        this->on_actionSave_PAL_as_triggered();
    } else {
        if (!this->pal->save(selectedPath)) {
            QMessageBox::critical(this, "Error", "Could not save PAL file.");
            return;
        }
    }
}

void MainWindow::on_actionSave_PAL_as_triggered()
{
    QString palFilePath = this->fileDialog(FILE_DIALOG_MODE::SAVE_CONF, "Save Palette File as...", "PAL Files (*.pal *.PAL)");

    if (palFilePath.isEmpty()) {
        return;
    }

    if (!this->pal->save(palFilePath)) {
        QMessageBox::critical(this, "Error", "Could not save PAL file.");
        return;
    }

    QFileInfo palFileInfo(palFilePath);
    QString path = palFileInfo.absoluteFilePath();
    QString name = palFileInfo.fileName();

    D1Pal *newPal = new D1Pal();
    if (!newPal->load(path)) {
        delete newPal;
        QMessageBox::critical(this, "Error", "Could not load PAL file.");
        return;
    }

    if (this->pals.contains(path))
        delete this->pals[path];
    this->pals[path] = newPal;
    this->palWidget->addPath(path, name);
    this->palWidget->selectPath(path);
}

void MainWindow::on_actionClose_PAL_triggered()
{
    QString selectedPath = this->palWidget->getSelectedPath();
    if (selectedPath == D1Pal::DEFAULT_PATH)
        return;

    if (this->pals.contains(selectedPath)) {
        delete this->pals[selectedPath];
        this->pals.remove(selectedPath);
    }

    this->palWidget->removePath(selectedPath);
    this->palWidget->selectPath(D1Pal::DEFAULT_PATH);
}

void MainWindow::on_actionNew_Translation_1_triggered()
{
    QString trnFilePath = this->fileDialog(FILE_DIALOG_MODE::SAVE_CONF, "New Translation File", "TRN Files (*.trn *.TRN)");

    if (trnFilePath.isEmpty()) {
        return;
    }

    QFileInfo trnFileInfo(trnFilePath);
    QString path = trnFileInfo.absoluteFilePath();
    QString name = trnFileInfo.fileName();

    D1Trn *newTrn = new D1Trn(this->pal);
    if (!newTrn->load(D1Trn::IDENTITY_PATH)) {
        delete newTrn;
        QMessageBox::critical(this, "Error", "Could not load TRN file.");
        return;
    }
    if (!newTrn->save(path)) {
        delete newTrn;
        QMessageBox::critical(this, "Error", "Could not save TRN file.");
        return;
    }

    if (this->trn1s.contains(path))
        delete this->trn1s[path];
    this->trn1s[path] = newTrn;
    this->trn1Widget->addPath(path, name);
    this->trn1Widget->selectPath(path);
}

void MainWindow::on_actionOpen_Translation_1_triggered()
{
    QString trnFilePath = this->fileDialog(FILE_DIALOG_MODE::OPEN, "Load Translation File", "TRN Files (*.trn *.TRN)");

    if (!trnFilePath.isEmpty() && this->loadTrn1(trnFilePath)) {
        this->trn1Widget->selectPath(trnFilePath);
    }
}

void MainWindow::on_actionSave_Translation_1_triggered()
{
    QString selectedPath = this->trn1Widget->getSelectedPath();
    if (selectedPath == D1Trn::IDENTITY_PATH) {
        this->on_actionSave_Translation_1_as_triggered();
    } else {
        if (!this->trn1->save(selectedPath)) {
            QMessageBox::critical(this, "Error", "Could not save TRN file.");
            return;
        }
    }
}

void MainWindow::on_actionSave_Translation_1_as_triggered()
{
    QString trnFilePath = this->fileDialog(FILE_DIALOG_MODE::SAVE_CONF, "Save Translation File as...", "TRN Files (*.trn *.TRN)");

    if (trnFilePath.isEmpty()) {
        return;
    }

    if (!this->trn1->save(trnFilePath)) {
        QMessageBox::critical(this, "Error", "Could not save TRN file.");
        return;
    }

    QFileInfo trnFileInfo(trnFilePath);
    QString path = trnFileInfo.absoluteFilePath();
    QString name = trnFileInfo.fileName();

    D1Trn *newTrn = new D1Trn(this->pal);
    if (!newTrn->load(path)) {
        delete newTrn;
        QMessageBox::critical(this, "Error", "Could not load TRN file.");
        return;
    }

    if (this->trn1s.contains(path))
        delete this->trn1s[path];
    this->trn1s[path] = newTrn;
    this->trn1Widget->addPath(path, name);
    this->trn1Widget->selectPath(path);
}

void MainWindow::on_actionClose_Translation_1_triggered()
{
    QString selectedPath = this->trn1Widget->getSelectedPath();
    if (selectedPath == D1Trn::IDENTITY_PATH)
        return;

    if (this->trn1s.contains(selectedPath)) {
        delete this->trn1s[selectedPath];
        this->trn1s.remove(selectedPath);
    }

    this->trn1Widget->removePath(selectedPath);
    this->trn1Widget->selectPath(D1Trn::IDENTITY_PATH);
}

void MainWindow::on_actionNew_Translation_2_triggered()
{
    QString trnFilePath = this->fileDialog(FILE_DIALOG_MODE::SAVE_CONF, "New Translation File", "TRN Files (*.trn *.TRN)");

    if (trnFilePath.isEmpty()) {
        return;
    }

    QFileInfo trnFileInfo(trnFilePath);
    QString path = trnFileInfo.absoluteFilePath();
    QString name = trnFileInfo.fileName();

    D1Trn *newTrn = new D1Trn(this->trn1->getResultingPalette());
    if (!newTrn->load(D1Trn::IDENTITY_PATH)) {
        delete newTrn;
        QMessageBox::critical(this, "Error", "Could not load TRN file.");
        return;
    }
    if (!newTrn->save(path)) {
        delete newTrn;
        QMessageBox::critical(this, "Error", "Could not save TRN file.");
        return;
    }

    if (this->trn2s.contains(path))
        delete this->trn2s[path];
    this->trn2s[path] = newTrn;
    this->trn2Widget->addPath(path, name);
    this->trn2Widget->selectPath(path);
}

void MainWindow::on_actionOpen_Translation_2_triggered()
{
    QString trnFilePath = this->fileDialog(FILE_DIALOG_MODE::OPEN, "Load Translation File", "TRN Files (*.trn *.TRN)");

    if (!trnFilePath.isEmpty() && this->loadTrn2(trnFilePath)) {
        this->trn2Widget->selectPath(trnFilePath);
    }
}

void MainWindow::on_actionSave_Translation_2_triggered()
{
    QString selectedPath = this->trn2Widget->getSelectedPath();
    if (selectedPath == D1Trn::IDENTITY_PATH) {
        this->on_actionSave_Translation_2_as_triggered();
    } else {
        if (!this->trn2->save(selectedPath)) {
            QMessageBox::critical(this, "Error", "Could not save TRN file.");
            return;
        }
    }
}

void MainWindow::on_actionSave_Translation_2_as_triggered()
{
    QString trnFilePath = this->fileDialog(FILE_DIALOG_MODE::SAVE_CONF, "Save Translation File as...", "TRN Files (*.trn *.TRN)");

    if (trnFilePath.isEmpty()) {
        return;
    }

    if (!this->trn2->save(trnFilePath)) {
        QMessageBox::critical(this, "Error", "Could not save TRN file.");
        return;
    }

    QFileInfo trnFileInfo(trnFilePath);
    QString path = trnFileInfo.absoluteFilePath();
    QString name = trnFileInfo.fileName();

    D1Trn *newTrn = new D1Trn(this->trn1->getResultingPalette());
    if (!newTrn->load(path)) {
        delete newTrn;
        QMessageBox::critical(this, "Error", "Could not load TRN file.");
        return;
    }

    if (this->trn2s.contains(path))
        delete this->trn2s[path];
    this->trn2s[path] = newTrn;
    this->trn2Widget->addPath(path, name);
    this->trn2Widget->selectPath(path);
}

void MainWindow::on_actionClose_Translation_2_triggered()
{
    QString selectedPath = this->trn2Widget->getSelectedPath();
    if (selectedPath == D1Trn::IDENTITY_PATH)
        return;

    if (this->trn2s.contains(selectedPath)) {
        delete this->trn2s[selectedPath];
        this->trn2s.remove(selectedPath);
    }

    this->trn2Widget->removePath(selectedPath);
    this->trn2Widget->selectPath(D1Trn::IDENTITY_PATH);
}

void MainWindow::on_actionAbout_triggered()
{
    QString architecture;
    QString operatingSystem;

#ifdef Q_PROCESSOR_X86_64
    architecture = "(64-bit)";
#endif

#ifdef Q_PROCESSOR_X86_32
    architecture = "(32-bit)";
#endif

#ifdef Q_OS_WIN
    operatingSystem = "Windows";
#endif

#ifdef Q_OS_MAC
    operatingSystem = "macOS";
#endif

#ifdef Q_OS_LINUX
    operatingSystem = "Linux";
#endif

    QMessageBox::about(this, "About", "Diablo 1 Graphics Tool " + QString(D1_GRAPHICS_TOOL_VERSION) + " (" + operatingSystem + ") " + architecture);
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this, "About Qt");
}
