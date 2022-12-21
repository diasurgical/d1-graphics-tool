#pragma once

#include "d1celbase.h"

class D1Cl2Frame : public D1CelFrameBase {
public:
    D1Cl2Frame() = default;

    virtual quint16 computeWidthFromHeader(QByteArray &);
    bool load(QByteArray rawData, OpenAsParam *params = nullptr);
};

class D1Cl2 : public D1CelBase {
public:
    D1Cl2();

    bool load(QString cl2FilePath, OpenAsParam *params = nullptr);

protected:
    virtual D1Cl2Frame *createFrame();
};
