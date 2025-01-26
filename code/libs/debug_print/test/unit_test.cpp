#include <gtest/gtest.h>

#include <climits>
#include <format>
#include <string>

#include "debug_print/debug_print.h"

std::string output;
void        putChar(char c) { output.push_back(c); }
void        resetOutput() { output = ""; }

// ################################## GENERAL ######################################
TEST(General, without_any_formatting) {
    resetOutput();
    std::string format_str = "asdassda";
    debug_print::printFormat(format_str.c_str());
    ASSERT_EQ(output, format_str);
}

TEST(General, without_argument) {
    resetOutput();
    std::string format_str = "test % string";
    std::string expected   = "test [No argument?] string";
    debug_print::printFormat(format_str.c_str());
    ASSERT_EQ(output, expected);
}

TEST(General, only_argument) {
    resetOutput();
    std::string expected = "c";
    debug_print::printFormat("%", 'c');
    ASSERT_EQ(output, expected);
}

TEST(General, null_termination_after_placeholder) {
    resetOutput();
    std::string expected = "c";
    debug_print::printFormat("%", 'c');
    ASSERT_EQ(output, expected);
}

TEST(General, auto_add_carriage_return) {
    resetOutput();
    std::string expected = "\r\n";
    debug_print::printFormat("\n");
    ASSERT_EQ(output, expected);
}

TEST(General, format_placeholder_char) {
    resetOutput();
    std::string expected = "%";
    debug_print::printFormat("%", '%');
    ASSERT_EQ(output, expected);
}
// #################################################################################

// ################################## STRING ######################################
TEST(String, format_string) {
    resetOutput();
    std::string expected = "test asd string";
    debug_print::printFormat("test % string", "asd");
    ASSERT_EQ(output, expected);
}
// #################################################################################

// ################################## Char ######################################
TEST(Char, format_char) {
    resetOutput();
    std::string expected = "test c string";
    debug_print::printFormat("test % string", 'c');
    ASSERT_EQ(output, expected);
}
// #################################################################################

// ################################## UINT #########################################
TEST(Unsigned_int, format_uint) {
    resetOutput();
    std::string expected = "test 420 string";
    debug_print::printFormat("test % string", 420);
    ASSERT_EQ(output, expected);
}

TEST(Unsigned_int, min_max) {
    resetOutput();
    std::string expected = "test " + std::to_string(std::numeric_limits<unsigned int>::max()) + " string";
    debug_print::printFormat("test % string", std::numeric_limits<unsigned int>::max());
    ASSERT_EQ(output, expected);

    resetOutput();
    expected = "test string " + std::to_string(std::numeric_limits<unsigned int>::min());
    debug_print::printFormat("test string %", std::numeric_limits<unsigned int>::min());
    ASSERT_EQ(output, expected);
}
// #################################################################################

// ################################## INT ##########################################
TEST(Signed_int, format_int) {
    resetOutput();
    std::string expected = "test -420 string";
    debug_print::printFormat("test % string", -420);
    ASSERT_EQ(output, expected);
}

TEST(Signed_int, min_max) {
    resetOutput();
    std::string expected = "test " + std::to_string(std::numeric_limits<int>::max()) + " string";
    debug_print::printFormat("test % string", std::numeric_limits<int>::max());
    ASSERT_EQ(output, expected);

    resetOutput();
    expected = "test string " + std::to_string(std::numeric_limits<int>::min());
    debug_print::printFormat("test string %", std::numeric_limits<int>::min());
    ASSERT_EQ(output, expected);
}
// ##################################################################################

// ################################## FLOAT #########################################
TEST(Floating_point, format_float) {
    resetOutput();
    std::string expected = "test 1.23456 string";
    debug_print::printFormat("test % string", 1.23456);
    ASSERT_EQ(output, expected);
}

TEST(Floating_point, format_negative_float) {
    resetOutput();
    std::string expected = "test -1.23456 string";
    debug_print::printFormat("test % string", -1.23456);
    ASSERT_EQ(output, expected);
}
// ##################################################################################

// ################################## POINTER ##########################################
TEST(Pointer, format_pointer) {
    resetOutput();
    int*        ptr      = reinterpret_cast<int*>(57005);
    std::string expected = "57005";
    debug_print::printFormat("%", ptr);
    ASSERT_EQ(output, expected);
}
// #####################################################################################

// ################################## BOOL ##########################################
TEST(Bool, format_bool_true) {
    resetOutput();
    debug_print::printFormat("%", true);
    ASSERT_EQ(output, "true");
}

TEST(Bool, format_bool_false) {
    resetOutput();
    debug_print::printFormat("%", false);
    ASSERT_EQ(output, "false");
}
// #####################################################################################

// ################################ CUSTOM TYPE ########################################
TEST(Custom_type, format_custom_type) {
    resetOutput();
    struct A {
        int a;
    } b;
    debug_print::printFormat("%", b);
    ASSERT_EQ(output, "[Unsupported type?]");
}
// #####################################################################################

// ############################## FORMAT OPTIONS #######################################
TEST(Format_options, uint_as_hex) {
    resetOutput();
    debug_print::printFormat("%x", 0xdeadbeef);
    ASSERT_EQ(output, "0xDEADBEEF");
}

TEST(Format_options, uint_as_binary) {
    resetOutput();
    debug_print::printFormat("%b", 0b1010);
    ASSERT_EQ(output, "0b1010");
}

TEST(Format_options, int_as_hex) {
    resetOutput();
    debug_print::printFormat("%x", -0x1234567);
    ASSERT_EQ(output, "-0x1234567");
}

TEST(Format_options, int_as_binary) {
    resetOutput();
    debug_print::printFormat("%b", -0b1011);
    ASSERT_EQ(output, "-0b1011");
}

TEST(Format_options, floating_point_decimal_precision_cut) {
    resetOutput();
    debug_print::printFormat("%3", 1.1234567);
    ASSERT_EQ(output, "1.123");
}

TEST(Format_options, floating_point_decimal_precision_add) {
    resetOutput();
    debug_print::printFormat("%5", 1.12300);
}

TEST(Format_options, ptr_as_hex) {
    resetOutput();
    int*        ptr      = reinterpret_cast<int*>(0xdead);
    std::string expected = "0xDEAD";
    debug_print::printFormat("%x", ptr);
    ASSERT_EQ(output, expected);
}
// #####################################################################################

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);

    debug_print::connectPutCharFunction(putChar);

    return RUN_ALL_TESTS();
}