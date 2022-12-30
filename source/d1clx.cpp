#include "d1clx.h"

#include <QMessageBox>

quint16 D1ClxFrame::computeWidthFromHeader(QByteArray &rawFrameData)
{
    QDataStream in(rawFrameData);
    in.setByteOrder(QDataStream::LittleEndian);
    in.skipRawData(2);
    quint16 result;
    in >> result;
    in.skipRawData(6);
    return result;
}

D1ClxFrame *D1Clx::createFrame()
{
    return new D1ClxFrame;
}

bool D1Clx::writeFileData(QFile &outFile, SaveAsParam *params)
{
    QMessageBox::critical(nullptr, "Warning", "Not supported.");
    return false;
}
