#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <functional>

namespace burger {
namespace net {

// All client visible callbacks go here.
using TimerCallback = std::function<void()> ;

} // namespace net

} // namespace burger



#endif // CALLBACKS_H