#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTime>

MainWindow::MainWindow( QWidget *parent ) :
    QMainWindow( parent ),
    ui( new Ui::MainWindow ),
    configuration( new QJsonObject ),
    settingsDialog( new SettingsDialog(this) ),
    exportDialog( new ExportDialog(this) ),
    batchExportDialog( new BatchExportDialog(this) ),
    pal( new D1Pal ),
    trn1( new D1Trn ),
    trn2( new D1Trn ),
    cel( new D1Cel ),
    min( new D1Min ),
    til( new D1Til )
{
    //QCoreApplication::setAttribute( Qt::AA_EnableHighDpiScaling, true );

    this->loadConfiguration();

    ui->setupUi( this );

    // Initialize undo/redo
    this->undoStack = new QUndoStack( this );
    this->undoAction = undoStack->createUndoAction( this, "Undo" );
    this->undoAction->setShortcuts( QKeySequence::Undo );
    this->redoAction = undoStack->createRedoAction( this, "Redo" );
    this->redoAction->setShortcuts( QKeySequence::Redo );
    this->ui->menuEdit->addAction( this->undoAction );
    this->ui->menuEdit->addAction( this->redoAction );

    this->ui->menuPalette->setEnabled( false );
}

MainWindow::~MainWindow()
{
    delete ui;

    if( this->undoStack )
        delete this->undoStack;

    if( this->undoAction )
        delete this->undoAction;

    if( this->redoAction )
        delete this->redoAction;

    if( this->pal )
        delete this->pal;

    if( this->trn1 )
        delete this->trn1;

    if( this->trn2 )
        delete this->trn2;

    if( this->cel )
        delete this->cel;

    if( this->min )
        delete this->min;

    if( this->til )
        delete this->til;

    if( this->palHits )
        delete this->palHits;
}

void MainWindow::setPal( QString path )
{
    this->pal = this->pals[path];
    this->trn1->setPalette( this->pal );
    this->trn1->refreshResultingPalette();
    this->trn2->refreshResultingPalette();

    this->palWidget->setPal( this->pal );
}

void MainWindow::setTrn1( QString path )
{
    this->trn1 = this->trn1s[path];
    this->trn1->setPalette( this->pal );
    this->trn1->refreshResultingPalette();
    this->trn2->setPalette( this->trn1->getResultingPalette() );
    this->trn2->refreshResultingPalette();

    this->trn1Widget->setTrn( this->trn1 );
}

void MainWindow::setTrn2( QString path )
{
    this->trn2 = this->trn2s[path];
    this->trn2->setPalette( this->trn1->getResultingPalette() );
    this->trn2->refreshResultingPalette();

    this->cel->setPalette( this->trn2->getResultingPalette() );

    this->trn2Widget->setTrn( this->trn2 );
}

void MainWindow::loadConfiguration()
{
    QString jsonFilePath = QCoreApplication::applicationDirPath() + "/D1GraphicsTool.config.json";
    bool configurationModified = false;

    // If configuration file exists load it otherwise create it
    if( QFile::exists(jsonFilePath) )
    {
        QFile loadJson(jsonFilePath);
        loadJson.open( QIODevice::ReadOnly );
        QJsonDocument loadJsonDoc = QJsonDocument::fromJson( loadJson.readAll() );
        this->configuration = new QJsonObject( loadJsonDoc.object() );
        loadJson.close();

        if( !this->configuration->contains("PaletteDefaultColor") )
        {
            this->configuration->insert( "PaletteDefaultColor", "#FF00FF" );
            configurationModified = true;
        }
        if( !this->configuration->contains("PaletteSelectionBorderColor") )
        {
            this->configuration->insert( "PaletteSelectionBorderColor", "#FF0000" );
            configurationModified = true;
        }
    }
    else
    {
        this->configuration->insert( "WorkingDirectory",QCoreApplication::applicationDirPath() );
        this->configuration->insert( "PaletteDefaultColor", "#FF00FF" );
        this->configuration->insert( "PaletteSelectionBorderColor", "#FF0000" );
        configurationModified = true;
    }

    if( configurationModified )
    {
        QFile saveJson( jsonFilePath );
        saveJson.open( QIODevice::WriteOnly );
        QJsonDocument saveDoc( *this->configuration );
        saveJson.write( saveDoc.toJson() );
        saveJson.close();
    }
}

