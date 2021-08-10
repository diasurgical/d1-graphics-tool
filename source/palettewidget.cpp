#include "palettewidget.h"
#include "ui_palettewidget.h"

EditColorsCommand::EditColorsCommand( D1Pal* p, quint8 sci, quint8 eci, QColor nc, QUndoCommand *parent ) :
    QUndoCommand(parent),
    pal( p ),
    startColorIndex( sci ),
    endColorIndex( eci ),
    newColor( nc )
{
    // Get the initial color values before doing any modification
    for( int i = startColorIndex; i <= endColorIndex; i++ )
        initialColors.append( this->pal->getColor(i) );

}

EditColorsCommand::~EditColorsCommand()
{}

void EditColorsCommand::undo()
{
    if( this->pal.isNull() )
        return;

    for( int i = startColorIndex; i <= endColorIndex; i++ )
        this->pal->setColor( i, this->initialColors.at(i-this->startColorIndex) );
}

void EditColorsCommand::redo()
{
    if( this->pal.isNull() )
        return;

    for( int i = startColorIndex; i <= endColorIndex; i++ )
        this->pal->setColor( i, this->newColor );
}

PaletteWidget::PaletteWidget(QWidget *parent, QString title) :
    QWidget(parent),
    ui(new Ui::PaletteWidget),
    isLevelCel( false ),
    isTrn( false ),
    scene( new QGraphicsScene(0,0,PALETTE_WIDTH,PALETTE_WIDTH) ),
    selectedColorIndex( 0 ),
    selectedFirstColorIndex( 0 ),
    selectedLastColorIndex( 0 ),
    selectedTranslationIndex( 0 ),
    selectedFirstTranslationIndex( 0 ),
    selectedLastTranslationIndex( 0 ),
    pickingTranslationColor( false ),
    temporarilyDisplayingAllColors( false ),
    buildingPathComboBox( false )
{
    ui->setupUi(this);
    ui->graphicsView->setScene( this->scene );

    ui->groupBox->setTitle( title );

    // When there is a modification to the PAL or TRNs then UI must be refreshed
    QObject::connect( this, &PaletteWidget::modified, this, &PaletteWidget::refresh );

    // Slots need to be written connected manually because I use multiple instances of PaletteWidget
    // thus Qt is not able to differentiate between children widgets with the same name.
    // e.g. the pathComboBox will be present three times, one for the PAL and two for the TRNs.
    QObject::connect(
        this->findChild<QComboBox*>("pathComboBox"), &QComboBox::currentTextChanged,
        this, &PaletteWidget::pathComboBox_currentTextChanged );
    QObject::connect(
        this->findChild<QComboBox*>("displayComboBox"), &QComboBox::currentTextChanged,
        this, &PaletteWidget::displayComboBox_currentTextChanged );

    // Install the mouse events filter on the QGraphicsView
    ui->graphicsView->installEventFilter( this );
}

PaletteWidget::~PaletteWidget()
{
    delete ui;
}

void PaletteWidget::setPal( D1Pal *p )
{
    this->pal = p;

    emit this->modified();
}

void PaletteWidget::setTrn( D1Trn *t )
{
    this->trn = t;

    emit this->modified();
}

void PaletteWidget::initialize( D1Pal *p, CelView *c, D1PalHits *ph )
{
    this->pal = p;
    this->celView = c;
    this->palHits = ph;

    this->initializeUi();
}

void PaletteWidget::initialize( D1Pal *p, LevelCelView *lc, D1PalHits *ph )
{
    this->pal = p;
    this->isLevelCel = true;
    this->levelCelView = lc;
    this->palHits = ph;

    this->initializeUi();
}

void PaletteWidget::initialize( D1Pal *p, D1Trn *t, CelView *c, D1PalHits *ph )
{
    this->isTrn = true;
    this->pal = p;
    this->trn = t;
    this->celView = c;
    this->palHits = ph;

    this->initializeUi();
}

void PaletteWidget::initialize( D1Pal *p, D1Trn *t, LevelCelView *lc, D1PalHits *ph )
{
    this->isTrn = true;
    this->pal = p;
    this->trn = t;
    this->isLevelCel = true;
    this->levelCelView = lc;
    this->palHits = ph;

    this->initializeUi();
}

