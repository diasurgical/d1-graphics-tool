#ifndef D1PAL
#define D1PAL

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QColor>

#define D1PAL_COLORS 256
#define D1PAL_COLOR_BITS 8
#define D1PAL_SIZE_BYTES 768

enum class D1PAL_TYPE
{
    REGULAR,
    JASC
};

class D1Pal : public QObject
{
    Q_OBJECT

public:
    D1Pal();
    D1Pal( QString );
    ~D1Pal();

    bool load( QString );
    bool loadRegularPalette();
    bool loadJascPalette();
    bool save( QString );

    bool isModified();

    QString getFilePath();
    bool isFileOpen();

    QColor getColor( quint8 );
    void setColor( quint8, QColor );

private:
    D1PAL_TYPE type;
    bool modified;
    QFile file;
    QColor *colors;
};

#endif // D1PAL

