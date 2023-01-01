#include "marian-lite/common/types.h"

namespace marian {

// this function calculates the amount of bytes needed to contain a tensor of given shape and type. 
// For most situation that is trivial (just number of elements time size of single element).
// But for instance, for intransparent types like packed tensors, it cannot easily be inferred by
// multiplying. All cases are handed here and can later be passed to allocators etc. 
size_t requiredBytes(const Shape& shape, Type type) {

  if (isIntgemm(type)) {
    /* Intgemm tensors have an extra float at the back that stores the quantization multiplier */
    return shape.elements() * sizeOf(type) + sizeOf(Type::float32);
  }
  return shape.elements() * sizeOf(type);

}

}