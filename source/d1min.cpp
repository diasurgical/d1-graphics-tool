#include "d1min.h"

D1Min::D1Min() :
    type( D1MIN_TYPE::REGULAR_HEIGHT ),
    file( new QFile ),
    cel( NULL ),
    subtileWidth( 2 ),
    subtileHeight( 5 ),
    subtileCount( 0 )
{}

D1Min::D1Min( QString path, D1Cel* c ) :
    type( D1MIN_TYPE::REGULAR_HEIGHT ),
    file( new QFile ),
    cel( c ),
    subtileWidth( 2 ),
    subtileHeight( 5 ),
    subtileCount( 0 )
{
    this->load( path );
}

D1Min::~D1Min()
{
    if( this->file.isOpen() )
        this->file.close();
}

bool D1Min::load( QString minFilePath )
{
    quint8 subtileNumberOfCelFrames = 0;
    quint16 readWord;
    QList<quint16> celFrameIndicesList;
    QList<quint8> celFrameTypesList;

    // Opening MIN file with a QBuffer to load it in RAM
    if( !QFile::exists( minFilePath ) )
        return false;

    if( this->file.isOpen() )
        this->file.close();

    this->file.setFileName( minFilePath );

    if( !this->file.open(QIODevice::ReadOnly) )
        return false;

    if( this->file.size() < 64 )
        return false;

    QByteArray fileData = this->file.readAll();
    QBuffer fileBuffer( &fileData );

    if( !fileBuffer.open(QIODevice::ReadOnly) )
        return false;

    // Read MIN binary data
    QDataStream in( &fileBuffer );
    in.setByteOrder( QDataStream::LittleEndian );

    // Determine MIN type by checking if ground CEL frames
    // of the first 2 sub-tiles are empty or not
    for( int i = 0; i < 2; i++ )
    {
        fileBuffer.seek( 18+i*20 );
        in >> readWord;

        if( readWord == 0x0000 )
        {
            this->type = D1MIN_TYPE::EXTENDED_HEIGHT;
            break;
        }
    }

    // File size checks
    if( this->type == D1MIN_TYPE::REGULAR_HEIGHT )
    {
        if( this->file.size() % 20 != 0 )
            return false;
        this->subtileHeight = 5;
        this->subtileCount = this->file.size() / 20;
    }
    if( this->type == D1MIN_TYPE::EXTENDED_HEIGHT )
    {
        if( this->file.size() % 32 != 0 )
            return false;
        this->subtileHeight = 8;
        this->subtileCount = this->file.size() / 32;
    }

    // Read sub-tile data
    subtileNumberOfCelFrames = this->subtileWidth * this->subtileHeight;
    fileBuffer.seek( 0 );
    this->celFrameIndices.clear();
    this->celFrameTypes.clear();
    for( int i = 0; i < this->subtileCount; i++ )
    {
        celFrameIndicesList.clear();
        celFrameTypesList.clear();
        for( int j = 0; j < subtileNumberOfCelFrames; j++ )
        {
            in >> readWord;
            celFrameIndicesList.append( readWord & 0x0FFF );
            celFrameTypesList.append( (readWord & 0xF000) >> 12 );
        }
        this->celFrameIndices.append( celFrameIndicesList );
        this->celFrameTypes.append( celFrameTypesList );
    }

    return true;
}

QImage D1Min::getSubtileImage( quint16 subtileIndex )
{
    quint16 celFrameIndex = 0;
    quint16 dx = 0, dy = 0;
    QImage subtile;

    if( this->cel == NULL || subtileIndex >= this->celFrameIndices.size() )
        return QImage();

    //QList<quint16> test = this->celFrameIndices.at( subtileIndex );

    subtile = QImage( this->subtileWidth * 32,
        this->subtileHeight * 32, QImage::Format_ARGB32 );
    subtile.fill( Qt::transparent );
    QPainter subtilePainter( &subtile );

    for( int i = 0; i < this->subtileWidth*this->subtileHeight; i++ )
    {
        celFrameIndex = this->celFrameIndices.at( subtileIndex ).at( i );

        if( celFrameIndex > 0 )
            subtilePainter.drawImage( dx, dy,
                this->cel->getFrameImage( celFrameIndex-1 ) );

        if( dx == 32 )
        {
            dy += 32;
            dx = 0;
        }
        else
        {
            dx = 32;
        }
    }

    subtilePainter.end();
    return subtile;
}

D1MIN_TYPE D1Min::getType()
{
    return this->type;
}

QString D1Min::getFilePath()
{
    if( this->file.isOpen() )
        return this->file.fileName();
    else
        return QString();
}

bool D1Min::isFileOpen()
{
    return this->file.isOpen();
}

D1CelBase* D1Min::getCel()
{
    return this->cel;
}

void D1Min::setCel( D1CelBase* c )
{
    this->cel = c;
}

quint16 D1Min::getSubtileWidth()
{
    return this->subtileWidth;
}

quint16 D1Min::getSubtileHeight()
{
    return this->subtileHeight;
}

quint16 D1Min::getSubtileCount()
{
    return this->subtileCount;
}

QList<quint16> D1Min::getCelFrameIndices( quint16 subTileIndex )
{
    if( subTileIndex < this->subtileCount )
        return this->celFrameIndices.at( subTileIndex );
    else
        return QList<quint16>();
}
