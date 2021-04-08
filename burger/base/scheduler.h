#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <mutex>
#include <memory>
#include <vector>
namespace burger {

class Scheduler {
public:
    using ptr = std::shared_ptr<Scheduler>;
    Scheduler(size_t threads = 1, bool useCaller = true, const std::string& name = "");
    virtual ~Scheduler();
    const std::string& getName() const { return name_;}
    static Scheduler* GetThis();
    static Coroutine* GetMainFiber();

    void start();


    void stop();

    // threadId 协程执行的线程id,-1标识任意线程
    template<class CorouOrCb>
    void schedule(CorouOrCb fc, int threadId = -1) {
        bool need_tickle = false;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            needTickle = scheduleNoLock(fc, threadId);
        }

        if(needTickle) {
            tickle();
        }
    }

    /**
     * @brief 批量调度协程
     * @param[in] begin 协程数组的开始
     * @param[in] end 协程数组的结束
     */
    template<class InputIterator>
    void schedule(InputIterator begin, InputIterator end) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while(begin != end) {
                need_tickle = scheduleNoLock(&*begin, -1) || need_tickle;
                ++begin;
            }
        }
        if(need_tickle) {
            tickle();
        }
    }

    void switchTo(int thread = -1);
    std::ostream& dump(std::ostream& os);
protected:
    /**
     * @brief 通知协程调度器有任务了
     */

    virtual void tickle();
    /**
     * @brief 协程调度函数
     */
    void run();

    /**
     * @brief 返回是否可以停止
     */
    virtual bool stopping();

    /**
     * @brief 协程无任务可调度时执行idle协程
     */
    virtual void idle();

    /**
     * @brief 设置当前的协程调度器
     */
    void setThis();

    /**
     * @brief 是否有空闲线程
     */
    bool hasIdleThreads() { return m_idleThreadCount > 0;}
private:
    /**
     * @brief 协程调度启动(无锁)
     */
    template<class CorouOrCb>
    bool scheduleNoLock(CorouOrCb fc, int thread) {
        bool need_tickle = m_fibers.empty();
        FiberAndThread ft(fc, thread);
        if(ft.fiber || ft.cb) {
            m_fibers.push_back(ft);
        }
        return need_tickle;
    }
private:
    /**
     * @brief 协程/函数/线程组
     */
    struct CoroutineAndThread {
        /// 协程
        Coroutine::ptr fiber;
        /// 协程执行函数
        std::function<void()> cb;
        /// 线程id
        int thread;

        /**
         * @brief 构造函数
         * @param[in] f 协程
         * @param[in] thr 线程id
         */
        FiberAndThread(Coroutine::ptr f, int thr)
            :fiber(f), thread(thr) {
        }

        /**
         * @brief 构造函数
         * @param[in] f 协程指针
         * @param[in] thr 线程id
         * @post *f = nullptr
         */
        FiberAndThread(Coroutine::ptr* f, int thr)
            :thread(thr) {
            fiber.swap(*f);
        }

        /**
         * @brief 构造函数
         * @param[in] f 协程执行函数
         * @param[in] thr 线程id
         */
        FiberAndThread(std::function<void()> f, int thr)
            :cb(f), thread(thr) {
        }

        /**
         * @brief 构造函数
         * @param[in] f 协程执行函数指针
         * @param[in] thr 线程id
         * @post *f = nullptr
         */
        FiberAndThread(std::function<void()>* f, int thr)
            :thread(thr) {
            cb.swap(*f);
        }

        /**
         * @brief 无参构造函数
         */
        FiberAndThread()
            :thread(-1) {
        }

        /**
         * @brief 重置数据
         */
        void reset() {
            fiber = nullptr;
            cb = nullptr;
            thread = -1;
        }
    };
private:
    std::mutex mutex_;
    std::vector<std::thread> threadList_;
    /// 待执行的协程队列
    std::list<FiberAndThread> m_fibers;
    Coroutine::ptr m_rootFiber;  // use_caller为true时有效, 调度协程

    std::string name_;
protected:
    std::vector<int> threadIdList_;  // 协程下的线程id数组
    size_t threadCount_ = 0;
    std::atomic<size_t> activeThreadCount_ = {0};
    std::atomic<size_t> idleThreadCount_ = {0};
    bool stopping_ = true;  // 是否正在停止
    bool autoStop_ = false;  // 是否自动停止
    int rootThread_ = 0; // 主线程id(useCaller)
};

class SchedulerSwitcher : public Noncopyable {
public:
    SchedulerSwitcher(Scheduler* target = nullptr);
    ~SchedulerSwitcher();
private:
    Scheduler* m_caller;
};

} // namespace burger



#endif // SCHEDULER_H