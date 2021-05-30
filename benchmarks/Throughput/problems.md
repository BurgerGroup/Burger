## 遇到问题: docker里1w个连接端口耗尽

在我们的docker 环境中

当连接数较大上万，errno = 32, errno = 104

查看连接数

netstat -nat|grep -i "8888"|wc -l 

查看连接状况

netstat -nat | grep 端口号

```
cat /proc/sys/net/ipv4/ip_local_port_range
32768	60999
```
todo : 为何我们1w多个连接就出现问题，而此处应该一样28000个端口可用

https://blog.csdn.net/u010585120/article/details/80826999

## 单机tcp最大连接数是多少

https://www.huaweicloud.com/articles/afb0a117242fab283b3d5f9e9e3e10b2.html

## 地址重用（unix的SO_REUSEADDR选项）

