#include "Version.h"
namespace burger {
namespace http {

std::string httpVersionTostr(Version version) {
    switch(version) {
    case Version::kHttp10: 
        return "HTTP/1.0"; 
        break;
    case Version::kHttp11:
        return "HTTP/1.1";
        break;
    default:
        return "UNKNOWN";
        break;
    return "UNKNOWN";
    }
}

} // namespace http
} // namespace burger