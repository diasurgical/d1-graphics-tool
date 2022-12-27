#pragma once

#include <QFile>

#include "d1celbase.h"
#include "d1min.h"

class D1CelTileset : public D1CelBase {
public:
    D1CelTileset() = default;
    D1CelTileset(D1Min *);

    bool load(QString celFilePath, OpenAsParam *params = nullptr);
    bool save(SaveAsParam *params = nullptr);

private:
    bool writeFileData(QFile &outFile);

    D1Min *min;
};