void MainWindow::pushCommandToUndoStack( QUndoCommand *cmd )
{
    this->undoStack->push( cmd );
}

void MainWindow::on_actionOpen_triggered()
{
    QString errorMessage;
    bool minTilFound = false;
    QString celDirectory;
    QString celFileName;
    QString minFilePath;
    QString tilFilePath;

    QString openFilePath = QFileDialog::getOpenFileName(
        this, "Open Graphics", this->configuration->value("WorkingDirectory").toString(),
        "CEL/CL2 Files (*.cel *.cl2);;PCX Files (*.pcx);;GIF Files (*.gif)" );

    if( !openFilePath.isEmpty() )
    {
        // Check file extension
        if( openFilePath.toLower().endsWith( ".cel" ) || openFilePath.toLower().endsWith( ".cl2" ) )
        {
            this->on_actionClose_triggered();

            this->ui->statusBar->showMessage("Loading...");
            this->ui->statusBar->repaint();

            // Loading default.pal
            this->pals[":/default.pal"] = new D1Pal(":/default.pal");
            this->pal = this->pals[":/default.pal"];

            // Loading default null.trn
            this->trn1s[":/null.trn"] = new D1Trn(":/null.trn", this->pal );
            this->trn1 = this->trn1s[":/null.trn"];
            this->trn2s[":/null.trn"] = new D1Trn(":/null.trn", this->trn1->getResultingPalette() );
            this->trn2 = this->trn2s[":/null.trn"];


            if( openFilePath.toLower().endsWith( ".cel" ) )
            {
                this->cel = new D1Cel;
                errorMessage = "Could not open CEL file.";
            }
            else
            {
                this->cel = new D1Cl2;
                errorMessage = "Could not open CL2 file.";
            }

            if( !this->cel->load( openFilePath ) )
            {
                QMessageBox::critical( this, "Error", errorMessage );
                return;
            }

            this->cel->setPalette( this->trn2->getResultingPalette() );

            // Add palette widgets for PAL and TRNs
            this->palWidget = new PaletteWidget( this->configuration, nullptr, "Palette" );
            this->trn2Widget = new PaletteWidget( this->configuration, nullptr, "Translation" );
            this->trn1Widget = new PaletteWidget( this->configuration, nullptr, "Unique translation" );
            this->ui->palFrame->layout()->addWidget( this->palWidget );
            this->ui->palFrame->layout()->addWidget( this->trn2Widget );
            this->ui->palFrame->layout()->addWidget( this->trn1Widget );

            // Configuration update triggers refresh of the palette widgets
            QObject::connect( this->settingsDialog, &SettingsDialog::configurationSaved, this->palWidget, &PaletteWidget::reloadConfig );
            QObject::connect( this->settingsDialog, &SettingsDialog::configurationSaved, this->trn1Widget, &PaletteWidget::reloadConfig );
            QObject::connect( this->settingsDialog, &SettingsDialog::configurationSaved, this->trn2Widget, &PaletteWidget::reloadConfig );
            QObject::connect( this->settingsDialog, &SettingsDialog::configurationSaved, this->palWidget, &PaletteWidget::refresh );

            // Palette and translation file selection
            // When a .pal or .trn file is selected in the PaletteWidget update the pal or trn
            QObject::connect( this->palWidget, &PaletteWidget::pathSelected, this, &MainWindow::setPal );
            QObject::connect( this->trn1Widget, &PaletteWidget::pathSelected, this, &MainWindow::setTrn1 );
            QObject::connect( this->trn2Widget, &PaletteWidget::pathSelected, this, &MainWindow::setTrn2 );

            // Refresh PAL/TRN view chain
            QObject::connect( this->palWidget, &PaletteWidget::refreshed, this->trn1Widget, &PaletteWidget::refresh );
            QObject::connect( this->trn1Widget, &PaletteWidget::refreshed, this->trn2Widget, &PaletteWidget::refresh );

            // Translation color selection
            QObject::connect( this->palWidget, &PaletteWidget::colorsSelected, this->trn1Widget, &PaletteWidget::checkTranslationsSelection );
            QObject::connect( this->trn1Widget, &PaletteWidget::colorsSelected, this->trn2Widget, &PaletteWidget::checkTranslationsSelection );
            QObject::connect( this->trn1Widget, &PaletteWidget::displayAllRootColors, this->palWidget, &PaletteWidget::temporarilyDisplayAllColors );
            QObject::connect( this->trn2Widget, &PaletteWidget::displayAllRootColors, this->trn1Widget, &PaletteWidget::temporarilyDisplayAllColors );
            QObject::connect( this->trn1Widget, &PaletteWidget::displayRootInformation, this->palWidget, &PaletteWidget::displayInfo );
            QObject::connect( this->trn2Widget, &PaletteWidget::displayRootInformation, this->trn1Widget, &PaletteWidget::displayInfo );
            QObject::connect( this->trn1Widget, &PaletteWidget::displayRootBorder, this->palWidget, &PaletteWidget::displayBorder );
            QObject::connect( this->trn2Widget, &PaletteWidget::displayRootBorder, this->trn1Widget, &PaletteWidget::displayBorder );
            QObject::connect( this->trn1Widget, &PaletteWidget::clearRootInformation, this->palWidget, &PaletteWidget::clearInfo );
            QObject::connect( this->trn2Widget, &PaletteWidget::clearRootInformation, this->trn1Widget, &PaletteWidget::clearInfo );
            QObject::connect( this->trn1Widget, &PaletteWidget::clearRootBorder, this->palWidget, &PaletteWidget::clearBorder );
            QObject::connect( this->trn2Widget, &PaletteWidget::clearRootBorder, this->trn1Widget, &PaletteWidget::clearBorder );

            // Send editing actions to the undo/redo stack
            QObject::connect( this->palWidget, &PaletteWidget::sendEditingCommand, this, &MainWindow::pushCommandToUndoStack );
            QObject::connect( this->trn1Widget, &PaletteWidget::sendEditingCommand, this, &MainWindow::pushCommandToUndoStack );
            QObject::connect( this->trn2Widget, &PaletteWidget::sendEditingCommand, this, &MainWindow::pushCommandToUndoStack );
            
            // Look for all palettes in the same folder as the CEL/CL2 file
            QFileInfo celFileInfo( openFilePath );
            QDirIterator it( celFileInfo.absolutePath(), QStringList() << "*.pal", QDir::Files );
            QString firstPaletteFound = QString();
            while( it.hasNext() )
            {
                QString sPath = it.next();

                if( sPath != "1" )
                {
                    QFileInfo palFileInfo( sPath );
                    QString path = palFileInfo.absoluteFilePath();
                    QString name = palFileInfo.fileName();
                    this->pals[path] = new D1Pal();

                    if( !this->pals[path]->load( path ) )
                    {
                        delete this->pals[path];
                        this->pals.remove( path );
                        QMessageBox::critical( this, "Error", "Could not load PAL file." );
                        return;
                    }

                    this->palWidget->addPath( path, name );

                    if( firstPaletteFound.isEmpty() )
                        firstPaletteFound = path;
                }
            }

            // If the CEL file is a level CEL file, then look for
            // associated MIN and TIL files
            if( this->cel->getType() == D1CEL_TYPE::V1_LEVEL )
            {
                QFileInfo celFileInfo = QFileInfo( openFilePath );
                celDirectory = celFileInfo.absolutePath();
                celFileName = celFileInfo.fileName();
                minFilePath = celDirectory + "/" + celFileName.toLower().replace(".cel",".min");
                tilFilePath = celDirectory + "/" + celFileName.toLower().replace(".cel",".til");

                if( QFileInfo::exists( minFilePath )
                    && QFileInfo::exists( tilFilePath ) )
                    minTilFound = true;
            }

            // If the CEL file is a level file and the required MIN and TIL
            // were found then build a LevelCelView
            if( minTilFound )
            {
                // Loading MIN
                this->min = new D1Min;
                if( !this->min->load( minFilePath ) )
                {
                    QMessageBox::critical( this, "Error", "Failed loading MIN file: "+minFilePath );
                    return;
                }
                this->min->setCel( this->cel );

                // Loading TIL
                this->til = new D1Til;
                if( !this->til->load( tilFilePath ) )
                {
                    QMessageBox::critical( this, "Error", "Failed loading TIL file: "+tilFilePath );
                    return;
                }
                this->til->setMin( this->min );

                this->levelCelView = new LevelCelView;
                this->levelCelView->initialize( this->cel, this->min, this->til );

                // Refresh CEL view if a PAL or TRN is modified
                QObject::connect( this->palWidget, &PaletteWidget::modified, this->levelCelView, &LevelCelView::displayFrame );
                QObject::connect( this->trn1Widget, &PaletteWidget::modified, this->levelCelView, &LevelCelView::displayFrame );
                QObject::connect( this->trn2Widget, &PaletteWidget::modified, this->levelCelView, &LevelCelView::displayFrame );

                // Select color when level CEL view clicked
                QObject::connect( this->levelCelView, &LevelCelView::colorIndexClicked, this->palWidget, &PaletteWidget::selectColor );
                QObject::connect( this->levelCelView, &LevelCelView::colorIndexClicked, this->trn1Widget, &PaletteWidget::selectColor );
                QObject::connect( this->levelCelView, &LevelCelView::colorIndexClicked, this->trn2Widget, &PaletteWidget::selectColor );

                // Refresh palette widgets when frame, subtile of tile is changed
                QObject::connect( this->levelCelView, &LevelCelView::frameRefreshed, this->palWidget, &PaletteWidget::refresh );

                // Initialize palette widgets
                this->palHits = new D1PalHits( this->cel, this->min, this->til );
                this->palWidget->initialize( this->pal, this->levelCelView, this->palHits );
                this->trn1Widget->initialize( this->pal, this->trn1, this->levelCelView, this->palHits );
                this->trn2Widget->initialize( this->trn1->getResultingPalette(), this->trn2, this->levelCelView, this->palHits );


                this->levelCelView->displayFrame();
            }
            // Otherwise build a CelView
            else
            {
                this->celView = new CelView;
                this->celView->initialize( this->cel );

                // Refresh CEL view if a PAL or TRN is modified
                QObject::connect( this->palWidget, &PaletteWidget::modified, this->celView, &CelView::displayFrame );
                QObject::connect( this->trn1Widget, &PaletteWidget::modified, this->celView, &CelView::displayFrame );
                QObject::connect( this->trn2Widget, &PaletteWidget::modified, this->celView, &CelView::displayFrame );

                // Select color when CEL view clicked
                QObject::connect( this->celView, &CelView::colorIndexClicked, this->palWidget, &PaletteWidget::selectColor );
                QObject::connect( this->celView, &CelView::colorIndexClicked, this->trn1Widget, &PaletteWidget::selectColor );
                QObject::connect( this->celView, &CelView::colorIndexClicked, this->trn2Widget, &PaletteWidget::selectColor );

                // Refresh palette widgets when frame
                QObject::connect( this->celView, &CelView::frameRefreshed, this->palWidget, &PaletteWidget::refresh );

                // Initialize palette widgets
                this->palHits = new D1PalHits( this->cel );
                this->palWidget->initialize( this->pal, this->celView, this->palHits );
                this->trn1Widget->initialize( this->pal, this->trn1, this->celView, this->palHits );
                this->trn2Widget->initialize( this->trn1->getResultingPalette(), this->trn2, this->celView, this->palHits );

                this->celView->displayFrame();
            }

            // Select the first palette found in the same folder as the CEL/CL2 if it exists
            if( !firstPaletteFound.isEmpty() )
                this->palWidget->selectPath( firstPaletteFound );

            // Adding the CelView to the main frame
            if( this->celView )
                this->ui->mainFrame->layout()->addWidget( this->celView );
            else
                this->ui->mainFrame->layout()->addWidget( this->levelCelView );

            // Adding the PalView to the pal frame
            //this->ui->palFrame->layout()->addWidget( this->palView );
            this->ui->menuPalette->setEnabled( true );
            this->ui->actionExport->setEnabled( true );

            // Clear loading message from status bar
            this->ui->statusBar->clearMessage();
        }
    }

    //QMessageBox::information( this, "Debug", celFilePath );
    //QMessageBox::information( this, "Debug", QString::number(cel->getFrameCount()));

    //QTime timer = QTime();
    //timer.start();
    //QMessageBox::information( this, "time", QString::number(timer.elapsed()) );
}

