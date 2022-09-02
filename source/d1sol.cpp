#include "d1sol.h"

D1Sol::D1Sol() :
    file( )
{}

D1Sol::D1Sol( QString path ) :
    file( )
{
    this->load( path );
}

D1Sol::~D1Sol()
{
    if( this->file.isOpen() )
        this->file.close();
}

bool D1Sol::load( QString solFilePath )
{
    // Opening SOL file with a QBuffer to load it in RAM
    if( !QFile::exists( solFilePath ) )
        return false;

    if( this->file.isOpen() )
        this->file.close();

    this->file.setFileName( solFilePath );

    if( !this->file.open(QIODevice::ReadOnly) )
        return false;

    QByteArray fileData = this->file.readAll();
    QBuffer fileBuffer( &fileData );

    if( !fileBuffer.open(QIODevice::ReadOnly) )
        return false;

    // Read SOL binary data
    QDataStream in( &fileBuffer );
    in.setByteOrder( QDataStream::LittleEndian );

    this->tileCount = this->file.size();

    quint8 readBytr;
    this->subProperties.clear();
    for( int i = 0; i < this->tileCount; i++ )
    {
        in >> readBytr;
        this->subProperties.append( readBytr );
    }

    return true;
}

QString D1Sol::getFilePath()
{
    if( this->file.isOpen() )
        return this->file.fileName();
    else
        return QString();
}

bool D1Sol::isFileOpen()
{
    return this->file.isOpen();
}

quint16 D1Sol::getTileCount()
{
    return this->tileCount;
}

quint8 D1Sol::getSubtileProperties( quint16 tileIndex )
{
    if( tileIndex < this->tileCount )
        return this->subProperties.at( tileIndex );
    else
        return 0;
}
