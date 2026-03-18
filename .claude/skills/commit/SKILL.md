---
name: commit
description: Create a git commit following project conventions. Use whenever the user asks to commit, make a commit, or says /commit.
user-invocable: true
---

# Commit Skill

Create a git commit for this project following the rules below.

## Steps

1. Run `git status` and `git diff` (staged + unstaged) to understand what changed.
2. Run `git log -5 --oneline` to match the project's commit style.
3. Stage relevant files (prefer specific files over `git add -A`).
4. Write a commit message following the format below.
5. Commit.
6. Run `git clang-format HEAD~1` — if it reports changes, stage them and amend the commit (`git commit --amend --no-edit`).

## Commit Message Format

```
<Project>: <Subsystem>: <Imperative Title>

<Context/Problem — what was wrong and why. Wrap at 72 chars.>

<Solution/Implementation — what was done and how. Wrap at 72 chars.>

Video: <link, only if provided by user>
Fix:   <link, only if provided by user>
```

**Rules:**
- Language: English only. Professional technical style.
- ASCII only. No emojis, no smart quotes, no em dashes. Use hyphen-minus (`-`) only.
- Body lines wrap at 72 characters.
- Imperative mood: "Fix crash", not "Fixed crash".
- Do NOT add `Co-Authored-By: Claude *` lines.
- Omit Video/Fix lines if not provided.

**Project/Subsystem tokens** (from CLAUDE.md):

| Subsystem path          | Token           |
|-------------------------|-----------------|
| src/libs/ecs            | ECS             |
| src/libs/ecs_window     | EcsWindow       |
| src/libs/ecs_imgui      | EcsImgui        |
| src/libs/gapi           | GAPI            |
| src/libs/gapi_dx12      | GAPI/DX12       |
| src/libs/gapi_diligent  | GAPI            |
| src/libs/gapi_webgpu    | GAPI            |
| src/libs/render         | Render          |
| src/libs/shader         | Shader          |
| src/libs/effect_library | FX              |
| src/libs/platform       | Platform        |
| src/libs/inputting      | Input           |
| src/libs/math           | Math            |
| src/libs/common         | Common          |
| src/libs/stl            | STL             |

Use `<Project>` from context (e.g. `Common`, `Render`, `ECS`, `GAPI`, `Shader`, `Platform`).

## Example

```
Render: FrameGraph: Fix UAV read from uninitialized resource

Bloom post-process pass reads from an uninitialized UAV when
MSAA is disabled, causing a GPU device lost on DX11.

Initialize the UAV with a zero-clear before the bloom dispatch
to ensure valid state on all hardware tiers.
```
