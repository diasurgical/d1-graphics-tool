#pragma once

#include <QFile>
#include <QString>

#include "d1gfx.h"
#include "dialogs/openasdialog.h"

class D1Cel {
public:
    static bool load(D1Gfx &gfx, QString celFilePath, const OpenAsParam &params);
    static bool save(D1Gfx &gfx, const QString &gfxPath);

private:
    static bool writeFileData(D1Gfx &gfx, QFile &outFile);
    static bool writeCompFileData(D1Gfx &gfx, QFile &outFile);
};
