#!/usr/bin/env python3

import argparse
import os
import re
import urllib.request

CC = "gcc"
CLIBS = "-lwayland-client -lwayland-egl -lwayland-cursor -lGL -lEGL -lm"
CFLAGS = f"{CC} -O3"

def parse_args():
    parser = argparse.ArgumentParser(description='Build script for SWCL')
    parser.add_argument("-r", "--regenerate-protocols", action="store_true", help="Regenerate Wayland protocols files")
    parser.add_argument("-e", "--build-examples", action="store_true", help="Build examples in 'examples' directory")
    parser.add_argument("-c", "--clean", action="store_true", help="Cleanup build files")
    return parser.parse_args()


def cleanup_header(src: str)->str:
    # Remove CPP guards
    src = src.replace('#ifdef  __cplusplus\nextern "C" {\n#endif', "")
    src = src.replace('#ifdef  __cplusplus\n}\n#endif', "")
    # Remove comments
    single_line_comment_pattern = r'//.*?$'
    multi_line_comment_pattern = r'/\*.*?\*/'
    src = re.sub(single_line_comment_pattern, '', src, flags=re.MULTILINE)
    src = re.sub(multi_line_comment_pattern, '', src, flags=re.DOTALL)
    # Remove empty lines
    src = "".join([line for line in src.splitlines(True) if line.strip() != ''])
    return src

def build_header():
    print("Building 'swcl.h'")
    # Load files
    with open(os.path.join("src", "swcl.h"), "r") as f:
        swcl_h:str = f.read()
    with open(os.path.join("src", "xdg-shell-protocol.h"), "r") as f:
        xdg_shell_protocol_h = cleanup_header(f.read())
    with open(os.path.join("src", "xdg-shell-protocol.c"), "r") as f:
        xdg_shell_protocol_c = cleanup_header(f.read())
    # Remove dev defines
    swcl_h = swcl_h.replace("\n#define SWCL_IMPLEMENTATION // DEV\n", "")
    # Insert headers and source files
    swcl_h = swcl_h.replace('#include "xdg-shell-protocol.h"', xdg_shell_protocol_h)
    swcl_h = swcl_h.replace('#include "xdg-shell-protocol.c"', xdg_shell_protocol_c)
    # Save to file
    with open("swcl.h", "w") as f:
        f.write(swcl_h)
    print("Done")

def build_examples():
    print("Building examples")
    examples = ["basic-window", "csd", "events"]
    for example in examples:
        print(f"Building {example}")
        os.system(f"{CFLAGS} examples/{example}.c -o examples/{example} {CLIBS} -DSWCL_ENABLE_DEBUG_LOGS")
    print("Done")

def regenerate_protocols():
    print("Generating protocols")
    os.system("wayland-scanner client-header < /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml > src/xdg-shell-protocol.h")
    os.system("wayland-scanner private-code < /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml > src/xdg-shell-protocol.c")
    urllib.request.urlretrieve("https://gitlab.freedesktop.org/wlroots/wlr-protocols/-/raw/master/unstable/wlr-layer-shell-unstable-v1.xml", "wlr-layer-shell-unstable-v1.xml")
    os.system("wayland-scanner client-header < wlr-layer-shell-unstable-v1.xml > src/wlr-layer-shell-protocol.h")
    os.system("wayland-scanner private-code < wlr-layer-shell-unstable-v1.xml > src/wlr-layer-shell-protocol.c")
    os.remove("wlr-layer-shell-unstable-v1.xml")
    print("Done")

def clean():
    print("Running cleanup")
    clean_files = ["examples/basic-window", "examples/csd", "examples/events"]
    for file in clean_files:
        try:
            os.remove(file)
        except Exception:
            pass
    print("Done")


if __name__ == "__main__":
    args = parse_args()
    if args.build_examples:
        build_examples()
    elif args.clean:
        clean()
    elif args.regenerate_protocols:
        regenerate_protocols()
    else:
        build_header()
