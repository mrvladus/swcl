CC=clang
CLIBS=-lwayland-client -lwayland-egl -lwayland-cursor -lGL -lEGL -lm

all: build

build:
	@echo "Building SWCL"
	@mkdir -p include include/swcl lib
	@cp src/swcl.h include/swcl/swcl.h
	@cp src/swcl-shapes.h include/swcl/swcl-shapes.h
	@$(CC) $(CFLAGS) -O3 -c src/*.c
	@ar rcs libswcl.a *.o
	@mv libswcl.a lib/libswcl.a
	@rm -f *.o
	@echo "Done"

build-debug:
	@echo "Building SWCL in debug mode"
	@mkdir -p include include/swcl lib
	@cp src/swcl.h include/swcl/swcl.h
	@cp src/swcl-shapes.h include/swcl/swcl-shapes.h
	@$(CC) $(CFLAGS) -c src/*.c -DSWCL_ENABLE_DEBUG_LOGS
	@ar rcs libswcl.a *.o
	@mv libswcl.a lib/libswcl.a
	@rm -f *.o
	@echo "Done"

build-examples: build-debug
	@rm -f examples/basic-window examples/csd examples/events
	@$(CC) $(CFLAGS) examples/basic-window.c -o examples/basic-window -Llib -lswcl $(CLIBS)
	@$(CC) $(CFLAGS) examples/csd.c -o examples/csd -Llib -lswcl $(CLIBS)
	@$(CC) $(CFLAGS) examples/events.c -o examples/events -Llib -lswcl $(CLIBS)

run: build-examples
	./examples/csd

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
	@rm -rf *.o examples/basic-window examples/csd examples/events include lib

.PHONY: build run regenerate-protocols clean
