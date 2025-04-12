#include "helpers.h"

namespace helpers {

QString intToHexString(int i) {
    if (i >= 0) return QString("0x%1").arg(i, 0, 16, QLatin1Char('0'));
    return QString("-0x%1").arg(i, 0, 16, QLatin1Char('0'));
};

}  // namespace helpers