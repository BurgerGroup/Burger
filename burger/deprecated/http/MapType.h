#ifndef MAPTYPE_H
#define MAPTYPE_H

#include <map>
#include <string>
#include "burger/base/Util.h"


namespace burger {
namespace http {
    
using MapType = std::map<std::string, std::string, util::CaseInsensitiveLess>;

/**
 * @brief 获取Map中的key值,并转成对应类型,返回是否成功
 * @param[in] m Map数据结构
 * @param[in] key 关键字
 * @param[out] val 保存转换后的值
 * @param[in] def 默认值
 * @return
 *      @retval true 转换成功, val 为对应的值
 *      @retval false 不存在或者转换失败 val = def
 */
template<class MapType, class T>
bool checkGetAs(const MapType& m, const std::string& key, T& val, const T& def = T()) {
    auto it = m.find(key);
    if(it == m.end()) {
        val = def;
        return false;
    }
    try {
        // 虽然在c中可是使用类似于atoi之类的函数对字符串转换成整型，但是我们在这儿还是推荐使用这个函数
        // 如果转换发生了错误，lexical_cast会抛出一个bad_lexical_cast异常，因此程序中需要对其进行捕捉。
        val = boost::lexical_cast<T>(it->second);
        return true;
    } catch (...) {
        val = def;
    }
    return false;
}

/**
 * @brief 获取Map中的key值,并转成对应类型
 * @param[in] m Map数据结构
 * @param[in] key 关键字
 * @param[in] def 默认值
 * @return 如果存在且转换成功返回对应的值,否则返回默认值
 */
template<class MapType, class T>
T getAs(const MapType& m, const std::string& key, const T& def = T()) {
    auto it = m.find(key);
    if(it == m.end()) {
        return def;
    }
    try {
        return boost::lexical_cast<T>(it->second);
    } catch (...) {
    }
    return def;
}



} // namespace http

} // namespace burger
    
    
    
    

#endif // MAPTYPE_H