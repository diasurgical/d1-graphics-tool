#pragma once

#include "d1celbase.h"
#include "d1celframe.h"

class D1Cel : public D1CelBase {
public:
    D1Cel() = default;
    D1Cel(QString, D1Pal *);

    bool load(QString);
};
