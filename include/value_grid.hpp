#ifndef VALUE_GRID_HPP
#define VALUE_GRID_HPP
#include <multi_array.hpp>
#include <numeric_concepts.hpp>
template<group T, size_t... extent_info>
struct value_grid{
    static_assert(sizeof...(extent_info) >= 1, "Please give at least one dimension parameter");
    multi_array<T, extent_info...> m_data;
    template<typename otherT>
    value_grid& operator+=(const otherT& other){
        return *this;
    }
};
#endif
