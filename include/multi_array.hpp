#ifndef MULTI_ARRAY_CORE_HPP
#define MULTI_ARRAY_CORE_HPP
#ifndef DERIVE_TIMEDNESS_FROM_NUMBER_OF_TEMPLATE_ARGUMENTS
#define DERIVE_TIMEDNESS_FROM_NUMBER_OF_TEMPLATE_ARGUMENTS 1
#endif
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <array>
#include <vector>
#include <cstdlib>
#include <type_traits>
#include <tuple>
#include <cassert>
#include <concepts>
template<typename T>
struct aligned_allocator{
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_move_assignment = std::false_type;
    T* allocate(size_type n){
        n *= sizeof(T);
        return (T*)std::aligned_alloc(32, n + ((32 - (n % 32)) % 32));
    }
    void deallocate(T* p, size_type n){
        (void)n;
        std::free(p);
    }
};
constexpr size_t Dynamic = ~0ULL;
template<typename T,typename allocator, size_t _size = Dynamic>
struct alignas(32) plain_vector{
    std::array<T, _size> m_data;
    using iterator = typename               std::array<T, _size>::iterator;
    using const_iterator = typename         std::array<T, _size>::const_iterator;
    using reverse_iterator = typename       std::array<T, _size>::reverse_iterator;
    using const_reverse_iterator = typename std::array<T, _size>::const_reverse_iterator;
    size_t size()const{return _size;}
    const T& operator[](size_t i)const{return m_data[i];}
    T& operator[](size_t i){return m_data[i];}
    auto begin       (){return m_data.begin  ();}
    auto end         (){return m_data.end    ();}
    auto rbegin      (){return m_data.rbegin ();}
    auto rend        (){return m_data.rend   ();}
    auto cbegin ()const{return m_data.cbegin ();}
    auto cend   ()const{return m_data.cend   ();}
    auto crbegin()const{return m_data.crbegin();}
    auto crend  ()const{return m_data.crend  ();}
    const T* data()const{return m_data.data  ();}
    T* data          (){return m_data.data  ();}
};
template<typename T, typename allocator>
struct plain_vector<T, allocator, Dynamic>{
    std::vector<T, allocator> m_data;
    using iterator = typename               std::vector<T>::iterator;
    using const_iterator = typename         std::vector<T>::const_iterator;
    using reverse_iterator = typename       std::vector<T>::reverse_iterator;
    using const_reverse_iterator = typename std::vector<T>::const_reverse_iterator;
    plain_vector(){}
    plain_vector(size_t size) : m_data(size){}
    plain_vector(size_t size, const T& fillvalue) : m_data(size, fillvalue){}
    size_t size()const{return m_data.size();}
    const T& operator[](size_t i)const{return m_data[i];}
    T& operator[](size_t i){return m_data[i];}
    auto begin       (){return m_data.begin  ();}
    auto end         (){return m_data.end    ();}
    auto rbegin      (){return m_data.rbegin ();}
    auto rend        (){return m_data.rend   ();}
    auto cbegin ()const{return m_data.cbegin ();}
    auto cend   ()const{return m_data.cend   ();}
    auto crbegin()const{return m_data.crbegin();}
    auto crend  ()const{return m_data.crend  ();}
    const T* data()const{return m_data.data  ();}
    T* data          (){return m_data.data  ();}
};
template<typename... Args>
constexpr auto product(Args... args) { 
    return (... * args); 
}
template<typename... Args>
constexpr auto sum(Args... args) { 
    return (... + args); 
}
template<size_t args0, size_t... argrest>
struct compiletime_index_mapper{
    template<typename T, typename... Ts>
    size_t operator()(T argument, Ts... arguments)const{
        if constexpr(sizeof...(arguments) > 0){
            static_assert(std::is_integral_v<T>);
            static_assert(std::conjunction_v<std::is_integral<Ts>...>);
            assert(argument < args0);
            return argument * product(argrest...) + compiletime_index_mapper<argrest...>()(arguments...);
        }
        else{
            return argument;
        }
    }
    size_t access_by_array(const std::array<size_t, sizeof...(argrest) + 1>& args)const{
        size_t index = 0;
        std::array<size_t, sizeof...(argrest) + 1> extents{args0, argrest...};
        size_t product = std::accumulate(extents.begin(), extents.end(), 1, [](size_t x, size_t y){return x * y;});
        auto ait = args.begin();
        for(auto it = extents.begin();it != extents.end();it++){
            index += *(ait++) * (product /= *it);
        }
        return index;
    }
    template<size_t dim>
    constexpr size_t extent()const{
        static_assert(dim < (sizeof...(argrest) + 1));
        return std::get<dim>(std::make_tuple(args0, argrest...));
    }
    constexpr size_t extent(size_t dim)const{
        return (std::array<size_t, sizeof...(argrest) + 1>{args0, argrest...})[dim];
    }
    size_t total_size()const{
        return args0 * (argrest * ...);
    }
    template<typename Function, size_t depth = 0, typename... Ts>
    void enumerate_index_combinations(Function f, Ts... args){
        if constexpr(depth < sizeof...(argrest) + 1){
            for(size_t i = 0;i < extent<depth>();i++){
                enumerate_index_combinations<Function, depth + 1>(f, args..., i);
            }
        }
        else{
            f(args...);
        }
    }
    template<std::invocable<std::array<size_t, sizeof...(argrest) + 1>> Function, size_t depth = 0, typename... Ts>
    void enumerate_index_combinations(Function f, Ts... args){
        if constexpr(depth < sizeof...(argrest) + 1){
            for(size_t i = 0;i < extent<depth>();i++){
                enumerate_index_combinations<Function, depth + 1>(f, args..., i);
            }
        }
        else{
            f(std::array<size_t, sizeof...(argrest) + 1>{args...});
        }
    }
    constexpr static size_t nDims(){
        return sizeof...(argrest) + 1;
    }
};
template<size_t dims>
struct alignas(32) runtime_index_mapper{
    std::array<size_t, dims> extents;
    std::array<size_t, dims - 1> products;
    
