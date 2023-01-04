#pragma once

#include <QFile>
#include <QString>

#include "d1gfx.h"
#include "openasdialog.h"
#include "saveasdialog.h"

class D1Cel {
public:
    static bool load(D1Gfx &gfx, QString celFilePath, const OpenAsParam &params);
    static bool save(D1Gfx &gfx, const SaveAsParam &params);

private:
    static bool writeFileData(D1Gfx &gfx, QFile &outFile, const SaveAsParam &params);
    static bool writeCompFileData(D1Gfx &gfx, QFile &outFile, const SaveAsParam &params);
};
