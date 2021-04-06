#include "burger/base/Env.h"
#include <iostream>
#include <algorithm>

using namespace burger;

// 在main函数前执行一个东西，用一个全局变量


int main(int argc, char* argv[]) {
    std::cout << "argc=" << argc << std::endl;
    EnvMgr::Instance().addHelp("s", "start with the terminal");
    EnvMgr::Instance().addHelp("d", "run as daemon");
    EnvMgr::Instance().addHelp("p", "print help");
    if(!EnvMgr::Instance().init(argc, argv)) {
        EnvMgr::Instance().printHelp();
        return 0;
    }
    // 我们只用写当前运行文件的当前目录的相对路径
    std::cout << "exe=" << EnvMgr::Instance().getExe() << std::endl;
    std::cout << "cwd=" << EnvMgr::Instance().getCwd() << std::endl;

    std::cout << "path=" << EnvMgr::Instance().getEnv("PATH", "xxx") << std::endl;

    std::cout << "test=" << EnvMgr::Instance().getEnv("TEST", "") << std::endl;
    std::cout << "set env " << EnvMgr::Instance().setEnv("TEST", "yy") << std::endl;
    std::cout << "test=" << EnvMgr::Instance().getEnv("TEST", "") << std::endl;
    if(EnvMgr::Instance().has("p")) {
        EnvMgr::Instance().printHelp();
    }
    std::string basePath = EnvMgr::Instance().getCwd();
    std::cout << basePath.find_last_of('/', basePath.size()-2) << std::endl;
    std::cout << basePath.substr(0, basePath.find_last_of('/', basePath.size()-2)) << std::endl;
    return 0;
}