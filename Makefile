CC=gcc
CLIBS=-lwayland-client -lwayland-egl -lwayland-cursor -lGL -lEGL -lm
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
	@echo "Building SWCL in debug mode"
	@mkdir -p include lib
	@cp src/swcl.h include/swcl.h
	@cp src/swcl-shapes.h include/swcl-shapes.h
	@$(CC) -c $(SOURCES) $(CLIBS) -DSWCL_ENABLE_DEBUG_LOGS
	@ar rcs libswcl.a *.o
	@mv libswcl.a lib/libswcl.a
	@rm -f *.o
	@echo "Done"

run: build-debug
	@$(CC) examples/rounded-corners.c -o examples/example -Llib -lswcl $(CLIBS)
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
	@rm -f *.o examples/events include lib

.PHONY: build run clean
