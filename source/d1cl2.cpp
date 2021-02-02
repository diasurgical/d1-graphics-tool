#include "d1cl2.h"

D1Cl2Frame::D1Cl2Frame() :
    D1CelFrameBase()
{}

D1Cl2Frame::D1Cl2Frame( QByteArray rawData ) :
    D1CelFrameBase()
{
    this->load( rawData );
}

quint16 D1Cl2Frame::computeWidthFromHeader( QByteArray &rawFrameData )
{
    // Reading the frame header
    QDataStream in( rawFrameData );
    in.setByteOrder( QDataStream::LittleEndian );

    quint16 celFrameHeader[5];
    quint16 celFrameWidth[4] = { 0, 0, 0, 0 };
    quint16 pixelCount = 0;
    quint8 readByte = 0;

    // Read the {CEL FRAME HEADER}
    for( int i = 0; i < 5; i++ )
        in >> celFrameHeader[i];

    // Read the five 32 pixel-lines block to calculate the image width
    for( int i = 0; i < 4; i++ )
    {
        if( celFrameHeader[i+1] == 0 )
            break;

        for( int j = celFrameHeader[i]; j < celFrameHeader[i+1]; j++ )
        {
            readByte = rawFrameData[j];

            if( readByte > 0x00 && readByte < 0x80 )
            {
                pixelCount += readByte;
            }
            else if( readByte >= 0x80 && readByte < 0xBF )
            {
                pixelCount += ( 0xBF - readByte );
                j++;
            }
            else if( readByte >= 0xBF )
            {
                pixelCount += ( 256 - readByte );
                j += ( 256 - readByte );
            }
        }

        celFrameWidth[i] = pixelCount / 32;
        pixelCount = 0;
    }

    // The calculated width has to be the identical for each 32 pixel-line block
    // If it's not the case, 0 is returned
    for( int i = 0; i < 3; i++ )
    {
        if( celFrameWidth[i+1] != 0 && celFrameWidth[i] != celFrameWidth[i+1] )
            return 0;
    }

    return celFrameWidth[0];

    return 0;
}

bool D1Cl2Frame::load( QByteArray rawData )
{
    quint32 frameDataStartOffset = 0;
    quint8 readByte = 0;
    QList<D1CelPixel> pixelLine;

    if( rawData.size() == 0 )
        return false;

    // Checking the presence of the {CL2 FRAME HEADER}
    if( (quint8)rawData[0] == 0x0A && (quint8)rawData[1] == 0x00 )
    {
        frameDataStartOffset += 0x0A;
        // If header is present, try to compute frame width from frame header
        this->width = this->computeWidthFromHeader( rawData );
    }
    else
        return false;

    if( this->width == 0 )
        return false;

    // READ {CL2 FRAME DATA}

    pixelLine.clear();
    for( int o = frameDataStartOffset; o < rawData.size(); o++ )
    {
        readByte = rawData[o];

        // Transparent pixels
        if( readByte > 0x00 && readByte < 0x80 )
        {
            for( int i = 0; i < readByte; i++ )
            {
                // Add transparent pixel
                pixelLine.append( D1CelPixel(true,0) );
                if( pixelLine.size() == this->width )
                {
                    pixels.insert( 0, pixelLine );
                    pixelLine.clear();
                }
            }
        }
        // Repeat palette index
        else if( readByte >= 0x80 && readByte < 0xBF )
        {
            // Go to the palette index offset
            o++;

            for( int i = 0; i < (0xBF-readByte); i++ )
            {
                // Add opaque pixel
                pixelLine.append( D1CelPixel(false,rawData[o]) );
                if( pixelLine.size() == this->width )
                {
                    pixels.insert( 0, pixelLine );
                    pixelLine.clear();
                }
            }
        }
        // Palette indices
        else if( readByte >= 0xBF )
        {
            for( int i = 0; i < (256-readByte); i++ )
            {
                // Go to the next palette index offset
                o++;
                // Add opaque pixel
                pixelLine.append( D1CelPixel(false,rawData[o]) );
                if( pixelLine.size() == this->width )
                {
                    pixels.insert( 0, pixelLine );
                    pixelLine.clear();
                }
            }
        }
        else if( readByte == 0x00 )
        {
            qDebug("0x00 found");
        }

    }

    if( this->height == 0 )
        this->height = pixels.size();

    return true;
}

D1Cl2::D1Cl2() :
    D1CelBase()
{
    this->type = D1CEL_TYPE::V2_MULTIPLE_GROUPS;
}

D1Cl2::D1Cl2( QString path, D1Pal* pal ) :
    D1CelBase( pal )
{
    this->type = D1CEL_TYPE::V2_MULTIPLE_GROUPS;
    this->load( path );
}

