#include "mainwindow.h"

#include <QDragEnterEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QMimeData>
#include <QTextStream>
#include <QTime>
#include <QUndoCommand>
#include <QUndoStack>

#include "d1cel.h"
#include "d1celtileset.h"
#include "d1cl2.h"
#include "d1clx.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // QCoreApplication::setAttribute( Qt::AA_EnableHighDpiScaling, true );

    this->loadConfiguration();
    this->lastFilePath = this->configuration->value("LastFilePath").toString();

    ui->setupUi(this);

    // Initialize undo/redo
    this->undoStack = new QUndoStack(this);
    this->undoAction = undoStack->createUndoAction(this, "Undo");
    this->undoAction->setShortcuts(QKeySequence::Undo);
    this->redoAction = undoStack->createRedoAction(this, "Redo");
    this->redoAction->setShortcuts(QKeySequence::Redo);
    this->ui->menuEdit->addAction(this->undoAction);
    this->ui->menuEdit->addAction(this->redoAction);

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
    delete settingsDialog;
    delete exportDialog;
    delete this->undoStack;
    delete this->undoAction;
    delete this->redoAction;
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

    this->cel->setPalette(this->trn2->getResultingPalette());

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

void MainWindow::pushCommandToUndoStack(QUndoCommand *cmd)
{
    this->undoStack->push(cmd);
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

QString MainWindow::fileDialog(bool save, const char *title, const char *filter)
{
    QString filePath;

    if (!this->lastFilePath.isEmpty()) {
        QFileInfo fi(this->lastFilePath);
        filePath = fi.absolutePath();
    }
    if (save) {
        filePath = QFileDialog::getSaveFileName(this, title, filePath, filter);
    } else {
        filePath = QFileDialog::getOpenFileName(this, title, filePath, filter);
    }

    if (!filePath.isEmpty()) {
        this->lastFilePath = filePath;
    }
    return filePath;
}

void MainWindow::on_actionOpen_triggered()
{
    QString openFilePath = this->fileDialog(false, "Open Graphics", "CEL/CL2/CLX Files (*.cel *.CEL *.cl2 *.CL2 *.clx *.CLX)");

    if (!openFilePath.isEmpty()) {
        this->openFile(openFilePath);
    }

    // QMessageBox::information( this, "Debug", celFilePath );
    // QMessageBox::information( this, "Debug", QString::number(cel->getFrameCount()));

    // QTime timer = QTime();
    // timer.start();
    // QMessageBox::information( this, "time", QString::number(timer.elapsed()) );
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasFormat("text/uri-list"))
        return;

    event->acceptProposedAction();

    for (const QUrl &url : event->mimeData()->urls())
        this->openFile(url.toLocalFile());
}

