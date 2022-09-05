#pragma once

#include "d1cl2.h"

class D1ClxFrame : public D1Cl2Frame {
public:
    D1ClxFrame() = default;

    quint16 computeWidthFromHeader(QByteArray &) override;
};

class D1Clx : public D1Cl2 {
public:
    using D1Cl2::D1Cl2;

protected:
    D1ClxFrame *createFrame() override;
};
