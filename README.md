<div align="center">

<img src="./docs/assets/logo.jpeg" width = "100" height = "80" alt="burger" align=center />

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

## Config Module

A TCP connection timeout parameter is defined. 

You can directly use CONF.Get() to get the value of the parameter. 

todo : When the configuration is modified and reloaded, the value is automatically updated. The above configuration format is as follows:

```
[tcp]
connection.Timeout = 100
```

## Docs 

* Read [overview todo]() 
* Read [getting started todo]() 
* Docs:
  * [Performance benchmark todo]()

## Projects Based on Burger

- [BurgerChat](https://github.com/chanchann/BurgerChat) - 🍔 Console-based chat IM for Linux

## Maintainers

[@chanchann](https://github.com/chanchann).

[@skyu98](https://github.com/skyu98).

# Thanks

Thanks for [crow] [spdlog], [gtest], [sylar], [trantor] projects.

[Burger] is highly inspired by [muduo]. Thanks [Chen Shuo](https://github.com/chenshuo "https://github.com/chenshuo")