#ifndef COMMON_LIBS_UTILS_ENUMHELPERS_H
#define COMMON_LIBS_UTILS_ENUMHELPERS_H

#include <meta>
#include <string_view>
#include <type_traits>

namespace utils {

/**
 * @brief Returns the name of an enumerator as written in the source code.
 *
 *        Built on C++26 static reflection, so no hand written switch has to be kept in sync with
 *        the enumeration. The comparisons are generated at compile time, one per enumerator.
 *
 *        Enumerators that share a value (aliases such as FIRST_ = first_real_enumerator) resolve to
 *        whichever one is declared first.
 *
 * @tparam EnumT      The enumeration type, deduced from the argument.
 * @tparam Enumerable Whether the enumerators are visible here. Deduced, not meant to be passed.
 * @param value The enumerator to name.
 * @return The enumerator name, or "<UNKNOWN>" if the value matches no enumerator or the
 *         enumerators are not visible. The view is not guaranteed to be null terminated.
 */
template <typename EnumT, bool Enumerable = std::meta::is_enumerable_type(^^EnumT)>
    requires std::is_enum_v<EnumT>
constexpr std::string_view enumToString(EnumT value) {
    if constexpr (Enumerable)
        template for (constexpr auto enumerator : std::define_static_array(std::meta::enumerators_of(^^EnumT))) {
            if (value == [:enumerator:]) return std::meta::identifier_of(enumerator);
        }

    return "<UNKNOWN>";
}

/* TODO add something like this to for pc side which can use heap and would make it more
 * robust to showing something like static casting random value in runtime
template <typename EnumT>
    requires std::is_enum_v<EnumT>
std::string enumToStringFull(EnumT value)
{
    auto name = enumToString(value);

    if (name != "<unnamed>")
        return std::string{name};

    return std::string(std::meta::identifier_of(^^EnumT))
         + "::"
         + std::to_string(std::to_underlying(value));
}


template<typename E>
    requires std::is_enum_v<E>
std::string_view enum_to_string(E value, std::span<char> buffer)
{
    // named enumerator
    template for (constexpr auto e :
                  std::define_static_array(std::meta::enumerators_of(^^E))) {
        if (value == [:e:])
            return std::meta::identifier_of(e);
    }

    // Otherwise write:
    // Color::42
    // into buffer.
}
*/

}  // namespace utils

#endif  // COMMON_LIBS_UTILS_ENUMHELPERS_H
