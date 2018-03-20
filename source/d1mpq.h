#ifndef D1MPQ_H
#define D1MPQ_H

#include <QObject>
#include <QPointer>
#include <QFile>
#include <QBuffer>
#include <QMap>

class D1MpqHeader : public QObject
{
    Q_OBJECT

public:
    D1MpqHeader();
    ~D1MpqHeader();

protected:

};

class D1MpqBlockTable : public QObject
{
    Q_OBJECT

public:
    D1MpqBlockTable();
    ~D1MpqBlockTable();

protected:

};

class D1MpqHashTable : public QObject
{
    Q_OBJECT

public:
    D1MpqHashTable();
    ~D1MpqHashTable();

protected:

};

class D1Mpq : public QObject
{
    Q_OBJECT

public:
    D1Mpq();
    D1Mpq( QString );
    ~D1Mpq();

    virtual bool load( QString ) = 0;

protected:
    QFile file;

    QPointer<D1MpqHeader> header;
    QPointer<D1MpqBlockTable> blockTable;
    QPointer<D1MpqHashTable> hashTable;

/*
    QList< QPair<quint16,quint16> > groupFrameIndices;
    QList< QPair<quint32,quint32> > frameOffsets;
    QList< QPointer<D1CelFrameBase> > frames;
*/

};

#endif // D1MPQ_H