void MainWindow::on_actionClose_triggered()
{
    this->undoStack->clear();

    if( this->celView )
        delete this->celView;

    if( this->levelCelView )
        delete this->levelCelView;

    if( this->palWidget )
        delete this->palWidget;
    if( this->trn1Widget )
        delete this->trn1Widget;
    if( this->trn2Widget )
        delete this->trn2Widget;

    if( this->cel )
        delete this->cel;

    qDeleteAll( this->pals );
    this->pals = QMap<QString,D1Pal*>();

    qDeleteAll( this->trn1s );
    this->trn1s = QMap<QString,D1Trn*>();

    qDeleteAll( this->trn2s );
    this->trn2s = QMap<QString,D1Trn*>();

    if( this->min )
        delete this->min;

    if( this->til )
        delete this->til;

    if( this->palHits )
        delete this->palHits;

    ui->menuPalette->setEnabled( false );
    ui->actionExport->setEnabled( false );
}

void MainWindow::on_actionSettings_triggered()
{
    this->settingsDialog->initialize( this->configuration );
    this->settingsDialog->show();
}

void MainWindow::on_actionExport_triggered()
{
    if( this->min )
        this->exportDialog->setMin( this->min );

    if( this->til )
        this->exportDialog->setTil( this->til );

    if( this->cel )
        this->exportDialog->setCel( this->cel );

    this->exportDialog->show();
}