void PaletteWidget::initializeUi()
{
    if( this->isTrn )
    {
        this->ui->indexPickPushButton->setEnabled( true );
        this->ui->colorLineEdit->setEnabled( false );
        this->ui->colorPickPushButton->setEnabled( false );
    }
    else
    {
        this->ui->translationIndexLineEdit->hide();
        this->ui->translationLabel->hide();
    }

    this->selectedColor = this->pal->getColor( 0 );
    if( this->isTrn )
        this->selectedTranslationIndex = this->trn->getTranslation( 0 );


    this->initializePathComboBox();
    this->initializeDisplayComboBox();

    this->refreshColorLineEdit();
    this->refreshIndexLineEdit();
    if( this->isTrn )
        this->refreshTranslationIndexLineEdit();

    this->displayColors();
    this->displaySelection();
}

void PaletteWidget::initializePathComboBox()
{
    if( !this->isTrn )
    {
        this->paths[":/default.pal"] = "_default.pal";
    }
    else
    {
        this->paths[":/null.trn"] = "_null.trn";
    }

    this->refreshPathComboBox();
}

void PaletteWidget::initializeDisplayComboBox()
{
    this->buildingDisplayComboBox = true;

    ui->displayComboBox->addItem("Show all colors");

    if( !this->isTrn )
    {
        ui->displayComboBox->addItem("Show all frames hits");
        if( this->isLevelCel )
        {
            ui->displayComboBox->addItem("Show current tile hits");
            ui->displayComboBox->addItem("Show current sub-tile hits");
        }
        ui->displayComboBox->addItem("Show current frame hits");
    }
    else
    {
        ui->displayComboBox->addItem("Show translated colors");
    }

    this->buildingDisplayComboBox = false;
}

void PaletteWidget::selectColor( quint8 index )
{
    this->selectedColorIndex = index;

    // TODO: remove
    this->selectedFirstColorIndex = index;
    this->selectedLastColorIndex = index;


    this->selectedColor = this->pal->getColor( index );

    if( this->isTrn )
    {
        this->selectedTranslationIndex = this->trn->getTranslation( index );

        if( this->pickingTranslationColor )
        {
            this->clearInfo();
            emit this->clearRootInformation();
            emit this->clearRootBorder();
            this->pickingTranslationColor = false;
        }
    }

    this->temporarilyDisplayingAllColors = false;

    this->refresh();
    emit colorSelected( index );
}

void PaletteWidget::checkTranslationSelection( quint8 index )
{
    if( !this->pickingTranslationColor )
        return;

    this->selectedTranslationIndex = index;
    this->trn->setTranslation( this->selectedColorIndex, index );

    emit this->modified();

    this->pickingTranslationColor = false;
    this->clearInfo();

    emit this->clearRootInformation();
    emit this->clearRootBorder();
}

QString PaletteWidget::getPath( QString name )
{
    // Returns empty string if not found
    return this->paths.key( name );
}

void PaletteWidget::setPath( QString path, QString name )
{
    this->paths[path] = name;
}

void PaletteWidget::addPath( QString path, QString name )
{
    this->paths[path] = name;
}

void PaletteWidget::removePath( QString path )
{
    if( this->paths.contains(path) )
        this->paths.remove(path);
}

void PaletteWidget::selectPath( QString path )
{
    this->ui->pathComboBox->setCurrentText( this->paths[path] );
    this->ui->pathComboBox->setToolTip( path );

    emit this->pathSelected( path );
    emit this->modified();
}

QString PaletteWidget::getSelectedPath()
{
    return this->paths.key( this->ui->pathComboBox->currentText() );
}

QRectF PaletteWidget::getColorCoordinates( quint8 index )
{
    int ix = index % PALETTE_COLORS_PER_LINE;
    int iy = index / PALETTE_COLORS_PER_LINE;

    int w = PALETTE_WIDTH / PALETTE_COLORS_PER_LINE;

    QRectF coordinates( ix*w, iy*w, w, w );

    return coordinates;
}

quint8 PaletteWidget::getColorIndexFromCoordinates( QPointF coordinates )
{
    quint8 index = 0;

    int w = PALETTE_WIDTH / PALETTE_COLORS_PER_LINE;

    int ix = coordinates.x() / w;
    int iy = coordinates.y() / w;

    index = iy * PALETTE_COLORS_PER_LINE + ix;

    return index;
}

