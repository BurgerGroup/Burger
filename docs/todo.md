## todo stage 

1. burger v1.0 done 

2. 性能测试 todo 

3. 性能局部优化 todo

## TODO List 

1. string view(piece)

2. http 

3. websocket

4. 域名解析

5. logger 性能测试 done

6. threadpool性能测试 

7. 整体性能测试

8. threadpool优化  

9. logger参数调优，重新思考封装 

10. 协程 done

11. 读写锁？spinlock是否不需要?

12. 优化eventLoop中的pendingFunctors，可以用无锁队列  done

13. 优化EventLoop和EventLoopThread

14. 客户端挂掉会使服务器挂掉 done


## 思考 ：processor之间如何负载均衡

1. 想法1 :

```cpp
void Scheduler::addTask(const Coroutine::Callback& task, std::string name) {
    Processor* proc = pickOneProcesser();
    assert(proc != nullptr);
    proc->addPendingTask(task, name);
}
```
我们在这里pickOneProcesser(),

```cpp
Processor* Scheduler::pickOneProcesser() {
    std::lock_guard<std::mutex> lock(mutex_);
    if(mainProc_ == nullptr) {
        mainProc_ = util::make_unique<Processor>(this);
        workProcVec_.push_back(mainProc_.get());
        return mainProc_.get();
    }
    static size_t index = 0;
    assert(index < workProcVec_.size());
    Processor* proc = workProcVec_[index++];
    index = index % workProcVec_.size();
    return proc;
}
```
这里是用了round robin,我们可以在这里建造一个小根堆(根据load),向load小的加入task，但是这里会带来建堆的开销,但是这里本来processor数量就很少，这里开销也是常数级的

2. work steal -- 如何实现