void MainWindow::on_actionBatchExport_triggered()
{
    this->batchExportDialog->show();
}

void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
}

void MainWindow::on_actionNew_PAL_triggered()
{
    QString palFilePath = QFileDialog::getSaveFileName(
        this, "New palette file", QString(), "PAL Files (*.pal)" );

    if( palFilePath.isEmpty() )
        return;

    QFileInfo palFileInfo( palFilePath );
    QString path = palFileInfo.absoluteFilePath();
    QString name = palFileInfo.fileName();

    this->pals[path] = new D1Pal();
    if( !this->pals[path]->load( ":/default.pal" ) )
    {
        delete this->pals[path];
        this->pals.remove( path );
        QMessageBox::critical( this, "Error", "Could not load PAL file." );
        return;
    }
    this->pals[path]->save( path );

    this->palWidget->addPath( path, name );
    this->palWidget->selectPath( path );
}

void MainWindow::on_actionOpen_PAL_triggered()
{
    QString palFilePath = QFileDialog::getOpenFileName(
        this, "Load Palette File", QString(), "PAL Files (*.pal)" );

    if( palFilePath.isEmpty() )
        return;

    QFileInfo palFileInfo( palFilePath );
    QString path = palFileInfo.absoluteFilePath();
    QString name = palFileInfo.fileName();

    this->pals[path] = new D1Pal();
    if( !this->pals[path]->load( path ) )
    {
        delete this->pals[path];
        this->pals.remove( path );
        QMessageBox::critical( this, "Error", "Could not load PAL file." );
        return;
    }

    this->palWidget->addPath( path, name );
    this->palWidget->selectPath( path );
}

