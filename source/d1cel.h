#pragma once

#include <QFile>
#include <QString>

#include "d1gfx.h"
#include "openasdialog.h"
#include "saveasdialog.h"

class D1Cel {
public:
    static bool load(D1Gfx &gfx, QString celFilePath, OpenAsParam *params = nullptr);
    static bool save(D1Gfx &gfx, SaveAsParam *params = nullptr);

private:
    static bool writeFileData(D1Gfx &gfx, QFile &outFile, SaveAsParam *params);
    static bool writeCompFileData(D1Gfx &gfx, QFile &outFile, SaveAsParam *params);
};
