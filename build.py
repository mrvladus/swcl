#!/usr/bin/env python3

import os
import re

def remove_empty_lines_and_comments(src: str)->str:
    # Remove comments
    single_line_comment_pattern = r'//.*?$'
    multi_line_comment_pattern = r'/\*.*?\*/'
    src = re.sub(single_line_comment_pattern, '', src, flags=re.MULTILINE)
    src = re.sub(multi_line_comment_pattern, '', src, flags=re.DOTALL)
    # Remove empty lines
    src = "".join([line for line in src.splitlines(True) if line.strip() != ''])
    return src

def build_header():
    # Load files
    with open(os.path.join("src", "swcl.h"), "r") as f:
        swcl_h:str = f.read()
    with open(os.path.join("src", "xdg-shell-protocol.h"), "r") as f:
        xdg_shell_protocol_h = remove_empty_lines_and_comments(f.read())
    with open(os.path.join("src", "xdg-shell-protocol.c"), "r") as f:
        xdg_shell_protocol_c = remove_empty_lines_and_comments(f.read())
    # Comment out dev defines
    swcl_h = swcl_h.replace("\n#define SWCL_IMPLEMENTATION // DEV\n", "")
    # Insert headers and source files
    swcl_h = swcl_h.replace('#include "xdg-shell-protocol.h"', xdg_shell_protocol_h)
    swcl_h = swcl_h.replace('#include "xdg-shell-protocol.c"', xdg_shell_protocol_c)
    # Save to file
    with open("swcl.h", "w") as f:
        f.write(swcl_h)

def build_examples():
    CLIBS = "-lwayland-client -lwayland-egl -lwayland-cursor -lGL -lEGL -lm"
    os.system(f"gcc -O3 examples/basic-window.c -o examples/basic-window {CLIBS} -DSWCL_ENABLE_DEBUG_LOGS")
    os.system(f"gcc -O3 examples/csd.c -o examples/csd {CLIBS} -DSWCL_ENABLE_DEBUG_LOGS")
    os.system(f"gcc -O3 examples/events.c -o examples/events {CLIBS} -DSWCL_ENABLE_DEBUG_LOGS")

if __name__ == "__main__":
    build_header()
    build_examples()
