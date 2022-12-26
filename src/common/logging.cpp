#include "logging.h"
#include "common/config.h"
#include "3rd_party/ExceptionWithCallStack.h"

#include <ctime>
#include <cstdlib>
#ifdef __unix__
#include <csignal>
#endif

#ifdef _MSC_VER
#define noinline __declspec(noinline)
#else
#define noinline __attribute__((noinline))
#endif

namespace marian {
  static bool throwExceptionOnAbort = false;
  bool getThrowExceptionOnAbort() { return throwExceptionOnAbort; }
  void setThrowExceptionOnAbort(bool doThrowExceptionOnAbort) { throwExceptionOnAbort = doThrowExceptionOnAbort; };
}

namespace marian {
  std::string noinline getCallStack(size_t skipLevels) {
  #ifdef WASM_COMPATIBLE_SOURCE
    return "Callstacks not supported in WASM builds currently";
  #else
    return ::Microsoft::MSR::CNTK::DebugUtil::GetCallStack(skipLevels + 2, /*makeFunctionNamesStandOut=*/true);
  #endif
  }

  void noinline logCallStack(size_t skipLevels) {
    checkedLog("general", "critical", getCallStack(skipLevels));
  }
}
