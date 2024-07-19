#include "../src/swcl.h"
#include "tests.h"

SWCLTestResult test_swcl_application_new() {
  SWCLTestResult res = {0, 0};
  SWCL_LOG("Test: Create Application");
  if (!swcl_application_new("io.github.mrvladus.Test"))
    res.failed = 1;
  else
    res.passed = 1;

  return res;
}
