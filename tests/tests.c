#include "tests.h"
#include "test-application.c"
#include <stdio.h>

SWCLTestsResults test_results = {0, 0, 0};

void run_test(SWCLTestResult (*fn)()) {
  printf("====================================\n");
  printf("Run test: %d\n", ++test_results.total_run);
  SWCLTestResult res = fn();
  test_results.total_passed += res.passed;
  test_results.total_failed += res.failed;
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
  run_test(test_swcl_application_new);

  return finish_testing();
}
