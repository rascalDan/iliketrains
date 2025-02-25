#include "glMappedBufferWriter.h"
#include <iterator>

static_assert(std::weakly_incrementable<glMappedBufferWriter<int>>);
