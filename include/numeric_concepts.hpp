#ifndef NUMERIC_CONCEPTS_HPP
#define NUMERIC_CONCEPTS_HPP
#include <concepts>
template<typename T>
concept addable = requires(T a, T b) {
    {a + b} -> std::convertible_to<T>;
    {a += b} -> std::convertible_to<T>;
};
template<typename T>
concept subtractable = requires(T a, T b) {
    {a - b} -> std::convertible_to<T>;
    {a -= b} -> std::convertible_to<T>;
};
template<typename T>
concept negatable = requires(T a) {
    {-a} -> std::convertible_to<T>;
};
template<typename T>
concept multiplyable = requires(T a, T b) {
    {a * b} -> std::same_as<T>;
    {a *= b} -> std::convertible_to<T>;
};
template<typename T>
concept divisible = requires(T a, T b) {
    {a / b} -> std::convertible_to<T>;
    {a /= b} -> std::convertible_to<T>;
};
template<typename T>
concept group = addable<T> && subtractable<T> && negatable<T>;
template<typename T>
concept ring = group<T> && multiplyable<T>;
template<typename T>
concept field = ring<T> && divisible<T>;
#endif