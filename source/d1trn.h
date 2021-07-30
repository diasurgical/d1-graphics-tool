#ifndef D1TRN
#define D1TRN

#include <QPointer>
#include "d1pal.h"

#define D1TRN_TRANSLATIONS 256
#define D1TRN_TRANSLATIONS_BYTES 256

class D1Trn : public QObject
{
    Q_OBJECT

public:
    D1Trn();
    D1Trn( QString, D1Pal* );
    ~D1Trn();

    bool load( QString );
    bool save();
    bool save( QString );

    void refreshResultingPalette();
    QColor getResultingColor( quint8 );

    QString getFilePath();
    bool isFileOpen();
    quint8 getTranslation( quint8 );
    void setTranslation( quint8, quint8 );
    D1Pal* getPalette();
    void setPalette( D1Pal* );
    D1Pal* getResultingPalette();

private:
    QFile file;
    quint8 *translations;
    QPointer<D1Pal> palette;
    QPointer<D1Pal> resultingPalette;
};

#endif // D1TRN

