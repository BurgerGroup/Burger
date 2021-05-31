#include <vector>
#include <memory>
#include <iostream>


// g++ std=c++14
int main() {
    std::vector<std::unique_ptr<int> > vec;
    vec.push_back(std::make_unique<int>(1));
    vec.push_back(std::make_unique<int>(2));
    vec.push_back(std::make_unique<int>(3));
    for(auto& v : vec) {
        std::cout << *v << std::endl;
    }
}