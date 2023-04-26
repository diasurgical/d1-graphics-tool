#include "mainwindow.h"

#include <QDragEnterEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QImageReader>
#include <QInputDialog>
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
#include <QtWidgets>

#include "config.h"
#include "d1cel.h"
#include "d1celtileset.h"
#include "d1cl2.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow()
    : QMainWindow(nullptr)
    , ui(new Ui::MainWindow())
{
    // QCoreApplication::setAttribute( Qt::AA_EnableHighDpiScaling, true );

    this->lastFilePath = Config::value("LastFilePath").toString();

    ui->setupUi(this);

    this->setWindowTitle(D1_GRAPHICS_TOOL_TITLE);

    // initialize 'new' submenu of 'File'
    this->newMenu.addAction("Sprite", this, SLOT(on_actionNew_Sprite_triggered()));
    this->newMenu.addAction("Tileset", this, SLOT(on_actionNew_Tileset_triggered()));
    QAction *firstFileAction = (QAction *)this->ui->menuFile->actions()[0];
    this->ui->menuFile->insertMenu(firstFileAction, &this->newMenu);

    // Initialize 'Undo/Redo' of 'Edit
    this->undoStack = new QUndoStack(this);
    this->undoAction = undoStack->createUndoAction(this, "Undo");
    this->undoAction->setShortcuts(QKeySequence::Undo);
    this->ui->menuEdit->addAction(this->undoAction);
    this->redoAction = undoStack->createRedoAction(this, "Redo");
    this->redoAction->setShortcuts(QKeySequence::Redo);
    this->ui->menuEdit->addAction(this->redoAction);
    this->ui->menuEdit->addSeparator();

    this->ui->menuEdit->addAction(this->undoAction);
    this->ui->menuEdit->addAction(this->redoAction);
    this->ui->menuEdit->addSeparator();

    // Initialize 'Frame' submenu of 'Edit'
    this->frameMenu.setToolTipsVisible(true);
    this->frameMenu.addAction("Insert", this, SLOT(on_actionInsert_Frame_triggered()))->setToolTip("Add new frames before the current one");
    this->frameMenu.addAction("Add", this, SLOT(on_actionAdd_Frame_triggered()))->setToolTip("Add new frames at the end");
    this->frameMenu.addAction("Replace", this, SLOT(on_actionReplace_Frame_triggered()))->setToolTip("Replace the current frame");
    this->frameMenu.addAction("Delete", this, SLOT(on_actionDel_Frame_triggered()))->setToolTip("Delete the current frame");
    this->ui->menuEdit->addMenu(&this->frameMenu);

    // Initialize 'Subtile' submenu of 'Edit'
    this->subtileMenu.setToolTipsVisible(true);
    this->subtileMenu.addAction("Create", this, SLOT(on_actionCreate_Subtile_triggered()))->setToolTip("Create a new subtile");
    this->subtileMenu.addAction("Insert", this, SLOT(on_actionInsert_Subtile_triggered()))->setToolTip("Add new subtiles before the current one");
    this->subtileMenu.addAction("Add", this, SLOT(on_actionAdd_Subtile_triggered()))->setToolTip("Add new subtiles at the end");
    this->subtileMenu.addAction("Clone", this, SLOT(on_actionClone_Subtile_triggered()))->setToolTip("Add new subtiles at the end based on the current one");
    this->subtileMenu.addAction("Replace", this, SLOT(on_actionReplace_Subtile_triggered()))->setToolTip("Replace the current subtile");
    this->subtileMenu.addAction("Delete", this, SLOT(on_actionDel_Subtile_triggered()))->setToolTip("Delete the current subtile");
    this->ui->menuEdit->addMenu(&this->subtileMenu);

    // Initialize 'Tile' submenu of 'Edit'
    this->tileMenu.setToolTipsVisible(true);
    this->tileMenu.addAction("Create", this, SLOT(on_actionCreate_Tile_triggered()))->setToolTip("Create a new tile");
    this->tileMenu.addAction("Insert", this, SLOT(on_actionInsert_Tile_triggered()))->setToolTip("Add new tiles before the current one");
    this->tileMenu.addAction("Add", this, SLOT(on_actionAdd_Tile_triggered()))->setToolTip("Add new tiles at the end");
    this->tileMenu.addAction("Clone", this, SLOT(on_actionClone_Tile_triggered()))->setToolTip("Add new tile at the end based on the current one");
    this->tileMenu.addAction("Replace", this, SLOT(on_actionReplace_Tile_triggered()))->setToolTip("Replace the current tile");
    this->tileMenu.addAction("Delete", this, SLOT(on_actionDel_Tile_triggered()))->setToolTip("Delete the current tile");
    this->ui->menuEdit->addMenu(&this->tileMenu);

    this->buildRecentFilesMenu();

    this->closeAllElements();
    setAcceptDrops(true);
}