    template<typename... Ts>
    runtime_index_mapper(Ts... args) : extents{static_cast<size_t>(args)...}{
        static_assert(std::conjunction_v<std::is_integral<Ts>...>);
        assert(reinterpret_cast<size_t>(extents.data()) % 32 == 0);
        size_t product = std::accumulate(extents.begin(), extents.end(), 1, [](size_t x, size_t y){return x * y;});
        auto ait = products.begin();
        for(auto it = extents.begin();it != (extents.end() - 1);it++){
            *(ait++) = (product /= *it);
        }
    }
    runtime_index_mapper(const runtime_index_mapper<dims>&) = default;
    runtime_index_mapper(runtime_index_mapper<dims>&&) = default;
    runtime_index_mapper(const std::initializer_list<size_t>& _extents){
        assert(_extents.size() == dims);
        std::copy(_extents.begin(), _extents.end(), extents.begin());
    }
    template<typename... Ts>
    size_t operator()(Ts... arguments)const{
        static_assert(sizeof...(arguments) == dims);
        static_assert(std::conjunction_v<std::is_integral<Ts>...>);
        std::array<size_t, dims> args{static_cast<size_t>(arguments)...};
        /*size_t index = 0;
        size_t product = std::accumulate(extents.begin(), extents.end(), 1, [](size_t x, size_t y){return x * y;});
        auto ait = args.begin();
        for(auto it = extents.begin();it != extents.end();it++){
            index += *(ait++) * (product /= *it);
        }*/
        size_t alt_index = 0;
        auto pit = products.begin();
        auto ait = args.begin();
        for(auto it = extents.begin();it != (extents.end() - 1);it++){
            alt_index += *(ait++) * *(pit++);
        }
        alt_index += args.back();
        //assert(index == alt_index && "New index calculation is shit");
        return alt_index;
    }
    size_t access_by_array(const std::array<size_t, dims>& args)const{
        size_t index = 0;
        size_t product = std::accumulate(extents.begin(), extents.end(), 1, [](size_t x, size_t y){return x * y;});
        auto ait = args.begin();
        for(auto it = extents.begin();it != extents.end();it++){
            index += *(ait++) * (product /= *it);
        }
        return index;
    }
    size_t total_size()const{
        return std::accumulate(extents.begin(), extents.end(), 1, [](size_t x, size_t y){return x * y;});
    }
    template<size_t dim>
    size_t extent()const{
        return extents[dim];
    }
    size_t extent(size_t dim)const{
        return extents[dim];
    }
    template<typename Function, size_t depth = 0, typename... Ts>
    void enumerate_index_combinations(Function f, Ts... args){
        if constexpr(depth < dims){
            for(size_t i = 0;i < extent<depth>();i++){
                enumerate_index_combinations<Function, depth + 1>(f, args..., i);
            }
        }
        else{
            f(args...);
        }
    }
    template<std::invocable<std::array<size_t, dims>> Function, size_t depth = 0, typename... Ts>
    void enumerate_index_combinations(Function f, Ts... args){
        if constexpr(depth < dims){
            for(size_t i = 0;i < extent<depth>();i++){
                enumerate_index_combinations<Function, depth + 1>(f, args..., i);
            }
        }
        else{
            f(std::array<size_t, dims>{args...});
        }
    }
    constexpr static size_t nDims(){
        return dims;
    }
};
template<typename marray, size_t codim, size_t dim>
struct access_ref{
    marray* accessor;
    std::array<size_t, codim> indices;
    access_ref<marray, codim + 1, dim - 1> operator[](size_t index){
        access_ref<marray, codim + 1, dim - 1> ret;
        std::copy(indices.begin(), indices.end(), ret.indices.begin());
        ret.indices.back() = index;
        ret.accessor = accessor;
        return ret;
    }
};
template<typename marray, size_t codim>
struct access_ref<marray ,codim, 1>{
    marray* accessor;
    std::array<size_t, codim> indices;
    marray::value_type& operator[](size_t index){
        std::array<size_t, codim + 1> accindices;
        std::copy(indices.begin(), indices.end(), accindices.begin());
        accindices.back() = index;
        return accessor->access_by_array(accindices);
    }
};
template<typename marray, size_t codim, size_t dim>
struct const_access_ref{
    const marray* accessor;
    std::array<size_t, codim> indices;
    const_access_ref<marray, codim + 1, dim - 1> operator[](size_t index){
        const_access_ref<marray, codim + 1, dim - 1> ret;
        std::copy(indices.begin(), indices.end(), ret.indices.begin());
        ret.indices.back() = index;
        ret.accessor = accessor;
        return ret;
    }
};
template<typename marray, size_t codim>
struct const_access_ref<marray ,codim, 1>{
    const marray* accessor;
    std::array<size_t, codim> indices;
    const marray::value_type& operator[](size_t index){
        std::array<size_t, codim + 1> accindices;
        std::copy(indices.begin(), indices.end(), accindices.begin());
        accindices.back() = index;
        return accessor->access_by_array(accindices);
    }
};
template<size_t s1, size_t... sr>
struct index_mapper_type_finder{using type = compiletime_index_mapper<s1, sr...>;};
template<size_t s1>
struct index_mapper_type_finder<s1>{using type = runtime_index_mapper<s1>;};
#if DERIVE_TIMEDNESS_FROM_NUMBER_OF_TEMPLATE_ARGUMENTS
template<typename T, typename allocator, size_t... number_of_extends_or_extent_list>
struct multi_array_impl{
    static_assert(sizeof...(number_of_extends_or_extent_list) >= 1);
    constexpr static bool compile_time = (sizeof...(number_of_extends_or_extent_list) > 1);
#else
template<typename T,typename allocator, bool compile_time, size_t... number_of_extends_or_extent_list>
struct multi_array_impl{
    static_assert(compile_time || (sizeof...(number_of_extends_or_extent_list) == 1));
#endif
    using value_type = T;
    typename index_mapper_type_finder<number_of_extends_or_extent_list...>::type m_im;
    constexpr static size_t m_dims = decltype(m_im)::nDims();
    plain_vector<T, allocator, compile_time ? product(number_of_extends_or_extent_list...) : ~0ull> m_data;
    template<typename... Ts>
    multi_array_impl(Ts... args) : m_im(args...){
        if constexpr (sizeof...(args) == 0);
        else if constexpr(!compile_time) m_data = decltype(m_data)(product(args...));
    }
    #if DERIVE_TIMEDNESS_FROM_NUMBER_OF_TEMPLATE_ARGUMENTS
    multi_array_impl(const multi_array_impl<T, allocator, number_of_extends_or_extent_list...>& other) = default;
    multi_array_impl(multi_array_impl<T, allocator, number_of_extends_or_extent_list...>&& other) = default;
    #else
    multi_array_impl(const multi_array_impl<T, allocator, compile_time, number_of_extends_or_extent_list...>& other) = default;
    multi_array_impl(multi_array_impl<T, allocator, compile_time, number_of_extends_or_extent_list...>&& other) = default;
    #endif
    template<typename... Ts>
    const T& operator()(Ts... args)const{
        return m_data[m_im(args...)];
    }
    template<typename... Ts>
    T& operator()(Ts... args){
        return m_data[m_im(args...)];
    }
    const T& access_by_array(const std::array<size_t, m_dims>& arr)const{
        return m_data[m_im.access_by_array(arr)];
    }
    T& access_by_array(const std::array<size_t, m_dims>& arr){
        return m_data[m_im.access_by_array(arr)];
    }
    constexpr size_t nDims()const{
        return m_dims;
    }
    const value_type* data()const{
        return m_data.data();
    }
    value_type* data(){
        return m_data.data();
    }
    std::conditional_t<m_dims == 1,T&, access_ref<multi_array_impl<T, allocator, number_of_extends_or_extent_list...>, 1, m_dims - 1>> operator[](size_t index){
        if constexpr(m_dims == 1){
            return (m_data[index]);
        }
        else{
            access_ref<multi_array_impl<T, allocator, number_of_extends_or_extent_list...>, 1, m_dims - 1> ret;
            ret.indices[0] = index;
            ret.accessor = this;
            return ret;
        }
    }
    std::conditional_t<m_dims == 1,const T&, const_access_ref<multi_array_impl<T, allocator, number_of_extends_or_extent_list...>, 1, m_dims - 1>> operator[](size_t index)const{
        if constexpr(m_dims == 1){
            return (m_data[index]);
        }
        else{
            const_access_ref<multi_array_impl<T, allocator, number_of_extends_or_extent_list...>, 1, m_dims - 1> ret;
            ret.indices[0] = index;
            ret.accessor = this;
            return ret;
        }
    }
    
};
#ifndef DEFAULT_ALLOCATOR
#define DEFAULT_ALLOCATOR aligned_allocator
#endif
#if DERIVE_TIMEDNESS_FROM_NUMBER_OF_TEMPLATE_ARGUMENTS
template<typename T, size_t... sizes>
struct multi_array_selector{
    using type = multi_array_impl<T, DEFAULT_ALLOCATOR<T>, sizes...>;
};
template<typename T, size_t R>
struct multi_array_selector<T, R>{
    using type = multi_array_impl<T, DEFAULT_ALLOCATOR<T>, R>;
};
template<typename T, size_t... sizes>
using multi_array = multi_array_selector<T, sizes...>::type;
#else
template<typename T, bool compiletime, size_t... sizes>
struct multi_array_selector{
    using type = multi_array_impl<T, DEFAULT_ALLOCATOR<T>, compiletime, sizes...>;
};
template<typename T, bool compiletime, size_t R>
struct multi_array_selector<T, compiletime, R>{
    using type = multi_array_impl<T,DEFAULT_ALLOCATOR<T>, compiletime,  R>;
};
template<typename T, bool compiletime, size_t... sizes>
using multi_array = multi_array_selector<T, compiletime, sizes...>::type;
#endif
#endif