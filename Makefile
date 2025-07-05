CC=gcc

# Create single header file from source files
swcl.h: src/swcl.h src/protocols/xdg-shell-protocol.c src/protocols/xdg-shell-protocol.h
	@cp src/swcl.h swcl.h
	# Insert xdg-shell-protocol.c
	@$(CC) -fpreprocessed -dD -E -P src/protocols/xdg-shell-protocol.c > xdg-shell-protocol.c
	@sed -i "/#include \"protocols\/xdg-shell-protocol.c\"/r xdg-shell-protocol.c" swcl.h
	@sed -i "/#include \"protocols\/xdg-shell-protocol.c\"/d" swcl.h
	@rm -f xdg-shell-protocol.c
	# Insert xdg-shell-protocol.h
	@$(CC) -fpreprocessed -dD -E -P src/protocols/xdg-shell-protocol.h > xdg-shell-protocol.h
	@sed -i "/#include \"protocols\/xdg-shell-protocol.h\"/r xdg-shell-protocol.h" swcl.h
	@sed -i "/#include \"protocols\/xdg-shell-protocol.h\"/d" swcl.h
	@rm -f xdg-shell-protocol.h

regenerate-protocols:
	wayland-scanner client-header < /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml > src/xdg-shell-protocol.h
	wayland-scanner private-code < /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml > src/xdg-shell-protocol.c

clean:
	rm -f swcl.h xdg-shell-protocol.c xdg-shell-protocol.h

.PHONY: clean
