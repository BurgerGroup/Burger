#include <yaml-cpp/yaml.h>


// todo 如何解决编译warning的问题
int main() {
    YAML::Node root = YAML::LoadFile("../config/test.yml");
}

