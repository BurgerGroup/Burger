<div align="center">

<img src="./doc/assets/logo.jpeg" width = "100" height = "80" alt="burger" align=center />

A C++ network library for high concurrent server in Linux 

![](https://img.shields.io/badge/release-v1.0-blue.svg)
![](https://img.shields.io/badge/build-passing-green.svg)
![](https://img.shields.io/badge/dependencies-up%20to%20date-green.svg)
![](https://img.shields.io/badge/license-MIT-blue.svg)

</div>

-----

## Features:
- The network module uses the Reactor scheme.
- Use more c++11 features like thread and chrono library 
- Wrapped mysql c api, providing more easy to use high-level api.
- Using C++ language development, shielding the underlying details, more user-friendly to write server programs.
- Wrapped spdlog api, more easy to use logger

## Compile and install:

```
$ sudo apt install g++ cmake make libboost-all-dev mysql-server libmysqlclient-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev 
 
$ git clone https://github.com/chanchann/Burger.git

$ cd Burger

$ mkdir build && cmake ..

$ make 

$ make install   
```

* Read [overview]() Why should we 
* Read [getting started]() 
* Docs:
  * [Performance benchmark todo]()


## reference 

[muduo](https://github.com/chenshuo/muduo)

[sylar](https://github.com/sylar-yin/sylar)

