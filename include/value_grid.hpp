#ifndef VALUE_GRID_HPP
#define VALUE_GRID_HPP
#include <multi_array.hpp>
#include <numeric_concepts.hpp>
#include <cxxabi.h>
#include <packet.hpp>
#include <string>
const inline std::string demangle(const char* name) {
    int status = -4;
    char* res = abi::__cxa_demangle(name, NULL, NULL, &status);
    const char* const demangled_name = (status==0)?res:name;
    std::string ret_val(demangled_name);
    std::free(res);
    return ret_val;
}
template<size_t n, size_t arg0, size_t... args>
struct access_nth{
    constexpr static size_t value = access_nth<n - 1, args...>::value;
};
template<size_t arg0, size_t... args>
struct access_nth<0, arg0, args...>{
    constexpr static size_t value = arg0;
};
template<typename T, typename V, size_t i, size_t n>
struct same_extents{
    constexpr static bool value = ((T::template static_extent<i>() == V::template static_extent<i>()) && (same_extents<T, V, i + 1, n>::value));
};
template<typename T, typename V, size_t n>
struct same_extents<T, V, n, n>{
    constexpr static bool value = true;
};
template<typename L, typename R>
struct addition_expression{
    constexpr static bool compile_time = L::compile_time || R::compile_time;
    constexpr static bool contiguous = L::contiguous && R::contiguous;
    using value_type = decltype(std::declval<typename L::value_type>() + std::declval<typename R::value_type>());
    const L* left;
    const R* right;
    static_assert(L::static_nDims() == R::static_nDims());
    using this_type = addition_expression<L, R>;
    addition_expression(const L* l, const R* r) : left(l), right(r){
        if constexpr(L::compile_time && R::compile_time){
            static_assert(same_extents<L, R, 0, this_type::static_nDims()>::value);
        }
        else{
            for(size_t i = 0;i < static_nDims();i++){
                assert(left->extent(i) == right->extent(i));
            }
        }
    }
    constexpr static size_t static_nDims(){
        return L::static_nDims();
    }
    template<size_t dim>
        requires compile_time
    constexpr static size_t static_extent(){
        return L::template static_extent<dim>();
    }
    template<size_t dim>
    constexpr size_t extent()const{
        return left->template extent<dim>();
    }
    constexpr size_t extent(size_t dim)const{
        return left->template extent(dim);
    }
    template<typename... Ts>
    auto operator()(Ts... args)const{
        return left->operator()(args...) + right->operator()(args...);
    }
    template<size_t N, typename... Ts>
    packet<value_type, N> chunk_access(Ts... args)const{
        return left->template chunk_access<N>(args...) + right->template chunk_access<N>(args...);
    }
};

#if DERIVE_TIMEDNESS_FROM_NUMBER_OF_TEMPLATE_ARGUMENTS
template<group T, size_t... extent_info>
#define _value_grid_tmplt_args T, extent_info...
#else
template<group T, bool _compile_time, size_t... extent_info>
#define _value_grid_tmplt_args T, _compile_time, extent_info...
#endif
struct value_grid{
    multi_array<_value_grid_tmplt_args> m_data;
    constexpr static bool compile_time = decltype(m_data)::compile_time;
    constexpr static bool contiguous = true;
    using value_type = T;
    using this_type = value_grid<_value_grid_tmplt_args>;
    static_assert(sizeof...(extent_info) >= 1, "Please give at least one dimension parameter");
    template<typename... Ts>
    value_grid(Ts... args) : m_data(args...){

    }
    value_grid(const this_type& arg) = default;
    value_grid(this_type&& arg) = default;
    template<size_t dim>
        requires compile_time
    constexpr size_t extent()const{
        return m_data.m_im.template extent<dim>();
    }
    template<size_t dim>
        requires (!compile_time)
    size_t extent()const{
        return m_data.m_im.template extent<dim>();
    }
    constexpr size_t extent(size_t dim)const{
        return m_data.m_im.extent(dim);
    }
    template<size_t dim>
        requires compile_time && (dim < sizeof...(extent_info))
    constexpr static size_t static_extent(){
        return access_nth<dim, extent_info...>::value;
    }
    constexpr size_t nDims()const{
        if constexpr(sizeof...(extent_info) > 1){
            return sizeof...(extent_info);
        }
        return (extent_info + ...);
    }
    constexpr static size_t static_nDims(){
        if constexpr(sizeof...(extent_info) > 1){
            return sizeof...(extent_info);
        }
        return (extent_info + ...);
    }
    template<size_t N, typename... Ts>
    packet<value_type, N> chunk_access(Ts... args)const{
        size_t mapped_index = m_data.m_im(args...);
        packet<value_type, N> pack;
        pack.loadu(const_cast<value_type*>(m_data.data() + mapped_index));
        return pack;
    }
    template<typename... Ts>
    T& operator()(Ts... args){
        return m_data(args...);
    }
    template<typename... Ts>
    const T& operator()(Ts... args)const{
        return m_data(args...);
    }
    auto operator[](size_t index){
        return m_data[index];
    }
    auto operator[](size_t index)const{
        return m_data[index];
    }
    template<typename otherT>
    value_grid& operator+=(const otherT& other){
        static_assert(this_type::static_nDims() == otherT::static_nDims());
        if constexpr(compile_time && otherT::compile_time){
            static_assert(same_extents<this_type, otherT, 0ul, this_type::static_nDims()>::value);
        }
        else{
            for(size_t i = 0;i < nDims();i++){
                assert(extent(i) == other.extent(i));
            }
        }
        m_data.m_im.enumerate_index_combinations([this, &other](std::integral auto... indices){
            std::tuple<decltype(indices)...> tup(indices...);
            this->operator()(indices...) += other(indices...);
        });
        return *this;
    }
    template<typename otherT>
    addition_expression<this_type, otherT> operator+(const otherT& other)const{
        return addition_expression<this_type, otherT>(this, &other);
    }
    template<typename otherT>
    value_grid& operator=(const otherT& other){
        if constexpr(contiguous){
            m_data.m_im.template enumerate_index_combinations_with_stride<8>(
                /*chunk */[this, &other](std::integral auto... args){
                    packet<value_type, 8> pack = other.template chunk_access<8>(args...);
                    pack.storeu(const_cast<value_type*>((this->m_data).data() + (this->m_data).m_im(args...)));
                },
                /*single*/[this, &other](std::integral auto... args){
                    this->operator()(args...) = other(args...);
                }
            );
        }
        else{
            m_data.m_im.enumerate_index_combinations([this, &other](std::integral auto... args){
                this->operator()(args...) = other(args...);
                static_assert((std::is_integral_v<decltype(args)> && ...));
            });
        }
        return *this;
    }
};
#endif