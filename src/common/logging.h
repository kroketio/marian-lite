#pragma once

#include <iostream>

namespace marian {
  void logCallStack(size_t skipLevels);
  std::string getCallStack(size_t skipLevels);

  // Marian gives a basic exception guarantee. If you catch a
  // MarianRuntimeError you must assume that the object can be 
  // safely destructed, but cannot be used otherwise.

  // Internal multi-threading in exception-throwing mode is not 
  // allowed; and constructing a thread-pool will cause an exception.
  
  class MarianRuntimeException : public std::runtime_error {
  private:
    std::string callStack_;

  public:
    MarianRuntimeException(const std::string& message, const std::string& callStack) 
    : std::runtime_error(message), 
      callStack_(callStack) {}

    const char* getCallStack() const throw() {
      return callStack_.c_str();
    }
  };

  // Get the state of throwExceptionOnAbort (see logging.cpp), by default false
  bool getThrowExceptionOnAbort();

  // Set the state of throwExceptionOnAbort (see logging.cpp)
  void setThrowExceptionOnAbort(bool);
}


#ifdef __GNUC__
#define FUNCTION_NAME __PRETTY_FUNCTION__
#else
#ifdef _WIN32
#define FUNCTION_NAME __FUNCTION__
#else
#define FUNCTION_NAME "???"
#endif
#endif

/**
 * Prints critical error message and causes abnormal program termination by
 * calling std::abort().
 *
 * @param ... Message text and variables
 */
#define ABORT(...) std::abort();

#define ABORT_IF(condition, ...) \
  do {                           \
    if(condition) {              \
      ABORT(__VA_ARGS__);        \
    }                            \
  } while(0)

#define ABORT_UNLESS(condition, ...) \
  do {                               \
    if(!(bool)(condition)) {         \
      ABORT(__VA_ARGS__);            \
    }                                \
  } while(0)


namespace marian {
class Config;
}

template <class... Args>
void checkedLog(std::string logger, std::string level, Args... args) {

}

