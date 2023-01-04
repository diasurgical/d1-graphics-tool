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
    static bool load(D1Gfx &gfx, std::map<unsigned, D1CEL_FRAME_TYPE> &celFrameTypes, QString celFilePath, const OpenAsParam &params);
    static bool save(D1Gfx &gfx, const SaveAsParam &params);

private:
    static bool writeFileData(D1Gfx &gfx, QFile &outFile);
};
