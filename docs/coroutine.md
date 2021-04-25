## 协程模块

```
Coroutine::GetCurCo()
Thread -> main_co <-----------> sub_co
            |
            |
          sub_co

main_co 负责切换，回收, 不分配栈空间
```

协程调度模块scheduler

如何让协程在线程间切换

```
    1    --     N      1 -- M
scheduler --> thread --> co

        N  - --------  M 
1. 线程池，分配一组线程
2. 协程调度器，将协程，指定到相应的线程上去执行

```