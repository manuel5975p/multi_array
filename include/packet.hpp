#ifndef PACKET_HPP
#define PACKET_HPP
#include <avx.hpp>
#include <numeric_concepts.hpp>
#include <array>
template<field T, size_t _N>
struct packet_struct{
    constexpr static size_t N = _N;
    using value_type = T;
    std::array<T, N> data;
    packet_struct(T* address){
        std::copy(address, address + N, data.begin());
    }
    packet_struct operator+(const packet_struct& o)const{
        packet_struct<T, N> ret;
        for(size_t i = 0;i < N;i++){
            ret.data[i] = data[i] + o.data[i];
        }
        return ret;
    }
    packet_struct& operator+=(const packet_struct& o){
        for(size_t i = 0;i < N;i++){
            data[i] += o.data[i];
        }
        return *this;
    }
    packet_struct operator-(const packet_struct& o)const{
        packet_struct<T, N> ret;
        for(size_t i = 0;i < N;i++){
            ret.data[i] = data[i] - o.data[i];
        }
        return ret;
    }
    packet_struct& operator-=(const packet_struct& o){
        for(size_t i = 0;i < N;i++){
            data[i] -= o.data[i];
        }
        return *this;
    }
    packet_struct operator*(const packet_struct& o)const{
        packet_struct<T, N> ret;
        for(size_t i = 0;i < N;i++){
            ret.data[i] = data[i] * o.data[i];
        }
        return ret;
    }
    packet_struct& operator*=(const packet_struct& o){
        for(size_t i = 0;i < N;i++){
            data[i] *= o.data[i];
        }
        return *this;
    }
    packet_struct operator/(const packet_struct& o)const{
        packet_struct<T, N> ret;
        for(size_t i = 0;i < N;i++){
            ret.data[i] = data[i] / o.data[i];
        }
        return ret;
    }
    packet_struct& operator/=(const packet_struct& o){
        for(size_t i = 0;i < N;i++){
            data[i] /= o.data[i];
        }
        return *this;
    }
    packet_struct operator-()const{
        packet_struct<T, N> ret;
        for(size_t i = 0;i < N;i++){
            ret.data[i] = -data[i];
        }
        return ret;
    }
    void storeu(T* address)const{
        std::copy(data.begin(), data.end(), address);
    }
    
    void loadu(const T* address){
        std::copy(address, address + N, data.begin());
    }

    void store(T* address)const{
        std::copy(data.begin(), data.end(), address);
    }
    
    void load(const T* address){
        std::copy(address, address + N, data.begin());
    }
};
template<typename T, size_t N>
struct packet_selector{ using type = packet_struct<T, N>;};
template<>
struct packet_selector<float, 8>{using type = vec8f;};
template<>
struct packet_selector<double, 4>{using type = vec4d;};
template<>
struct packet_selector<std::uint32_t, 8>{using type = vec8i;};
template<>
struct packet_selector<std::uint64_t, 4>{using type = vec4i;};
template<typename T, size_t N>
using packet = packet_selector<T, N>::type;
#endif