void MainWindow::on_actionSave_PAL_triggered()
{
    QString selectedPath = this->palWidget->getSelectedPath();
    if( selectedPath.startsWith(":/") || selectedPath.isEmpty() )
    {
        this->on_actionSave_PAL_as_triggered();
    }
    else
    {
        if( !this->pal->save( selectedPath ) )
        {
            QMessageBox::critical( this, "Error", "Could not save PAL file." );
            return;
        }
    }
}

void MainWindow::on_actionSave_PAL_as_triggered()
{
    QString palFilePath = QFileDialog::getSaveFileName(
        this, "Save palette file as...", QString(), "PAL Files (*.pal)" );

    if( !palFilePath.isEmpty() )
    {
        if( !this->pal->save( palFilePath ) )
        {
            QMessageBox::critical( this, "Error", "Could not save PAL file." );
            return;
        }
    }

    QFileInfo palFileInfo( palFilePath );
    QString path = palFileInfo.absoluteFilePath();
    QString name = palFileInfo.fileName();

    if( this->pals.contains(path) )
        delete this->pals[path];

    this->pals[path] = new D1Pal( path );
    this->pal = this->pals[path];

    this->palWidget->addPath( path, name );
    this->palWidget->selectPath( path );
}

void MainWindow::on_actionClose_PAL_triggered()
{
    QString selectedPath = this->palWidget->getSelectedPath();
    if( selectedPath.startsWith(":/") || selectedPath.isEmpty() )
        return;

    if( this->pals.contains( selectedPath ) )
    {
        delete this->pals[selectedPath];
        this->pals.remove( selectedPath );
    }

    this->pal = this->pals[":/default.pal"];

    this->palWidget->removePath( selectedPath );
    this->palWidget->selectPath( ":/default.pal" );
}

