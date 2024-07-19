#include "../src/swcl.h"

int main(int argc, char const *argv[]) {
  SWCL_LOG("Testing Application");
  if (!swcl_application_new("io.github.mrvladus.Test"))
    SWCL_PANIC("Failed");
  SWCL_LOG("Application tests passed");
  return 0;
}
