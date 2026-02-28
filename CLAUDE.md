# Commit Message Format Reference

## Rules

- **Language:** English only. Professional technical style.
- **Encoding:** ASCII only. No emojis, no smart quotes, no em dashes (`-`). Use only hyphen-minus (`-`).
- **Line width:** Body lines wrap at 72 characters.
- **Mood:** Imperative ("Fix crash", not "Fixed crash").

## Template

```
<Project>: <Subsystem>: <Imperative Title>

<Context/Problem - what was wrong and why. Wrap at 72 chars.>

<Solution/Implementation - what was done and how. Wrap at 72 chars.>

Video: <link, only if provided>
Fix:   <link, only if provided>
```

## Example

```
Render: FrameGraph: Fix UAV read from uninitialized resource

Bloom post-process pass reads from an uninitialized UAV when
MSAA is disabled, causing a GPU device lost on DX11.

Initialize the UAV with a zero-clear before the bloom dispatch
to ensure valid state on all hardware tiers.
```
