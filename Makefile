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

run: build-debug
	@$(CC) examples/events.c -o examples/events -Llib -lswcl $(CLIBS) -DSWCL_ENABLE_DEBUG_LOGS
	@./examples/events
	@rm -f examples/events


clean:
	@rm -f *.o examples/events include lib

.PHONY: build test clean