void MainWindow::openFile(QString openFilePath, OpenAsParam *params)
{
    // Check file extension
    if (params == nullptr && !openFilePath.toLower().endsWith(".cel")
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
    QString solFilePath = basePath + ".sol";
    QString minFilePath = basePath + ".min";
    QString tilFilePath = basePath + ".til";
    bool isTileset = params == nullptr && QFileInfo::exists(solFilePath) && QFileInfo::exists(minFilePath) && QFileInfo::exists(tilFilePath);

    QString extension = celFileInfo.suffix();
    if (QString::compare(extension, "cel", Qt::CaseInsensitive) == 0) {
        if (isTileset) {
            // Loading SOL
            this->sol = new D1Sol();
            this->sol->load(solFilePath);

            // Loading MIN
            this->min = new D1Min();
            if (!this->min->load(minFilePath, this->sol->getSubtileCount())) {
                QMessageBox::critical(this, "Error", "Failed loading MIN file: " + minFilePath);
                return;
            }
            this->cel = new D1CelTileset(this->min);
            this->min->setCel(this->cel);

            // Loading TIL
            this->til = new D1Til();
            if (!this->til->load(tilFilePath)) {
                QMessageBox::critical(this, "Error", "Failed loading TIL file: " + tilFilePath);
                return;
            }
            this->til->setMin(this->min);

            // Loading AMP
            this->amp = new D1Amp();
            QString ampFilePath = basePath + ".amp";
            this->amp->load(ampFilePath, this->min->getSubtileCount());
        } else {
            this->cel = new D1Cel();
        }
    } else if (QString::compare(extension, "cl2", Qt::CaseInsensitive) == 0) {
        this->cel = new D1Cl2();
    } else if (QString::compare(extension, "clx", Qt::CaseInsensitive) == 0) {
        this->cel = new D1Clx();
    }

    if (!this->cel->load(openFilePath, params)) {
        QMessageBox::critical(this, "Error", "Could not open " + extension.toUpper() + " file.");
        return;
    }

    this->cel->setPalette(this->trn2->getResultingPalette());

    // Add palette widgets for PAL and TRNs
    this->palWidget = new PaletteWidget(this->configuration, nullptr, "Palette");
    this->trn2Widget = new PaletteWidget(this->configuration, nullptr, "Translation");
    this->trn1Widget = new PaletteWidget(this->configuration, nullptr, "Unique translation");
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

    // Send editing actions to the undo/redo stack
    QObject::connect(this->palWidget, &PaletteWidget::sendEditingCommand, this, &MainWindow::pushCommandToUndoStack);
    QObject::connect(this->trn1Widget, &PaletteWidget::sendEditingCommand, this, &MainWindow::pushCommandToUndoStack);
    QObject::connect(this->trn2Widget, &PaletteWidget::sendEditingCommand, this, &MainWindow::pushCommandToUndoStack);

    if (isTileset) {
        this->levelCelView = new LevelCelView();
        this->levelCelView->initialize(this->cel, this->min, this->til, this->sol, this->amp);

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
        this->palHits = new D1PalHits(this->cel, this->min, this->til, this->sol);
        this->palWidget->initialize(this->pal, this->levelCelView, this->palHits);
        this->trn1Widget->initialize(this->pal, this->trn1, this->levelCelView, this->palHits);
        this->trn2Widget->initialize(this->trn1->getResultingPalette(), this->trn2, this->levelCelView, this->palHits);

        this->levelCelView->displayFrame();
    }
    // Otherwise build a CelView
    else {
        this->celView = new CelView();
        this->celView->initialize(this->cel);

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
        this->palHits = new D1PalHits(this->cel);
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

        if (sPath != "1") {
            QFileInfo palFileInfo(sPath);
            QString path = palFileInfo.absoluteFilePath();
            QString name = palFileInfo.fileName();
            this->pals[path] = new D1Pal();

            if (!this->pals[path]->load(path)) {
                delete this->pals[path];
                this->pals.remove(path);
                QMessageBox::warning(this, "Warning", "Could not load PAL file.");
                continue;
            }

            this->palWidget->addPath(path, name);

            if (firstPaletteFound.isEmpty())
                firstPaletteFound = path;
        }
    }
    // Select the first palette found in the same folder as the CEL/CL2 if it exists
    if (!firstPaletteFound.isEmpty())
        this->palWidget->selectPath(firstPaletteFound);

    // Adding the CelView to the main frame
    if (this->celView != nullptr)
        this->ui->mainFrame->layout()->addWidget(this->celView);
    else
        this->ui->mainFrame->layout()->addWidget(this->levelCelView);

    // Adding the PalView to the pal frame
    // this->ui->palFrame->layout()->addWidget( this->palView );

    // update available menu entries
    this->ui->menuEdit->setEnabled(true);
    this->ui->menuPalette->setEnabled(true);
    this->ui->actionExport->setEnabled(true);
    this->ui->actionClose->setEnabled(true);

    // Clear loading message from status bar
    this->ui->statusBar->clearMessage();
}

void MainWindow::on_actionOpenAs_triggered()
{
    this->openAsDialog->initialize(this->configuration);
    this->openAsDialog->show();
}

void MainWindow::on_actionClose_triggered()
{
    this->undoStack->clear();

    delete this->celView;
    delete this->levelCelView;
    delete this->palWidget;
    delete this->trn1Widget;
    delete this->trn2Widget;
    delete this->cel;

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
    this->ui->menuPalette->setEnabled(false);
    this->ui->actionExport->setEnabled(false);
    this->ui->actionClose->setEnabled(false);
}

void MainWindow::on_actionSettings_triggered()
{
    this->settingsDialog->initialize(this->configuration);
    this->settingsDialog->show();
}

void MainWindow::on_actionExport_triggered()
{
    if (this->min != nullptr)
        this->exportDialog->setMin(this->min);

    if (this->til != nullptr)
        this->exportDialog->setTil(this->til);

    if (this->cel != nullptr)
        this->exportDialog->setCel(this->cel);

    if (this->amp != nullptr)
        this->exportDialog->setAmp(this->amp);

    if (this->sol != nullptr)
        this->exportDialog->setSol(this->sol);

    this->exportDialog->show();
}

void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
}

void MainWindow::on_actionNew_PAL_triggered()
{
    QString palFilePath = this->fileDialog(true, "New Palette File", "PAL Files (*.pal *.PAL)");

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
    QString palFilePath = this->fileDialog(false, "Load Palette File", "PAL Files (*.pal *.PAL)");

    if (palFilePath.isEmpty()) {
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
    QString palFilePath = this->fileDialog(true, "Save Palette File as...", "PAL Files (*.pal *.PAL)");

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
    QString trnFilePath = this->fileDialog(true, "New Translation File", "TRN Files (*.trn *.TRN)");

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
    QString trnFilePath = this->fileDialog(false, "Load Translation File", "TRN Files (*.trn *.TRN)");

    if (trnFilePath.isEmpty()) {
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
    QString trnFilePath = this->fileDialog(true, "Save Translation File as...", "TRN Files (*.trn *.TRN)");

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
    QString trnFilePath = this->fileDialog(true, "New Translation File", "TRN Files (*.trn *.TRN)");

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
    QString trnFilePath = this->fileDialog(false, "Load Translation File", "TRN Files (*.trn *.TRN)");

    if (trnFilePath.isEmpty()) {
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

    if (this->trn2s.contains(path))
        delete this->trn2s[path];
    this->trn2s[path] = newTrn;
    this->trn2Widget->addPath(path, name);
    this->trn2Widget->selectPath(path);
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
    QString trnFilePath = this->fileDialog(true, "Save Translation File as...", "TRN Files (*.trn *.TRN)");

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
