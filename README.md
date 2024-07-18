


# ![SWCL logo](logo.svg) SWCL - Simple Wayland Client Library

Create Wayland clients easily.

This library simplifies creating native Wayland window, receiving mouse, keyboard or touch events and helping with creation of Client-Side Decorations (CSD). 

In a few lines of code you will have a window with OpenGL context inside of it that you can use to create whatever you want:

- Applications
- GUI toolkits
- Game engines

You can also port your existing applications to support Wayland with this library.

**SWCL** is written in **C** and it's just two files: `swcl.h` and a static library `libswcl.a`. You can put them directly in your project.

## Tutorial
### Setup

First you need to clone repository:
```sh
git clone https://github.com/mrvladus/swcl
```
Then install Wayland development dependencies:

For Fedora/RHEL based distros:
```
sudo dnf install gcc make wayland-devel mesa-libGLES-devel
```

For Debian/Ubuntu based distros:
```
sudo apt install gcc make libwayland-dev libgles2-mesa-dev
```

### Building SWCL
Enter `swcl` directory:
```sh
cd swcl
```
Build it with:
```sh
make
```
It will create theese files:
```
swcl
├── include
│   └── swcl.h
├── lib
│   └── libswcl.a
...
```

### Creating your project
Copy `include` and `lib` directories inside of your project. Then create `main.c` so your project directory looks like this:
```
myproject
├── include
│   └── swcl.h
├── lib
│   └── libswcl.a
└── main.c
```
 
Inside of the `main.c` file write this:

```c
#include "include/swcl.h"

// Function that SWCL will be calling each frame
void draw_window(SWCLWindow *win) {
  // Clear background with opaque white color
  swcl_clear_background(1, 1, 1, 1);

  // Swap window buffer at the end of the frame render
  swcl_window_swap_buffers(win);
}

int main(int argc, char const *argv[]) {
  // Create application object
  SWCLApplication *app = swcl_application_new("io.github.mrvladus.MyApp");

  // Create window configuration
  SWCLWindowConfig cfg = {
      .title = "My First Wayland Window",
      .height = 800,
      .width = 1000,
      .min_height = 100,
      .min_width = 100,
      .on_draw_cb = draw_window, // Pass draw_window function as 'on_draw' callback
  };

  // Create new window with configuration
  SWCLWindow *win = swcl_window_new(app, cfg);

  // Start application
  swcl_application_run(app);

  return 0;
}
```

Now compile it with this command:

```sh
gcc main.c -o myapp -Llib -lswcl -lwayland-client -lwayland-egl -lwayland-cursor -lGLESv2 -lEGL
```
- `gcc main.c` compiles your code
- `-o myapp` outputs it to executable file `myapp`
- `-Llib` tells compiler to look for libraries in `lib` directory
- `-lswcl -lwayland-client -lwayland-egl -lwayland-cursor -lGLESv2 -lEGL` links our program with theese libraries

And then run it with:
```sh
./myapp
```
Congratulations! You've created your first Wayland window!

Go to the examples directory to learn more.