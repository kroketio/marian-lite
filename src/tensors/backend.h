#pragma once

#include "marian-lite/common/definitions.h"
#include "marian-lite/common/options.h"
#include "marian-lite/tensors/rand.h"

namespace marian {

class Backend {
protected:
  DeviceId deviceId_;
  size_t seed_;
  Ptr<RandomGenerator> randomGenerator_;

  // global clipping value for matrix-multiplies, should soon be removed.
  float clipValue_{0.f};

public:
  Backend(DeviceId deviceId, size_t seed)
      : deviceId_(deviceId), seed_(seed), randomGenerator_(createRandomGenerator(seed, deviceId)) {}
  virtual ~Backend() {};
  virtual DeviceId getDeviceId() { return deviceId_; };
  virtual Ptr<RandomGenerator> getRandomGenerator() { return randomGenerator_; }

  // for GPU only, calls cudaSetDevice, does nothing on CPU. Maybe change name.
  virtual void setDevice() = 0;
  virtual void configureDevice(Ptr<Options const> options) = 0;
  virtual void synchronize() = 0;

  virtual void setClip(float clipValue) { clipValue_ = clipValue; }
  float getClip() { return clipValue_; }

  // for CPU, sets to use optimized code for inference.
  // for GPU, this is invalid. for gpu, isOptimized() function always returns false.
  virtual void setInt16(bool optimize) = 0;
  virtual bool isInt16() = 0;
  virtual void setInt8(bool optimize) = 0;
  virtual bool isInt8() = 0;
  virtual void setShifted(bool shifted) = 0;
  virtual bool isShifted() = 0;
  virtual void setShiftedAll(bool shifted) = 0;
  virtual bool isShiftedAll() = 0;
  virtual void setDumpQuantMult(bool dump) = 0;
  virtual bool DumpQuantMult() = 0;
  virtual void setPrecomputedAlpha(bool alpha) = 0;
  virtual bool isPrecomputedAlpha() = 0;
  virtual void setLegacyBatchedGemm(bool legacyBatch) = 0;
  virtual bool isLegacyBatchedGemm() = 0;
};

Ptr<Backend> BackendByDeviceId(DeviceId deviceId, size_t seed);

}  // namespace marian
