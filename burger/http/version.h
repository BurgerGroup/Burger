#ifndef VERSION_H
#define VERSION_H

namespace burger {
namespace http {
    
enum class Version {
    kUnknown, kHttp10, kHttp11
};
// std::string versionTostr();

} // namespace http

} // namespace burger



#endif // VERSION_H