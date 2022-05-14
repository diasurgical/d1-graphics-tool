#include "d1trn.h"

D1Trn::D1Trn() :
    modified( false ),
    file( new QFile ),
    translations( new quint8[D1TRN_TRANSLATIONS] ),
    resultingPalette( new D1Pal )
{}

D1Trn::D1Trn( QString path, D1Pal* pal ) :
    modified( false ),
    file( new QFile ),
    translations( new quint8[D1TRN_TRANSLATIONS] ),
    palette( QPointer<D1Pal>(pal) ),
    resultingPalette( new D1Pal )
{
    this->load( path );
}

D1Trn::~D1Trn()
{
    delete[] translations;

    if( this->file.isOpen() )
        this->file.close();
}

bool D1Trn::load( QString trnFilePath )
{
    if( this->palette.isNull() )
        return false;

    if( !QFile::exists( trnFilePath ) )
        return false;

    if( this->file.isOpen() )
        file.close();

    this->file.setFileName( trnFilePath );

    if( !this->file.open(QIODevice::ReadOnly) )
        return false;

    if( this->file.size() != D1TRN_TRANSLATIONS_BYTES )
        return false;

    for( int i = 0; i < D1TRN_TRANSLATIONS; i++ )
    {
        QByteArray translationByte = this->file.read( 1 );
        this->translations[i] = translationByte[0];
        this->resultingPalette->setColor(
            i, this->palette->getColor( this->translations[i] ) );
    }

    this->modified = false;

    return true;
}

bool D1Trn::save( QString trnFilePath )
{
    if( this->file.isOpen() )
        file.close();

    this->file.setFileName( trnFilePath );

    if( !this->file.open(QIODevice::ReadWrite) )
        return false;

    for( int i = 0; i < D1TRN_TRANSLATIONS; i++ )
    {
        QByteArray colorBytes;
        colorBytes.resize(1);
        colorBytes[0] = this->translations[i];

        if( this->file.write( colorBytes ) == -1 )
            return false;
    }

    if( !this->file.flush() )
        return false;

    if( this->file.size() != D1TRN_TRANSLATIONS_BYTES )
        return false;

    this->modified = false;

    return true;
}

bool D1Trn::isModified()
{
    return this->modified;
}

void D1Trn::refreshResultingPalette()
{
    for( int i = 0; i < D1TRN_TRANSLATIONS; i++ )
    {
        this->resultingPalette->setColor(
            i, this->palette->getColor( this->translations[i] ) );
    }
}

QColor D1Trn::getResultingColor( quint8 index )
{
    return this->resultingPalette->getColor( index );
}

QString D1Trn::getFilePath()
{
    if( this->file.isOpen() )
        return this->file.fileName();
    else
        return QString();
}

bool D1Trn::isFileOpen()
{
    return this->file.isOpen();
}

quint8 D1Trn::getTranslation( quint8 index )
{
    return this->translations[index];
}

void D1Trn::setTranslation( quint8 index, quint8 translation )
{
    this->translations[index] = translation;

    this->modified = true;
}

D1Pal* D1Trn::getPalette()
{
    return this->palette;
}

void D1Trn::setPalette( D1Pal* pal )
{
    this->palette = pal;
}

D1Pal* D1Trn::getResultingPalette()
{
    return this->resultingPalette;
}
