#pragma once

#include <QFile>
#include <QString>

#include "d1gfx.h"
#include "openasdialog.h"
#include "saveasdialog.h"

class D1Cl2Frame {
    friend class D1Cl2;

public:
    static bool load(D1GfxFrame &frame, QByteArray rawFrameData, bool isClx = false, OpenAsParam *params = nullptr);

private:
    static quint16 computeWidthFromHeader(QByteArray &rawFrameData, bool isClx);
};

class D1Cl2 {
public:
    static bool load(D1Gfx &gfx, QString cl2FilePath, bool isClx = false, OpenAsParam *params = nullptr);
    static bool save(D1Gfx &gfx, bool isClx = false, SaveAsParam *params = nullptr);

protected:
    static bool writeFileData(D1Gfx &gfx, QFile &outFile, bool isClx, SaveAsParam *params);
};