bool D1Cl2::load( QString cl2FilePath )
{
    quint32 firstDword = 0;
    quint32 fileSizeDword = 0;
    quint32 lastCl2GroupHeaderOffset = 0;
    quint32 lastCl2GroupFrameCount = 0;

    quint32 cl2GroupOffset = 0;
    quint32 cl2GroupFrameCount = 0;
    quint32 cl2FrameStartOffset = 0;
    quint32 cl2FrameEndOffset = 0;

    quint32 cl2FrameSize = 0;

    QByteArray cl2FrameRawData;

    // Opening CL2 file with a QBuffer to load it in RAM
    if( !QFile::exists( cl2FilePath ) )
        return false;

    if( this->file.isOpen() )
        this->file.close();

    this->file.setFileName( cl2FilePath );

    if( !this->file.open(QIODevice::ReadOnly) )
        return false;

    QByteArray fileData = this->file.readAll();
    QBuffer fileBuffer( &fileData );

    if( !fileBuffer.open(QIODevice::ReadOnly) )
        return false;

    // Read CL2 binary data
    QDataStream in( &fileBuffer );
    in.setByteOrder( QDataStream::LittleEndian );

    // CL2 HEADER CHECKS

    // Read first DWORD
    in >> firstDword;

    // Trying to find file size in CL2 header
    if( fileBuffer.size() < (firstDword*4 + 4 + 4) )
        return false;

    fileBuffer.seek( firstDword*4 + 4 );
    in >> fileSizeDword;

    // If the dword is not equal to the file size then
    // check if it's a CL2 with multiple groups
    if( fileBuffer.size() != fileSizeDword )
    {
        // Read offset of the last CL2 group header
        fileBuffer.seek( firstDword - 4 );
        in >> lastCl2GroupHeaderOffset;

        // Read the number of frames of the last CL2 group
        if( fileBuffer.size() < lastCl2GroupHeaderOffset )
            return false;

        fileBuffer.seek( lastCl2GroupHeaderOffset );
        in >> lastCl2GroupFrameCount;

        // Read the last frame offset corresponding to the file size
        if( fileBuffer.size()
            < lastCl2GroupHeaderOffset + lastCl2GroupFrameCount*4 + 4 + 4 )
            return false;

        fileBuffer.seek( lastCl2GroupHeaderOffset + lastCl2GroupFrameCount*4 + 4 );
        in >> fileSizeDword;
        // The offset is from the beginning of the last group header
        // so we need to add the offset of the lasr group header
        // to have an offset from the beginning of the file
        fileSizeDword += lastCl2GroupHeaderOffset;

        if( fileBuffer.size() == fileSizeDword )
        {
            this->type = D1CEL_TYPE::V2_MULTIPLE_GROUPS;
            this->groupCount = firstDword/4;
        }
        else
            return false;
    }
    else
    {
        this->type = D1CEL_TYPE::V2_MONO_GROUP;
        this->groupCount = 1;
    }

    // CL2 FRAMES OFFSETS CALCULATION

    this->frameOffsets.clear();
    if( this->type == D1CEL_TYPE::V2_MULTIPLE_GROUPS )
    {
        // Going through all groups
        for( unsigned int i = 0; i*4 < firstDword; i++ )
        {
            fileBuffer.seek( i*4 );
            in >> cl2GroupOffset;

            fileBuffer.seek( cl2GroupOffset );
            in >> cl2GroupFrameCount;

            this->groupFrameIndices.append(
                qMakePair( this->frameOffsets.size(),
                    this->frameOffsets.size() + cl2GroupFrameCount - 1 ) );

            // Going through all frames of the group
            for( unsigned int j = 1; j <= cl2GroupFrameCount; j++ )
            {
                cl2FrameStartOffset = 0;
                cl2FrameEndOffset = 0;

                fileBuffer.seek( cl2GroupOffset + j*4 );
                in >> cl2FrameStartOffset;
                in >> cl2FrameEndOffset;

                this->frameOffsets.append(
                    qMakePair( cl2GroupOffset + cl2FrameStartOffset,
                               cl2GroupOffset + cl2FrameEndOffset ) );
            }
        }
    }
    else
    {
        // Going through all frames of the only group
        for( unsigned int i = 1; i <= firstDword; i++ )
        {
            cl2FrameStartOffset = 0;
            cl2FrameEndOffset = 0;

            fileBuffer.seek( i*4 );
            in >> cl2FrameStartOffset;
            in >> cl2FrameEndOffset;

            this->frameOffsets.append(
                qMakePair( cl2FrameStartOffset, cl2FrameEndOffset ) );

        }
    }

    if( !this->frameOffsets.empty() )
        this->frameCount = this->frameOffsets.size();
    else
        return false;

    // BUILDING {CL2 FRAMES}

    this->frames.clear();
    for( int i = 0; i < this->frameOffsets.size(); i++ )
    {
        cl2FrameSize = this->frameOffsets[i].second - this->frameOffsets[i].first;
        fileBuffer.seek( this->frameOffsets[i].first );
        cl2FrameRawData = fileBuffer.read( cl2FrameSize );

            this->frames.append(
                new D1Cl2Frame( cl2FrameRawData ) );
    }

    return true;
}
