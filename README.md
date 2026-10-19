# Bomberman

A 2D Bomberman for the FIB *Videojocs* course (2026/27 Q1). OpenGL 3 + GLFW + GLEW + GLM.

## Current state

This is work in progress. What runs today:

- Tilemap rendering and collisions, loaded from `assets/levels/`.
- Player movement.
- Two enemy types: `Enemy` (patrol) and `EnemyChaser` (patrols, then chases the player).
- Bombs.

Everything else from the assignment — menus, 5 levels, the boss, powerups, HUD, audio — is
still ahead of us. The feature roadmap lives on the project's Notion page.

## Building

You need a C++17 compiler, **CMake 3.21+** and **git**. Everything else (GLFW, GLM, GLEW) is
downloaded automatically the first time you configure, so the first configure needs a network
connection and takes a couple of minutes.

### Linux

```sh
cmake --preset linux-debug
cmake --build --preset linux-debug
./build/linux-debug/bomberman
```

Use `linux-release` instead of `linux-debug` for an optimised build. The presets use Ninja; on
Arch that is `pacman -S cmake ninja`, on Debian/Ubuntu `apt install cmake ninja-build`.

### Windows / Visual Studio

Either way works:

- **Open the folder in Visual Studio.** VS 2019+ configures CMake projects by itself; pick the
  `vs2022` preset and press F5.
- **Double-click `generate-vs.bat`.** It writes `build\vs2022\bomberman.sln` and opens it.

There is no `.sln` in the repository on purpose — it is generated from `CMakeLists.txt`, which
is the single source of truth for the build. Adding a source file means adding it to
`CMakeLists.txt`, nothing else.

## Layout

```
CMakeLists.txt        the build; the only place source files are listed
CMakePresets.json     linux-debug, linux-release, vs2022
cmake/                dependencies.cmake -- all FetchContent lives here
generate-vs.bat       convenience wrapper for the VS generator
src/                  all game and engine code
assets/               everything the game loads at runtime
  images/  levels/  shaders/
art/                  GIMP .xcf sources for the sprites (not shipped)
third_party/          stb_image.h
docs/                 the assignment PDFs
```

`assets/` is copied next to the executable on every build, and the game resolves asset paths
against its own location via `assetPath()` in `src/AssetPath.h`. That means the binary works
from any working directory — don't go back to bare relative paths like `"images/player.png"`.

## Controls

Arrow keys to move, space to drop a bomb.
