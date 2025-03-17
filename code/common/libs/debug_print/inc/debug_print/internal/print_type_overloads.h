#ifndef LIBS_DEBUG_PRINT_TYPE_OVERLOADS_H
#define LIBS_DEBUG_PRINT_TYPE_OVERLOADS_H

#include <cstdint>
#include <type_traits>

/**
 * The template implementation calls these overloads depending on the current template arguments type
 *
 * Should a new type be supported, then an overload function for that type shall be added here
 */
namespace debug_print::internal {

void printType(char c);
void printType(const char* str);

void printType(unsigned int value);
void printType(uint8_t value);
void printType(uint16_t value);
void printType(uint32_t value);
void printType(uint64_t value);

void printType(int value);
void printType(int8_t value);
void printType(int16_t value);
void printType(int32_t value);
void printType(int64_t value);

void printType(float value);
void printType(double value);

void printType(bool value);

template <typename T>
void printType(const T* ptr) {
    // uint64t should be big enough for pc test build and pico build
    uint64_t pointer_address = reinterpret_cast<uint64_t>(ptr);
    printType(pointer_address);
}

// Concept to check if the type is not a pointer
template <typename T>
concept NonPointer = !std::is_pointer_v<T>;
// Fallback for unsupported types
template <NonPointer T>
void printType(const T& value) {
    printType("[Unsupported type?]");
}

/* TODO does not work, ideally could call the print (or would asString be better?) method from custom type
// Concept to check if a type has a `print` method
template <typename T>
concept HasPrintMethod = requires(T t) { t.print(); };

template <typename T>
void printType(const T& value)
    requires HasPrintMethod<T>
{
    value.print();
}
*/

}  // namespace debug_print::internal

#endif  // LIBS_DEBUG_PRINT_TYPE_OVERLOADS_H