void MainWindow::on_actionNew_Translation_1_triggered()
{
    QString trnFilePath = QFileDialog::getSaveFileName(
        this, "New translation", QString(), "TRN Files (*.trn)" );

    if( trnFilePath.isEmpty() )
        return;

    QFileInfo trnFileInfo( trnFilePath );
    QString path = trnFileInfo.absoluteFilePath();
    QString name = trnFileInfo.fileName();

    this->trn1s[path] = new D1Trn();
    this->trn1s[path]->setPalette( this->pal );
    if( !this->trn1s[path]->load( ":/null.trn" ) )
    {
        delete this->trn1s[path];
        this->trn1s.remove( path );
        QMessageBox::critical( this, "Error", "Could not load TRN file." );
        return;
    }
    this->trn1s[path]->save( path );

    this->trn1Widget->addPath( path, name );
    this->trn1Widget->selectPath( path );
}

void MainWindow::on_actionOpen_Translation_1_triggered()
{
    QString trnFilePath = QFileDialog::getOpenFileName(
        this, "Load Palette Translation File", QString(), "TRN Files (*.trn)" );

    if( trnFilePath.isEmpty() )
        return;

    QFileInfo trnFileInfo( trnFilePath );
    QString path = trnFileInfo.absoluteFilePath();
    QString name = trnFileInfo.fileName();

    this->trn1s[path] = new D1Trn();
    this->trn1s[path]->setPalette( this->pal );
    if( !this->trn1s[path]->load( path ) )
    {
        delete this->trn1s[path];
        this->trn1s.remove( path );
        QMessageBox::critical( this, "Error", "Could not load TRN file." );
        return;
    }

    this->trn1Widget->addPath( path, name );
    this->trn1Widget->selectPath( path );
}

void MainWindow::on_actionSave_Translation_1_triggered()
{
    QString selectedPath = this->trn1Widget->getSelectedPath();
    if( selectedPath.startsWith(":/") || selectedPath.isEmpty() )
    {
        this->on_actionSave_Translation_1_as_triggered();
    }
    else
    {
        if( !this->trn1->save( selectedPath ) )
        {
            QMessageBox::critical( this, "Error", "Could not save TRN file." );
            return;
        }
    }
}

void MainWindow::on_actionSave_Translation_1_as_triggered()
{
    QString trnFilePath = QFileDialog::getSaveFileName(
        this, "Save translation file as...", QString(), "TRN Files (*.trn)" );

    if( !trnFilePath.isEmpty() )
    {
        if( !this->trn1->save( trnFilePath ) )
        {
            QMessageBox::critical( this, "Error", "Could not save TRN file." );
            return;
        }
    }

    QFileInfo trnFileInfo( trnFilePath );
    QString path = trnFileInfo.absoluteFilePath();
    QString name = trnFileInfo.fileName();

    if( this->trn1s.contains(path) )
        delete this->trn1s[path];

    this->trn1s[path] = new D1Trn( path, this->pal );
    this->trn1 = this->trn1s[path];

    this->trn1Widget->addPath( path, name );
    this->trn1Widget->selectPath( path );
}

void MainWindow::on_actionClose_Translation_1_triggered()
{
    QString selectedPath = this->trn1Widget->getSelectedPath();
    if( selectedPath.startsWith(":/") || selectedPath.isEmpty() )
        return;

    if( this->trn1s.contains( selectedPath ) )
    {
        delete this->trn1s[selectedPath];
        this->trn1s.remove( selectedPath );
    }

    this->trn1 = this->trn1s[":/null.trn"];

    this->trn1Widget->removePath( selectedPath );
    this->trn1Widget->selectPath( ":/null.trn" );
}

