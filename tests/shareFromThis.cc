#include <iostream>
#include <memory>
#include <cassert>

class Y: public std::enable_shared_from_this<Y> {
public:
    std::shared_ptr<Y> getPtr() {
        return shared_from_this();
    }
    Y* getRawPtr() { return this; }
};

int main() {
    std::shared_ptr<Y> p= std::make_shared<Y>();
    std::shared_ptr<Y> q = p->getPtr();
    
    Y* r = p->getRawPtr();
    assert(p == q);
    assert(p.get() == r);
    std::cout << p.use_count() << std::endl;
    // std::shared_ptr<Y> s(r);  // munmap_chunk(): invalid pointer -- 这里是创建了一个独立的shared_ptr，所以不是3， 裸指针的坑很多
    // std::cout << s.use_count() << std::endl;
    // assert(p == s);
    return 0;
}