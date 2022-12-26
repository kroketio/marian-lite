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
