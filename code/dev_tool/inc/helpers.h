#ifndef HELPERS_H
#define HELPERS_H
#include <QString>
#include <string_view>

namespace helpers {

QString intToHexString(int i);

inline QString stringViewToQString(std::string_view text) {
    return QString::fromUtf8(text.data(), static_cast<qsizetype>(text.size()));
}

}  // namespace helpers

#endif  // HELPERS_H
