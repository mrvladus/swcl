# ![SWCL logo](logo.svg) SWCL - Simple Wayland Client Library

Create Wayland clients easily.

This header-only library simplifies creating native Wayland window, receiving mouse or keyboard events and helping with creation of Client-Side Decorations (CSD).

In a few lines of code you will have a window with OpenGL context inside of it that you can use to create whatever you want:

- Applications
- GUI toolkits
- Game engines

You can also port your existing applications to support Wayland with this library.

**SWCL** is written in **C** and it's just one header file `swcl.h`. You can put it directly in your project.

## Tutorial

### Setup

First you need to clone repository:

```sh
git clone https://github.com/mrvladus/swcl
```

Then install Wayland development dependencies:

For Fedora/RHEL based distros:

```sh
sudo dnf install wayland-devel mesa-libGL-devel
```

For Debian/Ubuntu based distros:

```sh
sudo apt install libwayland-dev mesa-common-dev
```

### Creating your project

Copy `swcl.h` to your project. Then create `main.c` so your project directory looks like this:

```
myproject
│── swcl.h
└── main.c
```

Inside of the `main.c` file write this:

```c
// Because it's a header-only library - it contains functions declarations AND implementations
// Add this line to include SWCL functions implementations
// You need to add this line ONLY in ONE file
#define SWCL_IMPLEMENTATION
// Then include SWCL header
#include "swcl.h"

// Drawing function that called each frame
void draw(SWCLWindow *win) {
  // Draw white background
  swcl_clear_background((SWCLColor){255, 255, 255, 255});
  // Swap buffers
  swcl_window_swap_buffers(win);
}

int main() {
  // Create basic config
  SWCLConfig cfg = {.app_id = "io.github.mrvladus.Test"};
  // Create application
  SWCLApplication *app = swcl_application_new(&cfg);
  // Create window
  SWCLWindow *win = swcl_window_new(app, "Basic Window", 800, 600, 100, 100,
                                    false, false, draw);
  // Render first frame
  swcl_window_show(win);
  // Run application
  swcl_application_run(app);
  return 0;
}
```

Now compile it with this command:

```sh
gcc main.c -o myapp -lwayland-client -lwayland-egl -lwayland-cursor -lGL -lEGL -lm
```

- `gcc main.c` compiles your code
- `-o myapp` outputs it to executable file `myapp`
- `-lwayland-client -lwayland-egl -lwayland-cursor -lGL -lEGL -lm` links our program with needed libraries

And then run it with:

```sh
./myapp
```

Congratulations! You've created your first Wayland window!

Go to the examples directory to learn more.

### Examples

To build examples you need:

- Clone this repo
- Run `./build.py -e` inside it

This will build examples inside `examples` directory.
You can run them as any other binary.

### Documentation

See `swcl.h` for documentation. All of the functions and structs have comments. It's pretty simple.

### Development

TODO™

### Donations

If you like **SWCL** and want to support it, you can send your donations here:

- BTC

```
14pPYfZohS61PyGyXJyiGQHDFai4JR2q9w
```

- USDT

```
TCQn85c3vrRR7yMAfJABZmCXBa7gQvUcX5
```

- TON

```
UQBrhmho9259KsNZ8wYx6y_eeOwdcAo9aroOTRWRtnmcUmdr
```

Thank you!
