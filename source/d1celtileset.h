#pragma once

#include <QFile>
#include <QString>

#include "d1gfx.h"
#include "d1min.h"
#include "openasdialog.h"
#include "saveasdialog.h"

class D1CelTileset {
public:
    static bool load(D1Gfx &gfx, D1Min *min, QString celFilePath, OpenAsParam *params = nullptr);
    static bool save(D1Gfx &gfx, SaveAsParam *params = nullptr);

private:
    static bool writeFileData(D1Gfx &gfx, QFile &outFile);
};