MainWindow::~MainWindow()
{
    // close modal windows
    this->closeAllElements();
    // store last path
    Config::insert("LastFilePath", this->lastFilePath);
    // cleanup memory
    delete ui;
    delete this->undoStack;
    delete this->undoAction;
    delete this->redoAction;
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

void MainWindow::updateWindow()
{
    // rebuild palette hits
    this->palHits->update();
    this->palWidget->refresh();
    this->undoStack->clear();
    // update menu options
    bool hasFrame = this->gfx->getFrameCount() != 0;
    this->frameMenu.actions()[2]->setEnabled(hasFrame); // replace frame
    this->frameMenu.actions()[3]->setEnabled(hasFrame); // delete frame
    if (this->levelCelView != nullptr) {
        bool hasSubtile = this->min->getSubtileCount() != 0;
        this->subtileMenu.actions()[3]->setEnabled(hasSubtile); // replace subtile
        this->subtileMenu.actions()[4]->setEnabled(hasSubtile); // delete subtile
        this->tileMenu.actions()[0]->setEnabled(hasSubtile);    // create tile
        bool hasTile = this->til->getTileCount() != 0;
        this->tileMenu.actions()[3]->setEnabled(hasTile); // replace tile
        this->tileMenu.actions()[4]->setEnabled(hasTile); // delete tile
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

void MainWindow::resetPaletteCycle()
{
    this->pal->resetColors();
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
        QStringList patterns = pattern.split(QRegularExpression(" "));
        bool match = false;
        for (int i = 0; i < patterns.size(); i++) {
            pattern = patterns.at(i);
            if (pattern.isEmpty())
                continue;
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

void MainWindow::on_actionNew_Sprite_triggered()
{
    OpenAsParam params;
    params.isTileset = OPEN_TILESET_TYPE::No;
    this->openFile(params);
}

void MainWindow::on_actionNew_Tileset_triggered()
{
    OpenAsParam params;
    params.isTileset = OPEN_TILESET_TYPE::Yes;
    this->openFile(params);
}

void MainWindow::on_actionRegroupFrames_triggered()
{
    bool ok;
    int numGroups = QInputDialog::getInt(this, "Regroup Frames", "Groups:", 8, 1, 127, 1, &ok, Qt::WindowFlags());
    if (!ok)
        return;

    // update group indices
    const int numFrames = this->gfx->getFrameCount();
    if (numFrames == 0 || (numFrames % numGroups) != 0) {
        QMessageBox::critical(nullptr, "Error", QString::number(numFrames) + " frames can not be split equally in " + QString::number(numGroups) + " groups.");
        return;
    }

    this->celView->regroupFrames(numGroups);
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

    this->addRecentFile(openFilePath);

    if (!this->isOkToQuit()) {
        return;
    }

    this->closeAllElements();

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

    bool isTileset = params.isTileset == OPEN_TILESET_TYPE::Yes;
    if (params.isTileset == OPEN_TILESET_TYPE::Auto) {
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
    }

    // Add palette widgets for PAL and TRNs
    this->palWidget = new PaletteWidget(this->undoStack, "Palette");
    this->trn2Widget = new PaletteWidget(this->undoStack, "Translation");
    this->trn1Widget = new PaletteWidget(this->undoStack, "Unique translation");
    this->ui->palFrame->layout()->addWidget(this->palWidget);
    this->ui->palFrame->layout()->addWidget(this->trn2Widget);
    this->ui->palFrame->layout()->addWidget(this->trn1Widget);

    // Configuration update triggers refresh of the palette widgets
    QObject::connect(&this->settingsDialog, &SettingsDialog::configurationSaved, this->palWidget, &PaletteWidget::reloadConfig);
    QObject::connect(&this->settingsDialog, &SettingsDialog::configurationSaved, this->trn1Widget, &PaletteWidget::reloadConfig);
    QObject::connect(&this->settingsDialog, &SettingsDialog::configurationSaved, this->trn2Widget, &PaletteWidget::reloadConfig);
    QObject::connect(&this->settingsDialog, &SettingsDialog::configurationSaved, this->palWidget, &PaletteWidget::refresh);

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
        this->palHits = new D1PalHits(this->gfx, this->min, this->til);
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
    this->ui->menuSprite->setEnabled(!isTileset);
    this->ui->actionCelHeader->setChecked(this->gfx->hasHeader());
    this->ui->menuEdit->setEnabled(true);
    this->ui->menuPalette->setEnabled(true);
    this->ui->actionExport->setEnabled(true);
    this->ui->actionSave->setEnabled(true);
    this->ui->actionSaveAs->setEnabled(true);
    this->ui->actionClose->setEnabled(true);

    this->subtileMenu.setEnabled(isTileset);
    this->tileMenu.setEnabled(isTileset);

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

void MainWindow::saveFile(const QString &gfxPath)
{
    this->ui->statusBar->showMessage("Saving...");
    this->ui->statusBar->repaint();

    bool change = false;
    QString filePath = gfxPath.isEmpty() ? this->gfx->getFilePath() : gfxPath;
    if (this->gfx->isTileset()) {
        change = D1CelTileset::save(*this->gfx, gfxPath);
    } else {
        if (filePath.toLower().endsWith("cel")) {
            this->gfx->setHasHeader(this->ui->actionCelHeader->isChecked());
            change = D1Cel::save(*this->gfx, gfxPath);
        } else if (filePath.toLower().endsWith("cl2")) {
            change = D1Cl2::save(*this->gfx, false, gfxPath);
        } else if (filePath.toLower().endsWith("clx")) {
            change = D1Cl2::save(*this->gfx, true, gfxPath);
        } else {
            QMessageBox::critical(this, "Error", "Not supported.");
            // Clear loading message from status bar
            this->ui->statusBar->clearMessage();
            return;
        }
    }

    this->gfx->setModified(false);

    if (this->min != nullptr) {
        change |= this->min->save(gfxPath);
    }
    if (this->til != nullptr) {
        change |= this->til->save(gfxPath);
    }
    if (this->sol != nullptr) {
        change |= this->sol->save(gfxPath);
    }
    if (this->amp != nullptr) {
        change |= this->amp->save(gfxPath);
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
    this->openAsDialog.initialize();
    this->openAsDialog.show();
}

void MainWindow::on_actionSave_triggered()
{
    if (this->gfx->getFilePath().isEmpty()) {
        this->on_actionSaveAs_triggered();
        return;
    }
    this->saveFile(this->gfx->getFilePath());
}

void MainWindow::on_actionSaveAs_triggered()
{
    bool isTileset = this->gfx->isTileset();

    const char *filter = "CEL Files (*.cel)";
    if (!isTileset) {
        filter = "CEL/CL2/CLX Files (*.cel *.cl2 *.clx)";
        if (this->gfx->getGroupCount() != 1) {
            filter = "CL2/CLX Files (*.cl2 *.clx)";
        }
    }

    QString filePath = this->fileDialog(FILE_DIALOG_MODE::SAVE_NO_CONF, "Save Graphics as...", filter);
    if (filePath.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(filePath);
    if (fileInfo.suffix().isEmpty()) {
        filePath += isTileset ? ".cel" : ".cl2";
    } else if (isTileset && fileInfo.suffix() != "cel") {
        QMessageBox::critical(nullptr, "Error", "Only .cel is supported for tilesets.");
        return;
    }

    bool fileExists = QFile::exists(filePath);
    if (isTileset) {
        QString basePath = filePath;
        basePath.chop(3);
        fileExists |= QFile::exists(basePath + "min");
        fileExists |= QFile::exists(basePath + "til");
        fileExists |= QFile::exists(basePath + "sol");
        fileExists |= QFile::exists(basePath + "amp");
    }

    if (fileExists && QMessageBox::question(nullptr, "Confirmation", "Are you sure you want to overwrite?", QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    this->saveFile(filePath);
}

namespace {

bool QuestionDiscardChanges(bool isModified, QString filePath)
{
    if (isModified) {
        QString message = "Discard unsaved changes?";
        if (!filePath.isEmpty()) {
            QFileInfo fileInfo(filePath);
            message = "Discard changes to " + fileInfo.fileName() + "?";
        }

        if (QMessageBox::question(nullptr, "Confirmation", message, QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
            return false;
        }
    }

    return true;
}

} // namespace

bool MainWindow::isOkToQuit()
{
    for (D1Pal *pallete : this->pals) {
        if (!QuestionDiscardChanges(pallete->isModified(), pallete->getFilePath())) {
            return false;
        }
    }

    for (D1Trn *translation : this->trn1s) {
        if (!QuestionDiscardChanges(translation->isModified(), translation->getFilePath())) {
            return false;
        }
    }

    for (D1Trn *translation : this->trn2s) {
        if (!QuestionDiscardChanges(translation->isModified(), translation->getFilePath())) {
            return false;
        }
    }

    if (this->min != nullptr && !QuestionDiscardChanges(this->min->isModified(), this->min->getFilePath())) {
        return false;
    }
    if (this->til != nullptr && !QuestionDiscardChanges(this->til->isModified(), this->til->getFilePath())) {
        return false;
    }
    if (this->sol != nullptr && !QuestionDiscardChanges(this->sol->isModified(), this->sol->getFilePath())) {
        return false;
    }
    if (this->amp != nullptr && !QuestionDiscardChanges(this->amp->isModified(), this->amp->getFilePath())) {
        return false;
    }
    if (this->gfx != nullptr && !QuestionDiscardChanges(this->gfx->isModified(), this->gfx->getFilePath())) {
        return false;
    }

    return true;
}

void MainWindow::on_actionClose_triggered()
{
    if (!this->isOkToQuit()) {
        return;
    }

    this->closeAllElements();
}

void MainWindow::closeAllElements()
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
    this->ui->menuSprite->setEnabled(false);
    this->ui->menuTileset->setEnabled(false);
    this->ui->menuPalette->setEnabled(false);
    this->ui->actionExport->setEnabled(false);
    this->ui->actionSave->setEnabled(false);
    this->ui->actionSaveAs->setEnabled(false);
    this->ui->actionClose->setEnabled(false);
}

void MainWindow::on_actionSettings_triggered()
{
    this->settingsDialog.initialize();
    this->settingsDialog.show();
}

void MainWindow::on_actionExport_triggered()
{
    this->exportDialog.initialize(this->gfx, this->min, this->til, this->sol, this->amp);
    this->exportDialog.show();
}

void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!this->isOkToQuit()) {
        event->ignore();
        return;
    }

    event->accept();
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

void MainWindow::buildRecentFilesMenu()
{
    this->ui->menuOpen_Recent->clear();
    for (int i = 0; i < 10; i++) {
        QString recentFile = Config::value("RecentFile" + QString::number(i)).toString();
        if (recentFile.isEmpty())
            break;

        QFileInfo fileInfo(recentFile);
        QAction *recentFileAction = this->ui->menuOpen_Recent->addAction(fileInfo.fileName());
        connect(recentFileAction, &QAction::triggered, this, [this, filePath = recentFile] {
            OpenAsParam params;
            params.celFilePath = filePath;
            this->openFile(params);
        });
    }

    this->ui->menuOpen_Recent->addSeparator();
    this->ui->menuOpen_Recent->addAction("Clear History", this, SLOT(on_actionClear_History_triggered()));
}

void MainWindow::addRecentFile(QString filePath)
{
    for (int i = 0; i < 10; i++) {
        QString recentFile = Config::value("RecentFile" + QString::number(i)).toString();
        if (recentFile.isEmpty())
            break;
        if (recentFile == filePath)
            return;
    }
    for (int i = 0; i < 9; i++) {
        Config::insert("RecentFile" + QString::number(9 - i), Config::value("RecentFile" + QString::number(8 - i)).toString());
    }
    Config::insert("RecentFile0", filePath);
    this->buildRecentFilesMenu();
}

void MainWindow::on_actionClear_History_triggered()
{
    for (int i = 0; i < 10; i++) {
        Config::insert("RecentFile" + QString::number(i), "");
    }
    this->buildRecentFilesMenu();
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

void MainWindow::on_actionNew_PAL_triggered()
{
    QString palFilePath = this->fileDialog(FILE_DIALOG_MODE::SAVE_CONF, "New Palette File", "PAL Files (*.pal *.PAL)");

    if (palFilePath.isEmpty()) {
        return;
    }

    QFileInfo palFileInfo(palFilePath);
    QString path = palFileInfo.absoluteFilePath();
    QString name = palFileInfo.fileName();

    if (palFileInfo.suffix().isEmpty()) {
        path += ".pal";
        name += ".pal";
    } else if (palFileInfo.suffix() != "pal") {
        QMessageBox::critical(nullptr, "Error", "Only .pal is supported for palettes.");
        return;
    }

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

    QFileInfo palFileInfo(palFilePath);
    QString path = palFileInfo.absoluteFilePath();
    QString name = palFileInfo.fileName();

    if (palFileInfo.suffix().isEmpty()) {
        path += ".pal";
        name += ".pal";
    } else if (palFileInfo.suffix() != "pal") {
        QMessageBox::critical(nullptr, "Error", "Only .pal is supported for palettes.");
        return;
    }

    if (!this->pal->save(palFilePath)) {
        QMessageBox::critical(this, "Error", "Could not save PAL file.");
        return;
    }

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

    if (trnFileInfo.suffix().isEmpty()) {
        path += ".trn";
        name += ".trn";
    } else if (trnFileInfo.suffix() != "trn") {
        QMessageBox::critical(nullptr, "Error", "Only .trn is supported for translations.");
        return;
    }

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

    QFileInfo trnFileInfo(trnFilePath);
    QString path = trnFileInfo.absoluteFilePath();
    QString name = trnFileInfo.fileName();

    if (trnFileInfo.suffix().isEmpty()) {
        path += ".trn";
        name += ".trn";
    } else if (trnFileInfo.suffix() != "trn") {
        QMessageBox::critical(nullptr, "Error", "Only .trn is supported for translations.");
        return;
    }

    if (!this->trn1->save(trnFilePath)) {
        QMessageBox::critical(this, "Error", "Could not save TRN file.");
        return;
    }

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

    if (trnFileInfo.suffix().isEmpty()) {
        path += ".trn";
        name += ".trn";
    } else if (trnFileInfo.suffix() != "trn") {
        QMessageBox::critical(nullptr, "Error", "Only .trn is supported for translations.");
        return;
    }

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

    QFileInfo trnFileInfo(trnFilePath);
    QString path = trnFileInfo.absoluteFilePath();
    QString name = trnFileInfo.fileName();

    if (trnFileInfo.suffix().isEmpty()) {
        path += ".trn";
        name += ".trn";
    } else if (trnFileInfo.suffix() != "trn") {
        QMessageBox::critical(nullptr, "Error", "Only .trn is supported for translations.");
        return;
    }

    if (!this->trn2->save(trnFilePath)) {
        QMessageBox::critical(this, "Error", "Could not save TRN file.");
        return;
    }

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

#if defined(Q_OS_WIN)
#define OS_TYPE "Windows"
#elif defined(Q_OS_QNX)
#define OS_TYPE "qnx"
#elif defined(Q_OS_ANDROID)
#define OS_TYPE "android"
#elif defined(Q_OS_IOS)
#define OS_TYPE "iOS"
#elif defined(Q_OS_TVOS)
#define OS_TYPE "tvOS"
#elif defined(Q_OS_WATCHOS)
#define OS_TYPE "watchOS"
#elif defined(Q_OS_MACOS)
#define OS_TYPE "macOS"
#elif defined(Q_OS_DARWIN)
#define OS_TYPE "darwin"
#elif defined(Q_OS_WASM)
#define OS_TYPE "wasm"
#elif defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
#define OS_TYPE "linux"
#else
#define OS_TYPE tr("unknown")
#endif

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About"), QStringLiteral("%1 %2 (%3) (%4-bit)").arg(D1_GRAPHICS_TOOL_TITLE).arg(D1_GRAPHICS_TOOL_VERSION).arg(OS_TYPE).arg(sizeof(void *) == 8 ? "64" : "32"));
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}
