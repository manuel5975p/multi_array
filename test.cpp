#define DERIVE_TIMEDNESS_FROM_NUMBER_OF_TEMPLATE_ARGUMENTS 1
#define DEFAULT_ALLOCATOR aligned_allocator
#include <value_grid.hpp>
#include <Eigen/Dense>
void print_first(const multi_array<float, 3, 4, 5>& x){
    std::cout << x[0][0][0] << "\n";
}
int main(){
    multi_array <float, 3> x(3,4,5);
    multi_array <float, 3> y(10, 10, 20);
    
    for(size_t i = 0;i < x.m_im.extent<0>();i++){
        for(size_t j = 0;j < x.m_im.extent<1>();j++){
            for(size_t k = 0;k < x.m_im.extent<2>();k++){
                //std::cout << x(i,j,k) << ", ";
                x(i,j,k) = i + j + k;
            }
        }
    }
    x.m_im.enumerate_index_combinations([&x](std::array<size_t, 3> indices){std::cout << x(indices[0], indices[1], indices[2]) << ", ";});
    
    std::cout << "\n";
    //print_first(x);
    y[0][0][5] = 5;
    std::cout << y[0][0][5] << "\n";
    std::cout << x.nDims() << "\n";
    std::cout << reinterpret_cast<size_t>(x.data()) % 32 << "\n";
    multi_array <Eigen::Vector2d, 3> grid;
    
}