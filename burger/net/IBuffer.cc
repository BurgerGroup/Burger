#include "IBuffer.h"

namespace burger{
namespace net{

const size_t IBuffer::kCheapPrepend = 8;
const size_t IBuffer::kInitialSize = 1024;
const char IBuffer::kCRLF[] = "\r\n";

}
}