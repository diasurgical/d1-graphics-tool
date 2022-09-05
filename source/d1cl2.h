#pragma once

#include "d1celbase.h"

class D1Cl2Frame : public D1CelFrameBase {
public:
    D1Cl2Frame() = default;

    quint16 computeWidthFromHeader(QByteArray &) override;
    bool load(QByteArray);
};

class D1Cl2 : public D1CelBase {
public:
    D1Cl2();
    D1Cl2(QString, D1Pal *);

    bool load(QString);

protected:
    virtual D1Cl2Frame *createFrame();
};
