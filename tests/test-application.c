#include "../src/swcl.h"
#include "tests.h"

SWCLTestResult test_swcl_application_new() {
  SWCLTestResult res = {0, 0};
  if (!swcl_application_new("io.github.mrvladus.Test"))
    res.failed = 1;
  else
    res.passed = 1;
  return res;
}

SWCLTestResult test_swcl_application_get_cursor_position() {
  SWCLTestResult res = {0, 0};
  SWCLApplication *app = swcl_application_new("io.github.mrvladus.Test");
  SWCLPoint point = swcl_application_get_cursor_position(app);
  if (point.x != 0 || point.y != 0)
    res.failed = 1;
  else
    res.passed = 1;
  return res;
}
