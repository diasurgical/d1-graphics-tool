#include "mainwindow.h"

#include <QAction>
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
#include <QtWidgets>

#include "config/config.h"
#include "d1formats/d1cel.h"
#include "d1formats/d1celtileset.h"
#include "d1formats/d1cl2.h"
#include "ui_mainwindow.h"
#include "widgets/palettewidget.h"

MainWindow::MainWindow()
    : QMainWindow(nullptr)
    , ui(new Ui::MainWindow())
{
    // QCoreApplication::setAttribute( Qt::AA_EnableHighDpiScaling, true );

    this->lastFilePath = Config::value("LastFilePath").toString();

    ui->setupUi(this);

    this->setWindowTitle(D1_GRAPHICS_TOOL_TITLE);

    // initialize 'new' submenu of 'File'
    this->newMenu.addAction("Sprite", this, SLOT(actionNewSprite_triggered()));
    this->newMenu.addAction("Tileset", this, SLOT(actionNewTileset_triggered()));
    QAction *firstFileAction = (QAction *)this->ui->menuFile->actions()[0];
    this->ui->menuFile->insertMenu(firstFileAction, &this->newMenu);

    // Initialize 'Undo/Redo' of 'Edit
    this->undoStack = std::make_shared<UndoStack>();
    this->undoAction = new QAction("Undo", this);
    this->undoAction->setShortcuts(QKeySequence::Undo);
    this->ui->menuEdit->addAction(this->undoAction);
    this->redoAction = new QAction("Redo", this);
    this->redoAction->setShortcuts(QKeySequence::Redo);
    this->ui->menuEdit->addAction(this->redoAction);

    // Initialize widgets used for Undo/Redo
    this->m_progressDialog = std::make_unique<QProgressDialog>(this);
    this->m_progressDialog->close();
    this->m_progressDialog->setAutoReset(true);
    this->m_progressDialog->reset();
    this->m_progressDialog->setWindowModality(Qt::WindowModal);
    this->ui->menuEdit->addSeparator();

    this->ui->menuEdit->addAction(this->undoAction);
    this->ui->menuEdit->addAction(this->redoAction);
    this->ui->menuEdit->addSeparator();

    // Bind Undo/Redo actions to appropriate slots
    QObject::connect(this->undoAction, SIGNAL(triggered()), this, SLOT(actionUndo_triggered()));
    QObject::connect(this->redoAction, SIGNAL(triggered()), this, SLOT(actionRedo_triggered()));

    // Initialize 'Frame' submenu of 'Edit'
    this->frameMenu.setToolTipsVisible(true);
    this->frameMenu.addAction("Insert", this, SLOT(actionInsertFrame_triggered()))->setToolTip("Add new frames before the current one");
    this->frameMenu.addAction("Append", this, SLOT(actionAddFrame_triggered()))->setToolTip("Add new frames at the end");
    this->frameMenu.addAction("Replace", this, SLOT(actionReplaceFrame_triggered()))->setToolTip("Replace the current frame");
    this->frameMenu.addAction("Delete", this, SLOT(actionDelFrame_triggered()))->setToolTip("Delete the current frame");
    this->ui->menuEdit->addMenu(&this->frameMenu);

    // Initialize 'Subtile' submenu of 'Edit'
    this->subtileMenu.setToolTipsVisible(true);
    this->subtileMenu.addAction("Create", this, SLOT(actionCreateSubtile_triggered()))->setToolTip("Create a new tile");
    this->subtileMenu.addAction("Insert", this, SLOT(actionInsertSubtile_triggered()))->setToolTip("Add new tiles before the current one");
    this->subtileMenu.addAction("Append", this, SLOT(actionAddSubtile_triggered()))->setToolTip("Add new tiles at the end");
    this->subtileMenu.addAction("Clone", this, SLOT(actionCloneSubtile_triggered()))->setToolTip("Add new tiles at the end based on the current one");
    this->subtileMenu.addAction("Replace", this, SLOT(actionReplaceSubtile_triggered()))->setToolTip("Replace the current tile");
    this->subtileMenu.addAction("Delete", this, SLOT(actionDelSubtile_triggered()))->setToolTip("Delete the current tile");
    this->ui->menuEdit->addMenu(&this->subtileMenu);

    // Initialize 'Tile' submenu of 'Edit'
    this->tileMenu.setToolTipsVisible(true);
    this->tileMenu.addAction("Create", this, SLOT(actionCreateTile_triggered()))->setToolTip("Create a new megatile");
    this->tileMenu.addAction("Insert", this, SLOT(actionInsertTile_triggered()))->setToolTip("Add new megatiles before the current one");
    this->tileMenu.addAction("Append", this, SLOT(actionAddTile_triggered()))->setToolTip("Add new megatiles at the end");
    this->tileMenu.addAction("Clone", this, SLOT(actionCloneTile_triggered()))->setToolTip("Add new megatile at the end based on the current one");
    this->tileMenu.addAction("Replace", this, SLOT(actionReplaceTile_triggered()))->setToolTip("Replace the current megatile");
    this->tileMenu.addAction("Delete", this, SLOT(actionDelTile_triggered()))->setToolTip("Delete the current megatile");
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
    delete this->undoAction;
    delete this->redoAction;
}