void MainWindow::on_actionNew_Translation_2_triggered()
{
    QString trnFilePath = QFileDialog::getSaveFileName(
        this, "New translation", QString(), "TRN Files (*.trn)" );

    if( trnFilePath.isEmpty() )
        return;

    QFileInfo trnFileInfo( trnFilePath );
    QString path = trnFileInfo.absoluteFilePath();
    QString name = trnFileInfo.fileName();

    this->trn2s[path] = new D1Trn();
    this->trn2s[path]->setPalette( this->trn1->getResultingPalette() );
    if( !this->trn2s[path]->load( ":/null.trn" ) )
    {
        delete this->trn2s[path];
        this->trn2s.remove( path );
        QMessageBox::critical( this, "Error", "Could not load TRN file." );
        return;
    }
    this->trn2s[path]->save( path );

    this->trn2Widget->addPath( path, name );
    this->trn2Widget->selectPath( path );
}

void MainWindow::on_actionOpen_Translation_2_triggered()
{
    QString trnFilePath = QFileDialog::getOpenFileName(
        this, "Load Palette Translation File", QString(), "TRN Files (*.trn)" );

    if( trnFilePath.isEmpty() )
        return;

    QFileInfo trnFileInfo( trnFilePath );
    QString path = trnFileInfo.absoluteFilePath();
    QString name = trnFileInfo.fileName();

    this->trn2s[path] = new D1Trn();
    this->trn2s[path]->setPalette( this->pal );
    if( !this->trn2s[path]->load( path ) )
    {
        delete this->trn2s[path];
        this->trn2s.remove( path );
        QMessageBox::critical( this, "Error", "Could not load TRN file." );
        return;
    }

    this->trn2Widget->addPath( path, name );
    this->trn2Widget->selectPath( path );
}

void MainWindow::on_actionSave_Translation_2_triggered()
{
    QString selectedPath = this->trn2Widget->getSelectedPath();
    if( selectedPath.startsWith(":/") || selectedPath.isEmpty() )
    {
        this->on_actionSave_Translation_2_as_triggered();
    }
    else
    {
        if( !this->trn2->save( selectedPath ) )
        {
            QMessageBox::critical( this, "Error", "Could not save TRN file." );
            return;
        }
    }
}

void MainWindow::on_actionSave_Translation_2_as_triggered()
{
    QString trnFilePath = QFileDialog::getSaveFileName(
        this, "Save translation file as...", QString(), "TRN Files (*.trn)" );

    if( !trnFilePath.isEmpty() )
    {
        if( !this->trn2->save( trnFilePath ) )
        {
            QMessageBox::critical( this, "Error", "Could not save TRN file." );
            return;
        }
    }

    QFileInfo trnFileInfo( trnFilePath );
    QString path = trnFileInfo.absoluteFilePath();
    QString name = trnFileInfo.fileName();

    if( this->trn2s.contains(path) )
        delete this->trn2s[path];

    this->trn2s[path] = new D1Trn( path, this->trn1->getResultingPalette() );
    this->trn2 = this->trn2s[path];

    this->trn2Widget->addPath( path, name );
    this->trn2Widget->selectPath( path );
}

void MainWindow::on_actionClose_Translation_2_triggered()
{
    QString selectedPath = this->trn2Widget->getSelectedPath();
    if( selectedPath.startsWith(":/") || selectedPath.isEmpty() )
        return;

    if( this->trn2s.contains( selectedPath ) )
    {
        delete this->trn2s[selectedPath];
        this->trn2s.remove( selectedPath );
    }

    this->trn2 = this->trn1s[":/null.trn"];

    this->trn2Widget->removePath( selectedPath );
    this->trn2Widget->selectPath( ":/null.trn" );
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

    QMessageBox::about( this, "About", "Diablo 1 Graphics Tool "
        + QString( D1_GRAPHICS_TOOL_VERSION ) + " (" + operatingSystem + ") " + architecture );
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt( this, "About Qt" );
}
