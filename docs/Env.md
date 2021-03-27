## 环境变量

主要API : getenv / setenv

进程中的信息 

/proc/pid/ cmdline | cwd | exe

```
ps aux | grep xxx  得到进程号id1

ls -lh /proc/id1/  得到一大堆信息

利用/proc/pid/cmdline 和全局变量构造函数，实现在main函数前解析参数

1. 读写环境变量

2. 获取程序的绝对路径，给予绝对路径设置cwd

3. 可以通过cmdline，在进入main函数前，解析好参数

从而去把配置文件加载过来