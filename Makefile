CC=clang
CFLAGS=-Wall -Wextra
CLIBS=-lwayland-client -lwayland-egl -lwayland-cursor -lGL -lEGL -lm
SOURCES=src/*.c

all: build

check:
	@[ -f /usr/include/wayland-client.h ] && echo "Found wayland-client.h" || echo "Not found wayland-client.h"
	@[ -f /usr/include/wayland-cursor.h ] && echo "Found wayland-cursor.h" || echo "Not found wayland-cursor.h"
	@[ -f /usr/include/wayland-egl.h ] && echo "Found wayland-egl.h" || echo "Not found wayland-egl.h"
	@[ -f /usr/include/GL/gl.h ] && echo "Found GL/gl.h" || echo "Not found GL/gl.h"
	@[ -f /usr/include/EGL/egl.h ] && echo "Found EGL/egl.h" || echo "Not found EGL/egl.h"

build: check
	@echo "Building SWCL"
	@mkdir -p include include/swcl lib
	@cp src/swcl.h include/swcl/swcl.h
	@cp src/swcl-shapes.h include/swcl/swcl-shapes.h
	@$(CC) -O3 -c $(SOURCES)
	@ar rcs libswcl.a *.o
	@mv libswcl.a lib/libswcl.a
	@rm -f *.o
	@echo "Done"

build-debug:
	@echo "Building SWCL in debug mode"
	@mkdir -p include include/swcl lib
	@cp src/swcl.h include/swcl/swcl.h
	@cp src/swcl-shapes.h include/swcl/swcl-shapes.h
	@$(CC) -c $(SOURCES) -DSWCL_ENABLE_DEBUG_LOGS -g
	@ar rcs libswcl.a *.o
	@mv libswcl.a lib/libswcl.a
	@rm -f *.o
	@echo "Done"

install: build

run: build-debug
	@$(CC) examples/csd.c -o examples/example -Llib -lswcl $(CLIBS)
	@./examples/example
	@rm -f examples/example

regenerate-protocols:
	@echo "Regenerating Wayland Protocols files"
	@wayland-scanner client-header < /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml > src/xdg-shell-protocol.h
	@wayland-scanner private-code < /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml > src/xdg-shell-protocol.c
	@curl https://gitlab.freedesktop.org/wlroots/wlr-protocols/-/raw/master/unstable/wlr-layer-shell-unstable-v1.xml -o wlr-layer-shell-unstable-v1.xml
	@wayland-scanner client-header < wlr-layer-shell-unstable-v1.xml > src/wlr-layer-shell-protocol.h
	@wayland-scanner private-code < wlr-layer-shell-unstable-v1.xml > src/wlr-layer-shell-protocol.c
	@rm -f wlr-layer-shell-unstable-v1.xml
	@echo "Done"

clean:
	@rm -rf *.o examples/example include lib

.PHONY: check prepare build run regenerate-protocols clean
