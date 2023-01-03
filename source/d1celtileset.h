#pragma once

#include <map>

#include <QFile>
#include <QString>

#include "d1celtilesetframe.h"
#include "d1gfx.h"
#include "openasdialog.h"
#include "saveasdialog.h"

class D1CelTileset {
public:
    static bool load(D1Gfx &gfx, std::map<unsigned, D1CEL_FRAME_TYPE> &celFrameTypes, QString celFilePath, OpenAsParam *params = nullptr);
    static bool save(D1Gfx &gfx, SaveAsParam *params = nullptr);

private:
    static bool writeFileData(D1Gfx &gfx, QFile &outFile);
};