// This event filter is used on the QGraphicsView
bool PaletteWidget::eventFilter( QObject *obj, QEvent *event )
{
    if( event->type() == QEvent::MouseButtonPress )
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        qDebug() << "Clicked: " << mouseEvent->position().x() << "," << mouseEvent->position().y();

        // Check if selected color has changed
        quint8 colorIndex = getColorIndexFromCoordinates( mouseEvent->position() );
        this->selectColor( colorIndex );
        
        return true;
    }
    if( event->type() == QEvent::MouseButtonDblClick )
    {

        return true;
    }
    else
    {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void PaletteWidget::displayColors()
{
    // Positions
    int x = 0;
    int y = 0;

    // X delta
    int dx = PALETTE_WIDTH/PALETTE_COLORS_PER_LINE;
    // Y delta
    int dy = PALETTE_WIDTH/PALETTE_COLORS_PER_LINE;

    // Color width
    int w = PALETTE_WIDTH/PALETTE_COLORS_PER_LINE - 2*PALETTE_COLOR_SPACING;
    int bsw = PALETTE_COLOR_SPACING;

    // Removing existing items
    this->scene->clear();

    // Setting background color
    this->scene->setBackgroundBrush( Qt::white );

    // Displaying palette colors
    bool displayColor;
    quint32 indexHits;
    for( int i = 0; i < D1PAL_COLORS; i++ )
    {
        // Go to next line
        if( i%PALETTE_COLORS_PER_LINE == 0 && i != 0 )
        {
            x = 0;
            y += dy;
        }

        QBrush brush;
        if( !this->isTrn )
            brush = QBrush( this->pal->getColor(i) );
        else
            brush = QBrush( this->trn->getResultingPalette()->getColor(i) );
        QPen pen( Qt::NoPen );

        // Check palette display filter
        displayColor = true;
        indexHits = 0;

        // if user just click "Pick" button to select color in parent palette or translation, display all colors
        if( this->temporarilyDisplayingAllColors || this->palHits->getMode() == D1PALHITS_MODE::ALL_COLORS )
            indexHits = 1;
        else if( this->palHits->getMode() == D1PALHITS_MODE::ALL_FRAMES )
            indexHits = this->palHits->getIndexHits(i);
        else if( this->palHits->getMode() == D1PALHITS_MODE::CURRENT_TILE )
            indexHits = this->palHits->getIndexHits( i, this->levelCelView->getCurrentTileIndex() );
        else if( this->palHits->getMode() == D1PALHITS_MODE::CURRENT_SUBTILE )
            indexHits = this->palHits->getIndexHits( i, this->levelCelView->getCurrentSubtileIndex() );
        else if( this->palHits->getMode() == D1PALHITS_MODE::CURRENT_FRAME && !this->isLevelCel )
            indexHits = this->palHits->getIndexHits( i, this->celView->getCurrentFrameIndex() );
        else if( this->palHits->getMode() == D1PALHITS_MODE::CURRENT_FRAME && this->isLevelCel )
            indexHits = this->palHits->getIndexHits( i, this->levelCelView->getCurrentFrameIndex() );

        if( indexHits == 0 )
            displayColor = false;

        // Check translation display filter
        if( this->isTrn && ui->displayComboBox->currentText() == "Show translated colors"
            && this->trn->getTranslation(i) == i )
            displayColor = false;


        if( displayColor )
            this->scene->addRect( x+bsw, y+bsw, w, w, pen, brush );

        x += dx;
    }
}

void PaletteWidget::displaySelection()
{
    QBrush brush( Qt::NoBrush );
    QPen pen( Qt::red );
    pen.setStyle( Qt::SolidLine );
    pen.setJoinStyle( Qt::MiterJoin );
    pen.setWidth( PALETTE_SELECTION_WIDTH );

    QRectF coordinates = getColorCoordinates( selectedColorIndex );
    int a = PALETTE_SELECTION_WIDTH/2;
    coordinates.adjust( a, a, -a, -a );

    this->scene->addRect( coordinates, pen, brush );
}

void PaletteWidget::temporarilyDisplayAllColors()
{
    this->temporarilyDisplayingAllColors = true;
    this->displayColors();
}

void PaletteWidget::displayInfo( QString info )
{
    this->ui->informationLabel->setText( info );
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
    // This boolean is used to avoid infinite loop when adding items to the combo box
    // because adding items calls pathComboBox_currentIndexChanged() which itself calls
    // refresh() which calls pathComboBox_currentIndexChanged(), ...
    this->buildingPathComboBox = true;

    this->ui->pathComboBox->clear();


    // Go through the hits of the CEL frame and add them to the subtile hits
    QMapIterator<QString,QString> it( this->paths );
    while( it.hasNext() )
    {
        it.next();
        this->ui->pathComboBox->addItem( it.value() );
    }

    if( !this->isTrn )
    {
        this->ui->pathComboBox->setCurrentText( this->paths[this->pal->getFilePath()] );
        this->ui->pathComboBox->setToolTip( this->pal->getFilePath() );
    }
    else
    {
        this->ui->pathComboBox->setCurrentText( this->paths[this->trn->getFilePath()] );
        this->ui->pathComboBox->setToolTip( this->trn->getFilePath() );
    }

    this->buildingPathComboBox = false;
}

void PaletteWidget::refreshColorLineEdit()
{
    this->selectedColor = this->pal->getColor( this->selectedColorIndex );
    this->ui->colorLineEdit->setText( this->selectedColor.name() );
}

void PaletteWidget::refreshIndexLineEdit()
{
    this->ui->indexLineEdit->setText( QString::number(selectedColorIndex) );
}

void PaletteWidget::refreshTranslationIndexLineEdit()
{
    this->ui->translationIndexLineEdit->setText( QString::number(selectedTranslationIndex) );
}

void PaletteWidget::refresh()
{
    if( this->isTrn )
        this->trn->refreshResultingPalette();

    this->displayColors();
    this->displaySelection();
    this->refreshPathComboBox();
    this->refreshColorLineEdit();
    this->refreshIndexLineEdit();
    if( this->trn )
        this->refreshTranslationIndexLineEdit();

    emit refreshed();
}

void PaletteWidget::pathComboBox_currentTextChanged( const QString &arg1 )
{
    if( this->paths.isEmpty() || this->buildingPathComboBox )
        return;

    QString filePath = this->paths.key( arg1 );

    // Set tooltip to display full file path when mouse hover
    ui->pathComboBox->setToolTip( filePath );

    emit this->pathSelected( filePath );
    emit this->modified();
}

void PaletteWidget::displayComboBox_currentTextChanged( const QString &arg1 )
{
    if( this->buildingDisplayComboBox )
        return;

    if( arg1 == "Show all colors" && !this->isTrn )
        this->palHits->setMode( D1PALHITS_MODE::ALL_COLORS );
    else if( arg1 == "Show all frames hits" )
        this->palHits->setMode( D1PALHITS_MODE::ALL_FRAMES );
    else if( arg1 == "Show current tile hits" )
        this->palHits->setMode( D1PALHITS_MODE::CURRENT_TILE );
    else if( arg1 == "Show current sub-tile hits" )
        this->palHits->setMode( D1PALHITS_MODE::CURRENT_SUBTILE );
    else if( arg1 == "Show current frame hits" )
        this->palHits->setMode( D1PALHITS_MODE::CURRENT_FRAME );

    this->refresh();
}

void PaletteWidget::on_colorLineEdit_returnPressed()
{
    QColor color = QColor( ui->colorLineEdit->text() );

    // TODO: remove this variable
    this->selectedColor = color;

    EditColorsCommand *command = new EditColorsCommand(
        this->pal, this->selectedFirstColorIndex, this->selectedLastColorIndex, color );

    emit this->sendEditingCommand( command );

    //this->pal->setColor( this->selectedColorIndex, color );

    emit this->modified();
}

void PaletteWidget::on_translationIndexLineEdit_returnPressed()
{
    quint8 index = ui->translationIndexLineEdit->text().toUInt();

    this->selectedTranslationIndex = index;
    this->trn->setTranslation( this->selectedColorIndex, index );

    emit this->modified();
}


void PaletteWidget::on_colorPickPushButton_clicked()
{
    QColor color = QColorDialog::getColor();

    this->selectedColor = color;
    this->pal->setColor( this->selectedColorIndex, color );

    emit this->modified();
}


void PaletteWidget::on_indexResetPushButton_clicked()
{
    this->trn->setTranslation( this->selectedColorIndex, this->selectedColorIndex );

    emit this->modified();
}


void PaletteWidget::on_indexPickPushButton_clicked()
{
    this->pickingTranslationColor = true;

    emit this->displayAllRootColors();
    emit this->displayRootInformation( "<- Select translation" );
    emit this->displayRootBorder();
}
