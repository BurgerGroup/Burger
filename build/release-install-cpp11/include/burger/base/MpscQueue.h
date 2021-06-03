#ifndef MPSCQUEUE_H
#define MPSCQUEUE_H

#include <atomic>
#include <boost/noncopyable.hpp>
// https://stackoom.com/question/3hBmO/MPSC%E9%98%9F%E5%88%97-%E7%AB%9E%E8%B5%9B%E6%9D%A1%E4%BB%B6
// todo test 
namespace burger {
template <typename T>
class MpscQueue : boost::noncopyable {
public:
    MpscQueue();
    ~MpscQueue();
    void enqueue(T &&input);
    void enqueue(const T &input);
    bool dequeue(T &output);
    bool empty();
private:
    struct BufferNode {
        BufferNode() = default;
        BufferNode(const T &data) : dataPtr_(new T(data)) {}
        BufferNode(T &&data) : dataPtr_(new T(std::move(data))) {}
        T *dataPtr_;
        std::atomic<BufferNode *> next_{nullptr};
    };

    std::atomic<BufferNode *> head_;
    std::atomic<BufferNode *> tail_;
};
template<typename T>
MpscQueue<T>::MpscQueue() 
    : head_(new BufferNode), 
    tail_(head_.load(std::memory_order_relaxed)){
}

template<typename T>
MpscQueue<T>::~MpscQueue() {
    T output;
    while (this->dequeue(output)) {}
    BufferNode *front = head_.load(std::memory_order_relaxed);
    delete front;
}

template<typename T>
void MpscQueue<T>::enqueue(T &&input) {
    BufferNode *node{new BufferNode(std::move(input))};
    BufferNode *prevhead{head_.exchange(node, std::memory_order_acq_rel)};
    prevhead->next_.store(node, std::memory_order_release);
}

template<typename T>
void MpscQueue<T>::enqueue(const T &input) {
    BufferNode *node{new BufferNode(input)};
    BufferNode *prevhead{head_.exchange(node, std::memory_order_acq_rel)};
    prevhead->next_.store(node, std::memory_order_release);
}

template<typename T>
bool MpscQueue<T>::dequeue(T &output) {
    BufferNode *tail = tail_.load(std::memory_order_relaxed);
    BufferNode *next = tail->next_.load(std::memory_order_acquire);

    if (next == nullptr) {
        return false;
    }
    output = std::move(*(next->dataPtr_));
    delete next->dataPtr_;
    tail_.store(next, std::memory_order_release);
    delete tail;
    return true;
}

template<typename T>
bool MpscQueue<T>::empty() {
    BufferNode *tail = tail_.load(std::memory_order_relaxed);
    BufferNode *next = tail->next_.load(std::memory_order_acquire);
    return next == nullptr;
}

    
} // namespace burger



#endif // MPSCQUEUE_H