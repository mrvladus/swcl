#include "tests.h"
#include "test-all.c"
#include "test-application.c"

#include <stdint.h>
#include <stdio.h>

SWCLTestsResults test_results = {0, 0, 0};

void run_test(SWCLTestResult (*fn)(), const char *name) {
  printf("====================================\n");
  printf("TEST: %s\n", name);
  SWCLTestResult res = fn();
  test_results.total_passed += res.passed;
  test_results.total_failed += res.failed;
  test_results.total_run++;
  const char *msg;
  if (res.passed == 1)
    msg = "PASSED";
  else
    msg = "FAILED";
  printf("RESULT: %s\n", msg);
  printf("====================================\n");
}

uint8_t finish_testing() {
  printf("\n====================================\n");
  printf(
      "Finished running tests with results:\nTOTAL: %d\nPASSED: %d\nFAILED: %d",
      test_results.total_run, test_results.total_passed,
      test_results.total_failed);
  printf("\n====================================\n");
  return 1 ? test_results.total_failed > 0 : 0;
}

int main(int argc, char const *argv[]) {
  run_test(test_swcl_application_new, "swcl_application_new");
  run_test(test_swcl_application_get_cursor_position,
           "test_swcl_application_get_cursor_position");
  run_test(test_all, "test_all");
  return finish_testing();
}
