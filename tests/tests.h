#ifndef SWCL_TESTS_H
#define SWCL_TESTS_H

#include <stdint.h>

typedef struct SWCLTestResult {
  uint8_t passed;
  uint8_t failed;
} SWCLTestResult;

typedef struct {
  uint8_t total_run;
  uint8_t total_passed;
  uint8_t total_failed;
} SWCLTestsResults;

#endif
