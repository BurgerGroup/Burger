#include "burger/base/Env.h"
#include <iostream>

using namespace burger;

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

}