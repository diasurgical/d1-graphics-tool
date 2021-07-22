#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTime>

MainWindow::MainWindow( QWidget *parent ) :
    QMainWindow( parent ),
    ui( new Ui::MainWindow ),
    configuration( new QJsonObject ),
    settingsDialog( new SettingsDialog(this) ),
    exportDialog( new ExportDialog(this) ),
    pal( new D1Pal ),
    trn1( new D1Trn ),
    trn2( new D1Trn ),
    cel( new D1Cel ),
    min( new D1Min ),
    til( new D1Til )
{
    //QCoreApplication::setAttribute( Qt::AA_EnableHighDpiScaling, true );

    ui->setupUi( this );

    this->loadConfiguration();
}

MainWindow::~MainWindow()
{
    delete ui;

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
}

void MainWindow::loadConfiguration()
{
    QString jsonFilePath = QCoreApplication::applicationDirPath() + "/D1GraphicsTool.config.json";

    // If configuration file exists load it otherwise create it
    if( QFile::exists(jsonFilePath) )
    {
        QFile loadJson(jsonFilePath);
        loadJson.open( QIODevice::ReadOnly );
        QJsonDocument loadJsonDoc = QJsonDocument::fromJson( loadJson.readAll() );
        this->configuration = new QJsonObject( loadJsonDoc.object() );
        loadJson.close();
    }
    else
    {
        this->configuration->insert( "WorkingDirectory",QCoreApplication::applicationDirPath() );

        QFile saveJson( jsonFilePath );
        saveJson.open( QIODevice::WriteOnly );
        QJsonDocument saveDoc( *this->configuration );
        saveJson.write( saveDoc.toJson() );
        saveJson.close();
    }
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
            this->pal = new D1Pal;
            this->pal->load( ":/default.pal" );

            // Loading default null.trn
            this->trn1 = new D1Trn;
            this->trn1->setPalette( this->pal );
            this->trn1->load( ":/null.trn" );
            this->trn2 = new D1Trn;
            this->trn2->setPalette( this->trn1->getResultingPalette() );
            this->trn2->load( ":/null.trn" );

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
            this->palWidget = new PaletteWidget( nullptr, "Palette" );
            this->trn2Widget = new PaletteWidget( nullptr, "Translation" );
            this->trn1Widget = new PaletteWidget( nullptr, "Unique translation" );
            this->ui->palFrame->layout()->addWidget( this->palWidget );
            this->ui->palFrame->layout()->addWidget( this->trn2Widget );
            this->ui->palFrame->layout()->addWidget( this->trn1Widget );

            // Refresh PAL/TRN view chain
            QObject::connect( this->palWidget, &PaletteWidget::refreshed, this->trn1Widget, &PaletteWidget::refresh );
            QObject::connect( this->trn1Widget, &PaletteWidget::refreshed, this->trn2Widget, &PaletteWidget::refresh );

            // Look for all palettes in the same folder as the CEL/CL2 file
            QFileInfo celFileInfo( openFilePath );
            QDirIterator it( celFileInfo.absolutePath(), QStringList() << "*.pal", QDir::Files );
            QString firstPaletteFound = QString();
            bool relevantPaletteFound = false;
            while( it.hasNext() )
            {
                QString path = it.next();

                if( path != "1" )
                {
                    if( firstPaletteFound.isEmpty() )
                        firstPaletteFound = path;

                    QFileInfo palFileInfo( path );
                    this->palWidget->addPath( palFileInfo.fileName(), palFileInfo.absoluteFilePath() );

                    if( !relevantPaletteFound
                        && palFileInfo.fileName().toLower().startsWith(
                        celFileInfo.fileName().toLower().replace(".cel","").replace(".cl2","") ) )
                    {
                        if( !this->pal->load( palFileInfo.absoluteFilePath() ) )
                        {
                            QMessageBox::critical( this, "Error", "Could not load PAL file." );
                            return;
                        }

                        this->trn1->refreshResultingPalette();
                        this->trn2->refreshResultingPalette();

                        relevantPaletteFound = true;
                    }

                    if( !it.hasNext() && !relevantPaletteFound )
                    {
                        if( !this->pal->load( firstPaletteFound ) )
                        {
                            QMessageBox::critical( this, "Error", "Could not load PAL file." );
                            return;
                        }
                        this->trn1->refreshResultingPalette();
                        this->trn2->refreshResultingPalette();
                    }
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

                // Refresh CEL view if a PAL or TRN is modified
                QObject::connect( this->palWidget, &PaletteWidget::modified, this->levelCelView, &LevelCelView::displayFrame );
                QObject::connect( this->trn1Widget, &PaletteWidget::modified, this->levelCelView, &LevelCelView::displayFrame );
                QObject::connect( this->trn2Widget, &PaletteWidget::modified, this->levelCelView, &LevelCelView::displayFrame );

                this->levelCelView->initialize( this->cel, this->min, this->til );

                this->palWidget->initialize( this->pal, this->levelCelView );
                this->trn1Widget->initialize( this->pal, this->trn1, this->levelCelView );
                this->trn2Widget->initialize( this->trn1->getResultingPalette(), this->trn2, this->levelCelView );

                this->levelCelView->displayFrame();
            }
            // Otherwise build a CelView
            else
            {
                this->celView = new CelView;

                // Refresh CEL view if a PAL or TRN is modified
                QObject::connect( this->palWidget, &PaletteWidget::modified, this->celView, &CelView::displayFrame );
                QObject::connect( this->trn1Widget, &PaletteWidget::modified, this->celView, &CelView::displayFrame );
                QObject::connect( this->trn2Widget, &PaletteWidget::modified, this->celView, &CelView::displayFrame );

                this->celView->initialize( this->cel );

                this->palWidget->initialize( this->pal, this->celView );
                this->trn1Widget->initialize( this->pal, this->trn1, this->celView );
                this->trn2Widget->initialize( this->trn1->getResultingPalette(), this->trn2, this->celView );

                this->celView->displayFrame();
            }


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

    if( this->pal )
        delete pal;

    if( this->trn1 )
        delete trn1;

    if( this->trn2 )
        delete trn2;

    if( this->min )
        delete min;

    if( this->til )
        delete til;

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

void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
}

void MainWindow::on_actionOpen_PAL_triggered()
{
    QString palFilePath = QFileDialog::getOpenFileName(
        this, "Load Palette File", QString(), "PAL Files (*.pal)" );

    if( !palFilePath.isEmpty() )
    {
        if( !this->pal->load( palFilePath ) )
        {
            QMessageBox::critical( this, "Error", "Could not load PAL file." );
            return;
        }
    }

    this->trn1->refreshResultingPalette();
    this->trn2->refreshResultingPalette();

    // Add file name and file path to the PaletteWidget
    QFileInfo palFileInfo( this->pal->getFilePath() );
    this->palWidget->addPath( palFileInfo.fileName(), this->pal->getFilePath() );
    this->palWidget->refresh();
}

void MainWindow::on_actionOpen_Translation_1_triggered()
{
    QString trnFilePath = QFileDialog::getOpenFileName(
        this, "Load Palette Translation File", QString(), "TRN Files (*.trn)" );

    if( !trnFilePath.isEmpty() )
    {
        if( !this->trn1->load( trnFilePath ) )
        {
            QMessageBox::critical( this, "Error", "Could not load TRN file." );
            return;
        }
    }

    this->trn1->refreshResultingPalette();
    this->trn2->refreshResultingPalette();

    // Add file name and file path to the PaletteWidget
    QFileInfo trnFileInfo( this->trn1->getFilePath() );
    this->trn1Widget->addPath( trnFileInfo.fileName(), this->trn1->getFilePath() );
    this->trn1Widget->setSelectedPath( trnFileInfo.fileName() );
}

void MainWindow::on_actionOpen_Translation_2_triggered()
{
    QString trnFilePath = QFileDialog::getOpenFileName(
        this, "Load Palette Translation File", QString(), "TRN Files (*.trn)" );

    if( !trnFilePath.isEmpty() )
    {
        if( !this->trn2->load( trnFilePath ) )
        {
            QMessageBox::critical( this, "Error", "Could not load TRN file." );
            return;
        }
    }

    this->trn2->refreshResultingPalette();

    // Add file name and file path to the PaletteWidget
    QFileInfo trnFileInfo( this->trn2->getFilePath() );
    this->trn2Widget->addPath( trnFileInfo.fileName(), this->trn2->getFilePath() );
    this->trn2Widget->setSelectedPath( trnFileInfo.fileName() );
}

void MainWindow::on_actionReset_PAL_triggered()
{
    if( this->celView )
        this->celView->displayFrame();
    if( this->levelCelView )
        this->levelCelView->displayFrame();
}

void MainWindow::on_actionReset_Translation_1_triggered()
{
    this->trn1->load( ":/null.trn" );
    this->trn1->refreshResultingPalette();
    this->trn2->refreshResultingPalette();

    if( this->celView )
        this->celView->displayFrame();
    if( this->levelCelView )
        this->levelCelView->displayFrame();
}

void MainWindow::on_actionReset_Translation_2_triggered()
{
    this->trn2->load( ":/null.trn" );
    this->trn2->refreshResultingPalette();

    if( this->celView )
        this->celView->displayFrame();
    if( this->levelCelView )
        this->levelCelView->displayFrame();
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
