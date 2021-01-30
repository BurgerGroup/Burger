#include "gtest/gtest.h"

int add(int a, int b){
    return a + b;
}

int sub(int a, int b){
    return a - b;
}

TEST(addTest, test_add) {
    EXPECT_EQ(add(1,2), 3);
}

TEST(subTest, test_sub) {
    EXPECT_EQ(sub(1,2), -1);
}

int main(int argc, char **argv){  
    std::cout << "run google test --> " << std::endl;
    testing::InitGoogleTest(&argc, argv);  
    return RUN_ALL_TESTS(); 
} 