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

## Commit Message Format

See format rules below. Use subsystem names from the table above as
`<Subsystem>` tokens. Use `<Project>` from context (e.g. `Common`,
`Render`, `ECS`, `GAPI`, `Shader`, `Platform`).

---

## Commit Message Format Reference

### Rules

- **Language:** English only. Professional technical style.
- **Encoding:** ASCII only. No emojis, no smart quotes, no em dashes (`-`). Use only hyphen-minus (`-`).
- **Line width:** Body lines wrap at 72 characters.
- **Mood:** Imperative ("Fix crash", not "Fixed crash").

### Template

```
<Project>: <Subsystem>: <Imperative Title>

<Context/Problem - what was wrong and why. Wrap at 72 chars.>

<Solution/Implementation - what was done and how. Wrap at 72 chars.>

Video: <link, only if provided>
Fix:   <link, only if provided>
```

### Example

```
Render: FrameGraph: Fix UAV read from uninitialized resource

Bloom post-process pass reads from an uninitialized UAV when
MSAA is disabled, causing a GPU device lost on DX11.

Initialize the UAV with a zero-clear before the bloom dispatch
to ensure valid state on all hardware tiers.
```
