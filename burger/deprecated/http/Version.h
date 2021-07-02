#ifndef VERSION_H
#define VERSION_H

#include <string>

namespace burger {
namespace http {
    
enum class Version {
    kUnknown, kHttp10, kHttp11
};

std::string httpVersionTostr(Version version);


} // namespace http

} // namespace burger



#endif // VERSION_H