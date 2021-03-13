#include "burger/base/Timestamp.h"
#include <iostream>
#include <vector>

using namespace burger;

void passbyValue(Timestamp x) {
    std::cout << "Pass by value : " 
        << x.toString() << " " << x.toFormatTime() << std::endl;
}

void passByconstReference(const Timestamp& time) {
    std::cout << "Pass by const ref : " 
        << time.toString() << " " << time.toFormatTime() << std::endl;
}


void benchmark() {
    const int kNumber = 1000 * 1000;
    std::vector<Timestamp> stamps;

    stamps.reserve(kNumber);
    for(int i = 0; i < kNumber; i++) {
        stamps.push_back(Timestamp::now());
    }
    std::cout << stamps.front().toString() << std::endl;
    std::cout << stamps.back().toString() << std::endl;
    std::cout << timeDifference(stamps.back(), stamps.front());

    std::vector<int> increments(100, 0);
    uint64_t start = stamps.front().microSecondsSinceEpoch();
    for (size_t i = 1; i < kNumber; ++i) {
        uint64_t next = stamps[i].microSecondsSinceEpoch();
        int64_t inc = next - start;
        start = next;
        if (inc < 0) {
            std::cout << "reverse!" << std::endl;
        }
        else if (inc < 100) {
            ++increments[inc];
        } else {
            std::cout << "big gap" << static_cast<int>(inc) << std::endl;
        }
    }
    for (int i = 0; i < 100; ++i) {
        std::cout << i <<  " : " << increments[i] <<std::endl;
    }
}


int main() {
    Timestamp now(Timestamp::now());  
    std::cout << now.microSecondsSinceEpoch() << std::endl; 
    std::cout << now.toString() << std::endl;
    std::cout << now.toFormatTime() << std::endl;
    passByconstReference(now);
    passbyValue(now);
    benchmark();
}