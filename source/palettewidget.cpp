#include "palettewidget.h"
#include "ui_palettewidget.h"

PaletteWidget::PaletteWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaletteWidget),
    isCelLevel( false ),
    isTrn( false ),
    scene( new QGraphicsScene(0,0,PALETTE_WIDTH,PALETTE_WIDTH) ),
    selectedColorIndex( 0 ),
    buildingPathComboBox( false )
{
    ui->setupUi(this);
    ui->graphicsView->setScene( this->scene );

    // Slots need to be written connected manually because I use multiple instances of PaletteWidget
    // thus Qt is not able to differentiate between children widgets with the same name.
    // e.g. the pathComboBox will be present three times, one for the PAL and two for the TRNs.
    QObject::connect(
        this->findChild<QComboBox*>("pathComboBox"), &QComboBox::currentIndexChanged,
        this, &PaletteWidget::pathComboBox_currentIndexChanged );


    // Install the mouse events filter on the QGraphicsView
    ui->graphicsView->installEventFilter( this );
}

PaletteWidget::~PaletteWidget()
{
    delete ui;
}

void PaletteWidget::initialize( D1Pal *p, CelView *c )
{
    this->pal = p;
    this->celView = c;

    this->selectedColor = this->pal->getColor( 0 );

    this->initializeUi();
}

void PaletteWidget::initialize( D1Pal *p, LevelCelView *lc )
{
    this->pal = p;
    this->isCelLevel = true;
    this->levelCelView = lc;

    this->initializeUi();
}

void PaletteWidget::initialize( D1Pal *p, D1Trn *t, CelView *c )
{
    this->isTrn = true;
    this->pal = p;
    this->trn = t;
    this->celView = c;

    this->initializeUi();
}

void PaletteWidget::initialize( D1Pal *p, D1Trn *t, LevelCelView *lc )
{
    this->isTrn = true;
    this->pal = p;
    this->trn = t;
    this->isCelLevel = true;
    this->levelCelView = lc;

    this->initializeUi();
}

void PaletteWidget::initializeUi()
{
    if( this->isTrn )
    {
        this->ui->indexLineEdit->setEnabled( true );
        this->ui->indexPickPushButton->setEnabled( true );
        this->ui->indexResetPushButton->setEnabled( true );
        this->ui->colorLineEdit->setEnabled( false );
        this->ui->colorPickPushButton->setEnabled( false );
        this->ui->colorResetPushButton->setEnabled( false );
    }

    this->initializePathComboBox();
    this->refreshColorLineEdit();
    this->refreshIndexLineEdit();
    this->displayColors();
    this->displaySelection();
}

void PaletteWidget::initializePathComboBox()
{
    if( !this->isTrn )
    {
        this->paths["_default.pal"] = ":/default.pal";
        this->paths["_town.pal"] = ":/town.pal";
    }
    else
    {
        this->paths["_null.trn"] = ":/null.trn";
        //this->paths["_null2.trn"] = ":/null.trn";
    }

    this->refreshPathComboBox();
}

void PaletteWidget::selectColor( quint8 index )
{
    this->selectedColorIndex = index;
    this->selectedColor = this->pal->getColor( index );

    this->refresh();
    emit colorSelected( index );
}

void PaletteWidget::addPath( QString name, QString path )
{
    this->paths[name] = path;
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
bool PaletteWidget::eventFilter(QObject *obj, QEvent *event)
{
    if( event->type() == QEvent::MouseButtonPress )
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        qDebug() << "Clicked: " << mouseEvent->position().x() << "," << mouseEvent->position().y();

        // Check if selected color has changed
        quint8 colorIndex = getColorIndexFromCoordinates( mouseEvent->position() );

        if( colorIndex != this->selectedColorIndex )
        {
            this->selectColor( colorIndex );
        }
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
    for( int i = 0; i < D1PAL_COLORS; i++ )
    {
        // Go to next line
        if( i%PALETTE_COLORS_PER_LINE == 0 && i != 0 )
        {
            x = 0;
            y += dy;
        }

        QBrush brush( this->pal->getColor(i) );
        QPen pen( Qt::NoPen );
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

void PaletteWidget::refreshPathComboBox()
{
    // This boolean is used to avoid infinite loop when adding items to the combo box
    // because adding items calls pathComboBox_currentIndexChanged() which itself calls
    // refresh() which calls pathComboBox_currentIndexChanged(), ...
    this->buildingPathComboBox = true;

    ui->pathComboBox->clear();
    for( int i = 0; i < this->paths.keys().size(); i++ )
        ui->pathComboBox->addItem( this->paths.keys().at(i) );
    ui->pathComboBox->setCurrentText( this->paths.key(this->pal->getFilePath()) );

    this->buildingPathComboBox = false;
}

void PaletteWidget::refreshColorLineEdit()
{
    this->ui->colorLineEdit->setText( selectedColor.name() );
}

void PaletteWidget::refreshIndexLineEdit()
{
    this->ui->indexLineEdit->setText( QString::number(selectedColorIndex) );
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

    emit refreshed();
}

void PaletteWidget::pathComboBox_currentIndexChanged( int index )
{
    if( this->paths.isEmpty() || this->buildingPathComboBox )
        return;

    QString filePath = this->paths[ this->ui->pathComboBox->currentText() ];

    if( !filePath.isEmpty() )
    {
        if( !this->isTrn )
        {
            if( !this->pal->load( filePath ) )
            {
                QMessageBox::critical( this, "Error", "Could not load PAL file." );
                return;
            }
        }
        else
        {
            if( !this->trn->load( filePath ) )
            {
                QMessageBox::critical( this, "Error", "Could not load TRN file." );
                return;
            }
        }
    }

    this->refresh();
    emit this->modified();
}
