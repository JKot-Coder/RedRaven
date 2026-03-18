# CLAUDE.md - Project Reference

## Project

**OpenDemo** - graphics demo framework / engine foundation.
C++17, CMake + vcpkg, Windows primary (Linux/macOS supported).

## Build

```sh
cmake -B build --preset debug
cmake --build build
```

Dependencies managed by vcpkg (`vcpkg.json`). Requires MSVC 2022 or
LLVM/Clang via scoop. CMakePresets.json defines available presets.

## Repository Layout

```
src/libs/         - Core engine libraries (see subsystems below)
src/demo/         - Primary demo executable
src/tools/        - Utility tools
src/rfx/          - RFX shader compiler/toolchain
assets/           - Shaders and graphics assets
cmake/            - CMake helper modules
triplets/         - vcpkg architecture triplets
```

## Key Subsystems

| Path              | Subsystem    | Purpose                          |
|-------------------|--------------|----------------------------------|
| src/libs/ecs      | ECS          | Entity Component System core     |
| src/libs/ecs_window | EcsWindow  | Window management (ECS-based)    |
| src/libs/ecs_imgui | EcsImgui   | ImGui debug UI integration       |
| src/libs/gapi     | GAPI         | Graphics API abstraction layer   |
| src/libs/gapi_dx12 | GAPI/DX12  | Direct3D 12 backend              |
| src/libs/gapi_diligent | GAPI   | Diligent Engine backend          |
| src/libs/gapi_webgpu | GAPI     | WebGPU/Emscripten backend        |
| src/libs/render   | Render       | Rendering pipeline               |
| src/libs/shader   | Shader       | Shader compilation/management    |
| src/libs/effect_library | FX    | Visual effects library           |
| src/libs/platform | Platform     | OS abstractions                  |
| src/libs/inputting | Input       | Input handling                   |
| src/libs/math     | Math         | Math utilities                   |
| src/libs/common   | Common       | Shared utilities and helpers     |
| src/libs/stl      | STL          | Custom STL implementations       |

## Code Style

Based on `.clang-format` (WebKit style). Key rules:
- Braces always on new line (AfterFunction, AfterClass, BeforeElse, etc.)
- Indent case labels
- Namespace indentation: all levels
- Sorted includes
- Short functions/if/case allowed on single line

Run clang-format before committing.

## Testing

Framework: Catch2 v3.8.0

```
src/tests/            - Main test suite
src/libs/ecs/tests/   - ECS unit tests
src/rfx/tests/        - Shader compiler tests
```

## Git Preferences

- Do NOT add `Co-Authored-By: Claude *` to commit messages.
- After committing, run `git clang-format HEAD~1` and amend the commit
  if there are any formatting changes.

## Committing

Use the `/commit` skill to create commits. It handles message formatting,
staging, and post-commit clang-format automatically.
