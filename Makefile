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

test: build
	@echo "Building tests"
	@$(CC) tests/*.c -o tests/test -Llib -lswcl $(CLIBS)
	@echo "Running tests"
	@./tests/test
	@echo "Done"


clean:
	@rm -f *.o tests/test include lib

.PHONY: build test clean
