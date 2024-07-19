CC=gcc
CLIBS=-lwayland-client -lwayland-egl -lwayland-cursor -lGLESv2 -lEGL
SOURCES=src/*.c

TEST_CLIBS=-Llib -lswcl $(CLIBS)
TESTS_BIN=tests/test_application

build:
	@echo "Building SWCL"
	@mkdir -p include lib
	@cp src/swcl.h include/swcl.h
	@$(CC) -c $(SOURCES) $(CLIBS)
	@ar rcs libswcl.a *.o
	@mv libswcl.a lib/libswcl.a
	@rm -f *.o
	@echo "Done"

test: build
	@echo "Building tests"
	@$(CC) tests/test_application.c -o tests/test_application $(TEST_CLIBS)
	@echo "Running tests"
	@./tests/test_application
	@echo "Done"
	@rm -f $(TESTS_BIN)


clean:
	@rm -f *.o tests/test include lib

.PHONY: build test clean