void MainWindow::setPal(const QString &path)
{
    this->m_palWidget->setPal(path);
    this->m_trnUniqueWidget->setTrnPalette(m_palWidget->pal());
    this->m_trnWidget->refresh();
}

void MainWindow::setTrnUnique(const QString &path)
{
    this->m_trnUniqueWidget->setTrn(path);
    this->m_trnUniqueWidget->setTrnPalette(m_palWidget->pal());
    this->m_trnWidget->setTrnPalette(this->m_trnUniqueWidget->trn()->getResultingPalette());
    this->m_trnWidget->refresh();
}

void MainWindow::setTrn(const QString &path)
{
    this->m_trnWidget->setTrn(path);
    this->m_trnWidget->setTrnPalette(this->m_trnUniqueWidget->trn()->getResultingPalette());
    this->m_trnWidget->refresh();

    this->gfx->setPalette(this->m_trnWidget->trn()->getResultingPalette());
}

QString MainWindow::getLastFilePath()
{
    return this->lastFilePath;
}

void MainWindow::updateWindow()
{
    // rebuild palette hits
    this->palHits->update();
    this->m_palWidget->refresh();
    this->undoAction->setEnabled(this->undoStack->canUndo());
    this->redoAction->setEnabled(this->undoStack->canRedo());

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

void MainWindow::paletteWidget_callback(PaletteWidget *widget, PWIDGET_CALLBACK_TYPE type)
{
    switch (type) {
    case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_NEW:
        widget->newOrSaveAsFile(PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_NEW);
        break;
    case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_OPEN:
        widget->openPalette();
        break;
    case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_SAVE:
        widget->save();
        break;
    case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_SAVEAS:
        widget->newOrSaveAsFile(PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_SAVEAS);
        break;
    case PWIDGET_CALLBACK_TYPE::PWIDGET_CALLBACK_CLOSE:
        widget->closePalette();
        break;
    }
}

void MainWindow::resetPaletteCycle()
{
    // this->pal->resetColors();
    this->m_palWidget->modify();
}

void MainWindow::nextPaletteCycle(D1PAL_CYCLE_TYPE type)
{
    // this->pal->cycleColors(type);
    this->m_palWidget->modify();
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

void MainWindow::actionNewSprite_triggered()
{
    OpenAsParam params;
    params.isTileset = OPEN_TILESET_TYPE::No;
    this->openFile(params);
}

void MainWindow::actionNewTileset_triggered()
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

void MainWindow::on_actionImport_triggered()
{
    this->importDialog.initialize();
    this->importDialog.show();
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

    // Loading default null.trn
    D1Trn *newUniqTrn = new D1Trn(newPal);
    newUniqTrn->load(D1Trn::IDENTITY_PATH);

    D1Trn *newTrn = new D1Trn(newUniqTrn->getResultingPalette());
    newTrn->load(D1Trn::IDENTITY_PATH);

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
    this->gfx->setPalette(newTrn->getResultingPalette());
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
    this->m_palWidget = new PaletteWidget(this->undoStack, "Palette");
    this->m_trnWidget = new PaletteWidget(this->undoStack, "Translation");
    this->m_trnUniqueWidget = new PaletteWidget(this->undoStack, "Unique translation");
    this->ui->palFrame->layout()->addWidget(this->m_palWidget);
    this->ui->palFrame->layout()->addWidget(this->m_trnWidget);
    this->ui->palFrame->layout()->addWidget(this->m_trnUniqueWidget);

    // Configuration update triggers refresh of the palette widgets
    QObject::connect(&this->settingsDialog, &SettingsDialog::configurationSaved, this->m_palWidget, &PaletteWidget::reloadConfig);
    QObject::connect(&this->settingsDialog, &SettingsDialog::configurationSaved, this->m_trnUniqueWidget, &PaletteWidget::reloadConfig);
    QObject::connect(&this->settingsDialog, &SettingsDialog::configurationSaved, this->m_trnWidget, &PaletteWidget::reloadConfig);
    QObject::connect(&this->settingsDialog, &SettingsDialog::configurationSaved, this->m_palWidget, &PaletteWidget::refresh);

    // Palette and translation file selection
    // When a .pal or .trn file is selected in the PaletteWidget update the pal or trn
    QObject::connect(this->m_palWidget, &PaletteWidget::pathSelected, this, &MainWindow::setPal);
    QObject::connect(this->m_trnUniqueWidget, &PaletteWidget::pathSelected, this, &MainWindow::setTrnUnique);
    QObject::connect(this->m_trnWidget, &PaletteWidget::pathSelected, this, &MainWindow::setTrn);

    // Refresh PAL/TRN view chain
    QObject::connect(this->m_palWidget, &PaletteWidget::refreshed, this->m_trnUniqueWidget, &PaletteWidget::refresh);
    QObject::connect(this->m_trnUniqueWidget, &PaletteWidget::refreshed, this->m_trnWidget, &PaletteWidget::refresh);

    // Translation color selection
    QObject::connect(this->m_palWidget, &PaletteWidget::colorsSelected, this->m_trnWidget, &PaletteWidget::checkTranslationsSelection);
    QObject::connect(this->m_trnWidget, &PaletteWidget::colorsSelected, this->m_trnUniqueWidget, &PaletteWidget::checkTranslationsSelection);
    QObject::connect(this->m_trnWidget, &PaletteWidget::displayAllRootColors, this->m_palWidget, &PaletteWidget::temporarilyDisplayAllColors);
    QObject::connect(this->m_trnUniqueWidget, &PaletteWidget::displayAllRootColors, this->m_trnWidget, &PaletteWidget::temporarilyDisplayAllColors);
    QObject::connect(this->m_trnWidget, &PaletteWidget::displayRootInformation, this->m_palWidget, &PaletteWidget::displayInfo);
    QObject::connect(this->m_trnUniqueWidget, &PaletteWidget::displayRootInformation, this->m_trnWidget, &PaletteWidget::displayInfo);
    QObject::connect(this->m_trnWidget, &PaletteWidget::displayRootBorder, this->m_palWidget, &PaletteWidget::displayBorder);
    QObject::connect(this->m_trnUniqueWidget, &PaletteWidget::displayRootBorder, this->m_trnWidget, &PaletteWidget::displayBorder);
    QObject::connect(this->m_trnWidget, &PaletteWidget::clearRootInformation, this->m_palWidget, &PaletteWidget::clearInfo);
    QObject::connect(this->m_trnUniqueWidget, &PaletteWidget::clearRootInformation, this->m_trnWidget, &PaletteWidget::clearInfo);
    QObject::connect(this->m_trnWidget, &PaletteWidget::clearRootBorder, this->m_palWidget, &PaletteWidget::clearBorder);
    QObject::connect(this->m_trnUniqueWidget, &PaletteWidget::clearRootBorder, this->m_trnWidget, &PaletteWidget::clearBorder);

    if (isTileset) {
        this->levelCelView = new LevelCelView(this->undoStack, this);
        this->levelCelView->initialize(this->gfx, this->min, this->til, this->sol, this->amp);

        // Refresh CEL view if a PAL or TRN is modified
        QObject::connect(this->m_palWidget, &PaletteWidget::modified, this->levelCelView, &LevelCelView::displayFrame);
        QObject::connect(this->m_trnUniqueWidget, &PaletteWidget::modified, this->levelCelView, &LevelCelView::displayFrame);
        QObject::connect(this->m_trnWidget, &PaletteWidget::modified, this->levelCelView, &LevelCelView::displayFrame);

        // Select color when level CEL view clicked
        QObject::connect(this->levelCelView, &LevelCelView::colorIndexClicked, this->m_palWidget, &PaletteWidget::selectColor);
        QObject::connect(this->levelCelView, &LevelCelView::colorIndexClicked, this->m_trnUniqueWidget, &PaletteWidget::selectColor);
        QObject::connect(this->levelCelView, &LevelCelView::colorIndexClicked, this->m_trnWidget, &PaletteWidget::selectColor);

        // Refresh palette widgets when frame, subtile of tile is changed
        QObject::connect(this->levelCelView, &LevelCelView::frameRefreshed, this->m_palWidget, &PaletteWidget::refresh);

        // Initialize palette widgets
        this->palHits = new D1PalHits(this->gfx, this->min, this->til);
        this->m_palWidget->initialize(newPal, this->levelCelView, this->palHits, PaletteType::Palette);
        this->m_trnUniqueWidget->initialize(newPal, newUniqTrn, this->levelCelView, this->palHits, PaletteType::UniqTranslation);
        this->m_trnWidget->initialize(newUniqTrn->getResultingPalette(), newTrn, this->levelCelView, this->palHits, PaletteType::Translation);

        this->levelCelView->displayFrame();
    }
    // Otherwise build a CelView
    else {
        this->celView = new CelView(this->undoStack, this);
        this->celView->initialize(this->gfx);

        // Refresh CEL view if a PAL or TRN is modified
        QObject::connect(this->m_palWidget, &PaletteWidget::modified, this->celView, &CelView::displayFrame);
        QObject::connect(this->m_trnUniqueWidget, &PaletteWidget::modified, this->celView, &CelView::displayFrame);
        QObject::connect(this->m_trnWidget, &PaletteWidget::modified, this->celView, &CelView::displayFrame);

        // Select color when CEL view clicked
        QObject::connect(this->celView, &CelView::colorIndexClicked, this->m_palWidget, &PaletteWidget::selectColor);
        QObject::connect(this->celView, &CelView::colorIndexClicked, this->m_trnUniqueWidget, &PaletteWidget::selectColor);
        QObject::connect(this->celView, &CelView::colorIndexClicked, this->m_trnWidget, &PaletteWidget::selectColor);

        // Refresh palette widgets when frame
        QObject::connect(this->celView, &CelView::frameRefreshed, this->m_palWidget, &PaletteWidget::refresh);

        // Initialize palette widgets
        this->palHits = new D1PalHits(this->gfx);
        this->m_palWidget->initialize(newPal, this->celView, this->palHits, PaletteType::Palette);
        this->m_trnUniqueWidget->initialize(newPal, newUniqTrn, this->celView, this->palHits, PaletteType::UniqTranslation);
        this->m_trnWidget->initialize(newUniqTrn->getResultingPalette(), newTrn, this->celView, this->palHits, PaletteType::Translation);

        this->celView->displayFrame();
    }

    // Look for all palettes in the same folder as the CEL/CL2 file
    QDirIterator it(celFileInfo.absolutePath(), QStringList() << "*.pal", QDir::Files);
    QString firstPaletteFound = QString();
    while (it.hasNext()) {
        QString sPath = it.next();

        if (m_palWidget->loadPalette(sPath) && firstPaletteFound.isEmpty()) {
            firstPaletteFound = sPath;
        }
    }
    // Select the first palette found in the same folder as the CEL/CL2 if it exists
    if (!firstPaletteFound.isEmpty())
        this->m_palWidget->selectPath(firstPaletteFound);

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

void MainWindow::openPalFiles(const QStringList &filePaths, PaletteWidget *widget) const
{
    QString firstFound;

    this->ui->statusBar->showMessage("Reading...");
    this->ui->statusBar->repaint();

    for (const QString &path : filePaths) {
        if (widget->loadPalette(path) && firstFound.isEmpty()) {
            firstFound = path;
        }
    }

    if (!firstFound.isEmpty()) {
        widget->selectPath(firstFound);
    }

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::openFontFile(QString filePath, QColor renderColor, int pointSize, uint symbolPrefix)
{
    OpenAsParam params;
    params.isTileset = OPEN_TILESET_TYPE::No;
    this->openFile(params);

    int fontId = QFontDatabase::addApplicationFont(filePath);
    if (fontId == -1) {
        QMessageBox::critical(this, "Error", "Font could not be loaded.");
        return;
    }

    QStringList families = QFontDatabase::applicationFontFamilies(fontId);
    if (families.size() == 0) {
        QMessageBox::critical(this, "Error", "No font families loaded.");
        QFontDatabase::removeApplicationFont(fontId);
        return;
    }

    QFont font = QFont(families[0], pointSize);
    QFontMetrics metrics = QFontMetrics(font);

    uint symbolCount = 1 << 8;
    for (uint i = 0; i < symbolCount; i++) {
        char32_t codePoint = static_cast<char32_t>(symbolPrefix | i);
        QString text = QString::fromUcs4(&codePoint, 1);
        QSize renderSize = metrics.size(0, text);
        QImage image = QImage(renderSize, QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        QPainter painter = QPainter(&image);
        painter.setPen(renderColor);
        painter.setFont(font);
        painter.drawText(0, metrics.ascent(), text);

        this->gfx->insertFrame(i, image);
    }

    this->celView->initialize(this->gfx);
    this->celView->displayFrame();
    QFontDatabase::removeApplicationFont(fontId);
}

void MainWindow::updateStatusBar(const QString &status, const QString &styleSheet)
{
    this->ui->statusBar->setStyleSheet(styleSheet);
    this->ui->statusBar->showMessage(status);
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

namespace mw { // TODO: move that function in some better place later on, as both PaletteWidget and MW use it

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

} // namespace mw

bool MainWindow::isOkToQuit()
{
    if ((m_palWidget != nullptr) && (m_trnWidget != nullptr) && (m_trnUniqueWidget != nullptr)) {
        if (!m_palWidget->isOkToQuit() || !m_trnWidget->isOkToQuit() || !m_trnUniqueWidget->isOkToQuit())
            return false;
    }

    if (this->min != nullptr && !mw::QuestionDiscardChanges(this->min->isModified(), this->min->getFilePath())) {
        return false;
    }
    if (this->til != nullptr && !mw::QuestionDiscardChanges(this->til->isModified(), this->til->getFilePath())) {
        return false;
    }
    if (this->sol != nullptr && !mw::QuestionDiscardChanges(this->sol->isModified(), this->sol->getFilePath())) {
        return false;
    }
    if (this->amp != nullptr && !mw::QuestionDiscardChanges(this->amp->isModified(), this->amp->getFilePath())) {
        return false;
    }
    if (this->gfx != nullptr && !mw::QuestionDiscardChanges(this->gfx->isModified(), this->gfx->getFilePath())) {
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
    delete this->m_palWidget;
    delete this->m_trnUniqueWidget;
    delete this->m_trnWidget;

    this->m_palWidget = nullptr;
    this->m_trnUniqueWidget = nullptr;
    this->m_trnWidget = nullptr;

    delete this->gfx;

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

void MainWindow::actionUndo_triggered()
{
    this->undoStack->undo();
    this->undoAction->setEnabled(this->undoStack->canUndo());
    this->redoAction->setEnabled(this->undoStack->canRedo());
}

void MainWindow::actionRedo_triggered()
{
    this->undoStack->redo();
    this->undoAction->setEnabled(this->undoStack->canUndo());
    this->redoAction->setEnabled(this->undoStack->canRedo());
}

void MainWindow::actionInsertFrame_triggered()
{
    this->addFrames(false);
}

void MainWindow::actionAddFrame_triggered()
{
    this->addFrames(true);
}

void MainWindow::actionReplaceFrame_triggered()
{
    QString filter = imageNameFilter();
    QString imgFilePath = this->fileDialog(FILE_DIALOG_MODE::OPEN, "Replacement Image File", filter.toLatin1().data());

    if (imgFilePath.isEmpty()) {
        return;
    }

    this->ui->statusBar->showMessage("Reading...");
    this->ui->statusBar->repaint();

    if (this->celView != nullptr) {
        this->celView->sendReplaceCurrentFrameCmd(imgFilePath);
    }
    if (this->levelCelView != nullptr) {
        this->levelCelView->sendReplaceCurrentFrameCmd(imgFilePath);
    }
    this->updateWindow();

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::actionDelFrame_triggered()
{
    if (this->celView != nullptr) {
        this->celView->sendRemoveFrameCmd();
    }
    if (this->levelCelView != nullptr) {
        this->levelCelView->sendRemoveFrameCmd();
    }
    this->updateWindow();
}

void MainWindow::actionCreateSubtile_triggered()
{
    this->levelCelView->createSubtile();
    this->updateWindow();
}

void MainWindow::actionCloneSubtile_triggered()
{
    this->levelCelView->cloneSubtile();
    this->updateWindow();
}

void MainWindow::actionInsertSubtile_triggered()
{
    this->addSubtiles(false);
}

void MainWindow::actionAddSubtile_triggered()
{
    this->addSubtiles(true);
}

void MainWindow::actionReplaceSubtile_triggered()
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

void MainWindow::actionDelSubtile_triggered()
{
    this->levelCelView->removeCurrentSubtile();
    this->updateWindow();
}

void MainWindow::actionCreateTile_triggered()
{
    this->levelCelView->createTile();
    this->updateWindow();
}

void MainWindow::actionCloneTile_triggered()
{
    this->levelCelView->cloneTile();
    this->updateWindow();
}

void MainWindow::actionInsertTile_triggered()
{
    this->addTiles(false);
}

void MainWindow::actionAddTile_triggered()
{
    this->addTiles(true);
}

void MainWindow::actionReplaceTile_triggered()
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

void MainWindow::actionDelTile_triggered()
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

void MainWindow::setupUndoMacroWidget(std::unique_ptr<UserData> &userData, enum OperationType opType)
{
    this->m_currMacroOpType = opType;
    int prevMaximum = this->m_progressDialog->maximum();

    this->m_progressDialog->reset();
    this->m_progressDialog->show();
    this->m_progressDialog->setLabelText(userData->labelText());
    this->m_progressDialog->setCancelButtonText(userData->cancelButtonText());
    this->m_progressDialog->setMinimum(userData->min());
    this->m_progressDialog->setMaximum(userData->max());

    if (opType == OperationType::Undo && prevMaximum == userData->max())
        this->m_currProgDialogPos = this->m_progressDialog->maximum() - m_currProgDialogPos;
    else if (this->m_currProgDialogPos == prevMaximum || prevMaximum != userData->max())
        this->m_currProgDialogPos = 0;
}

void MainWindow::updateUndoMacroWidget(bool &result)
{
    this->m_currProgDialogPos++;

    this->m_progressDialog->setValue(m_currProgDialogPos);
    if (this->m_progressDialog->wasCanceled()) {
        this->m_progressDialog->reset();
        result = true;
        return;
    }

    QCoreApplication::processEvents();
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
