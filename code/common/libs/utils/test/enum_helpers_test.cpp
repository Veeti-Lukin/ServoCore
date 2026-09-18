#include <gtest/gtest.h>

#include <cstdint>
#include <string_view>

#include "utils/enum_helpers.h"

namespace {

enum class Color : uint8_t { red, green, blue };

enum class Sparse : int { low = -5, zero = 0, high = 1000 };

// Aliases share a value with an earlier enumerator, like ParameterValueType does
enum class WithAliases : uint8_t {
    first,
    second,
    third,

    FIRST_ITEM = first,
    LAST_ITEM  = third,
};

enum Unscoped : int { alpha, beta };

enum class Single { only_one };

enum class Opaque : int;  // declared, enumerators never defined

}  // namespace

// ################################## NAMES ########################################
TEST(EnumToString, names_scoped_enum) {
    ASSERT_EQ(utils::enumToString(Color::red), "red");
    ASSERT_EQ(utils::enumToString(Color::green), "green");
    ASSERT_EQ(utils::enumToString(Color::blue), "blue");
}

TEST(EnumToString, names_unscoped_enum) {
    ASSERT_EQ(utils::enumToString(alpha), "alpha");
    ASSERT_EQ(utils::enumToString(beta), "beta");
}

TEST(EnumToString, names_enumerator_with_single_enumerator) {
    ASSERT_EQ(utils::enumToString(Single::only_one), "only_one");
}

TEST(EnumToString, names_negative_and_sparse_values) {
    ASSERT_EQ(utils::enumToString(Sparse::low), "low");
    ASSERT_EQ(utils::enumToString(Sparse::zero), "zero");
    ASSERT_EQ(utils::enumToString(Sparse::high), "high");
}

// ################################## ALIASES ######################################
TEST(EnumToString, alias_resolves_to_the_first_declared_enumerator) {
    // FIRST_ITEM == first and LAST_ITEM == third, and the real names are declared first
    ASSERT_EQ(utils::enumToString(WithAliases::FIRST_ITEM), "first");
    ASSERT_EQ(utils::enumToString(WithAliases::LAST_ITEM), "third");
    ASSERT_EQ(utils::enumToString(WithAliases::second), "second");
}

// ################################## UNKNOWN ######################################
TEST(EnumToString, value_matching_no_enumerator_is_unknown) {
    ASSERT_EQ(utils::enumToString(static_cast<Color>(42)), "<UNKNOWN>");
    ASSERT_EQ(utils::enumToString(static_cast<Sparse>(-1)), "<UNKNOWN>");
}

TEST(EnumToString, enumeration_without_visible_enumerators_is_unknown) {
    ASSERT_EQ(utils::enumToString(static_cast<Opaque>(1)), "<UNKNOWN>");
}

// ################################## COMPILE TIME #################################
static_assert(utils::enumToString(Color::green) == "green");
static_assert(utils::enumToString(WithAliases::FIRST_ITEM) == "first");
static_assert(utils::enumToString(static_cast<Color>(42)) == "<UNKNOWN>");
static_assert(utils::enumToString(static_cast<Opaque>(1)) == "<UNKNOWN>");

TEST(EnumToString, usable_in_constant_expressions) {
    constexpr std::string_view name = utils::enumToString(Color::blue);
    ASSERT_EQ(name, "blue");
}

// ################################## RUNTIME ######################################
TEST(EnumToString, works_with_values_not_known_at_compile_time) {
    // Through a volatile variable so the compiler cannot fold the call away
    volatile int raw = 1;
    ASSERT_EQ(utils::enumToString(static_cast<Color>(raw)), "green");

    raw = 99;
    ASSERT_EQ(utils::enumToString(static_cast<Color>(raw)), "<UNKNOWN>");
}

TEST(EnumToString, returned_view_has_the_length_of_the_name) {
    ASSERT_EQ(utils::enumToString(Color::green).size(), std::string_view("green").size());
    ASSERT_EQ(utils::enumToString(Single::only_one).size(), std::string_view("only_one").size());
}
