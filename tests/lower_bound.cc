// tiny demo for TimerQueue.cc: getExpiredList()

#include <set>
#include <iostream>
#include <vector>

int main() {
    using Entry = std::pair<int, bool>;
    using Set = std::set<Entry>;
    Set s;
    s.insert({1, true});
    s.insert({2, true});
    s.insert({3, false});
    s.insert({5, true});
    s.insert({7, false});
    for(const auto& a : s) {
        std::cout << a.first << " " << a.second << std::endl; 
    }
    std::cout << "lower bound " << std::endl;
    Entry e(3, true);
    auto mid = s.lower_bound(e);
    std::cout << mid->first << std::endl;  // 5 

    std::vector<Entry> expiredList;
    // https://stackoverflow.com/questions/41997285/using-stdcopy-with-stdback-inserter
    std::copy(s.begin(), mid, std::back_inserter(expiredList));
    s.erase(s.begin(), mid);
    std::cout << "expired" << std::endl;
    for(const auto& a : expiredList) {
        std::cout << a.first << " " << a.second << std::endl; 
    }
    std::cout << "set s" << std::endl;
    for(const auto& a : s) {
        std::cout << a.first << " " << a.second << std::endl; 
    }

}