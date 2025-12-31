# Kine

[![Build](https://github.com/rhmvl/Kine/actions/workflows/kine.yaml/badge.svg)](https://github.com/rhmvl/Kine/actions/workflows/kine.yaml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)
![C++](https://img.shields.io/badge/C%2B%2B-20-blue)

A 2D engine written in C++.

This is a personal / learning project. It is not meant to be feature-complete, stable, or especially user-friendly.

## Building

First, fetch and build dependencies:
```
```
```bash
make deps
```

Then build the engine and examples:

```bash
make build
```

Run the main target:

```bash
make run
```

## Examples

Example projects live in `examples/`.

Each example is built as its own executable:

```bash
examples/<example_name>/bin/<example_name>
```

If an example has an `assets/` folder, it’s copied automatically after build.

## Dependencies

All third-party libraries are downloaded into `./external/`:

- GLFW, glad (windowing / OpenGL)
- GLM (math)
- EnTT (ECS)
- FreeType (fonts)
- ImGui (debug UI)
- Lua + sol2 (scripting)
- stb_image, miniaudio, nlohmann/json

## Compiler Requirements

- GCC / Clang with full C++20 support
- MSVC with `/std:c++20`
- Warnings are treated as errors

## Code Layout

- `include/` – public engine headers  
- `src/` – engine implementation  
- `examples/` – sample projects using the engine  

The main entry point is `include/kine.hpp`.

APIs are not stable and may change without warning.

## License

GPLv3.

Do whatever you want with it, but keep it open source.

## Status

Not stable.
Not finished.
Probably never will be.

