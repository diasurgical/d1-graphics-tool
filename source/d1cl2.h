#pragma once

#include <QFile>
#include <QString>

#include "d1gfx.h"
#include "openasdialog.h"

class D1Cl2Frame {
    friend class D1Cl2;

public:
    static bool load(D1GfxFrame &frame, QByteArray rawFrameData, bool isClx, const OpenAsParam &params);

private:
    static quint16 computeWidthFromHeader(QByteArray &rawFrameData, bool isClx);
};

class D1Cl2 {
public:
    static bool load(D1Gfx &gfx, QString cl2FilePath, bool isClx, const OpenAsParam &params);
    static bool save(D1Gfx &gfx, bool isClx, const QString &gfxPath);

protected:
    static bool writeFileData(D1Gfx &gfx, QFile &outFile, bool isClx, const QString &gfxPath);
};
