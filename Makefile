CC=gcc
CLIBS=-lwayland-client -lwayland-egl -lwayland-cursor -lGLESv2 -lEGL
SOURCES=src/*.c


build:
	@echo "Building SWCL"
	@mkdir -p include lib
	@cp src/swcl.h include/swcl.h
	@$(CC) -c $(SOURCES) $(CLIBS)
	@ar rcs libswcl.a *.o
	@mv libswcl.a lib/libswcl.a
	@rm -f *.o
	@echo "Done"

build-debug:
	@echo "Building SWCL"
	@mkdir -p include lib
	@cp src/swcl.h include/swcl.h
	@$(CC) -c $(SOURCES) $(CLIBS) -DSWCL_ENABLE_DEBUG_LOGS
	@ar rcs libswcl.a *.o
	@mv libswcl.a lib/libswcl.a
	@rm -f *.o
	@echo "Done"

test: build-debug
	@$(CC) tests/tests.c -o tests/tests -Llib -lswcl $(CLIBS) -DSWCL_ENABLE_DEBUG_LOGS
	@./tests/tests
	@rm -f tests/tests


clean:
	@rm -f *.o tests/test include lib

.PHONY: build test clean
