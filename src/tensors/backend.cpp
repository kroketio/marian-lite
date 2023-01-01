#include "marian-lite/tensors/backend.h"

#ifdef CUDA_FOUND
#include "marian-lite/tensors/gpu/backend.h"
#pragma warning(disable:4505) // "unreferenced local function has been removed" in cuda\v9.2\include\cuda_fp16.hpp
#endif

#include "marian-lite/tensors/cpu/backend.h"

namespace marian {

Ptr<Backend> BackendByDeviceId(DeviceId deviceId, size_t seed) {
#ifdef CUDA_FOUND
  if(deviceId.type == DeviceType::gpu)
    return New<gpu::Backend>(deviceId, seed);
  else
#endif
    return New<cpu::Backend>(deviceId, seed);
}
}  // namespace marian
