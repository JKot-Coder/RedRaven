# Binding Model Architecture

## 1. OVERVIEW

Engine supports DX12 and WebGPU. Three objects, three roles:

```
CommandEncoder                               [current]
  |-- BeginRenderPass(desc)  --> RenderPassEncoder
  |                                |-- Draw(effect, ...)   per-draw (Phase 10: ec*)
  |                                |-- End()
  |
  |-- Dispatch(ec, x, y, z)       Phase 10 -- not yet

EffectContext  (stack object, Phase 9)
  |-- SetTexture / SetUAV / SetFloat / ...   GlobalUniforms only
  |-- Bind(block)                            ParameterBlock ref
  |-- used as Draw/Dispatch argument
```

- **`CommandEncoder`** -- frame-level recorder. Issues compute
  dispatches. Begins render passes.
- **`RenderPassEncoder`** -- narrow scoped helper. Only render-pass
  commands: pass-level block binding, Draw, geometry setup. Knows
  nothing about effects or shader reflection.
- **`EffectContext`** -- staging object for one draw or dispatch.
  Stack-allocatable. `Set*` targets GlobalUniforms (global-scope
  Slang variables). `Bind` attaches a pre-built ParameterBlock.
  `Draw` / `Dispatch` snapshots the current state into the command
  list -- after that the context may be mutated and reused freely.

```
+------------------------------------------+
|  User code                               |
|  EffectContext ec(effect)                |
|  ec.SetTexture("src", tex)  -- global    |
|  ec.Bind(matBlock)          -- param blk |
|  rpe.Bind(frameBlock)       -- pass lvl  |
|  rpe.Draw(&ec, ...)                      |
+------------------------------------------+
|  Render layer                            |
|  snapshot ec -> SetBindGroup commands    |
|  uniform bytes -> cmdlist allocator      |
+-------------------+----------------------+
|  DX12 Backend     |  WebGPU Backend      |
|  CopyDescriptors  |  LRU cache           |
|  ring buffer      |  hash(view ptrs)     |
|  no cache         |  GPUBindGroup        |
+-------------------+----------------------+
```

Design goals:
- Zero overhead for DX12 (ring buffer, no caching)
- Correct operation with immutable `GPUBindGroup` in WebGPU
- Named API -- no slot indices, no space indices ever exposed
- Names resolved immediately at `Set*` / `Bind` time, not deferred
- Pass-level blocks bound once, hardware holds them across all draws

---

## 2. BINDING FREQUENCY

Each ParameterBlock has a fixed binding space assigned by the shader
compiler. Space encodes update frequency by convention:

| Space | Frequency    | Who binds          | Typical content              |
|-------|--------------|--------------------|------------------------------|
| 0     | per-frame    | `rpe.Bind(block)`  | VP matrix, time, light dir   |
| 1     | per-view     | `rpe.Bind(block)`  | viewport, frustum, exposure  |
| 2     | per-material | `ec.Bind(block)`   | albedo, normal, roughness    |
| N     | per-draw     | `ec.Set*(name)`    | GlobalUniforms: model matrix |

Pass-level blocks (space 0, 1) are bound once at pass setup.
Hardware (DX12 root table, WebGPU bind group) holds them for all
subsequent draws -- zero cost in draw loop.

Draw-level blocks (space 2+) and GlobalUniforms change per draw and
are submitted with each `Draw` / `Dispatch` call.

---

## 3. SHADER COMPILER GUARANTEES

ParameterBlock spaces are assigned and validated at **compile time**,
not at runtime.

The shader compiler output includes a global registry of
ParameterBlock declarations: name + structure layout + space index.
Individual shaders reference these blocks by name. If compilation
succeeds, all shaders in the project are guaranteed to agree on
space assignments for every named block.

```
Compiler output (per project):
  GlobalBlocks:
    "frame"    : FrameData    space=0
    "view"     : ViewData     space=1
    "material" : MaterialData space=2

  Shader "bloom.cs":
    uses GlobalUniforms (dynamic, space=N)
    -- no ParameterBlock refs

  Shader "pbr.vs":
    uses "frame"    (space=0)
    uses "view"     (space=1)
    uses "material" (space=2)
    uses GlobalUniforms (dynamic)
```

Runtime never validates spaces. If a shader loaded, spaces are
correct by construction.

---

## 4. OBJECT ROLES AND API

### 4.1 EffectContext  *(Phase 9 -- not yet implemented)*

Stack-allocatable staging object. Knows the Effect at construction,
so `Set*` resolves names immediately via O(1) hash lookup.

`Set*` methods write **only into GlobalUniforms** (the Slang
default parameter block, global-scope variables). Single flat
namespace -- no ambiguity possible.

`Bind` attaches a reference to a pre-built `BindingBlock` for a
named ParameterBlock. The block carries its own space index.

`Draw` / `Dispatch` snapshots the entire current state (GlobalUniforms
bytes + block refs) into the command list. After the snapshot the
context can be mutated and reused for the next draw.

```cpp
class EffectContext
{
    explicit EffectContext(const Effect* effect);

    // GlobalUniforms only -- immediate O(1) resolution.
    void SetTexture(std::string_view name, const GpuResourceView* view);
    void SetUAV    (std::string_view name, const GpuResourceView* uav);
    void SetSampler(std::string_view name, const GpuResourceView* s);
    void SetFloat  (std::string_view name, float v);
    void SetFloat2 (std::string_view name, float2 v);
    void SetFloat4 (std::string_view name, float4 v);
    void SetFloat4x4(std::string_view name, const float4x4& m);

    // ParameterBlock binding -- block carries its own space.
    void Bind(const BindingBlock* block);
};
```

### 4.2 RenderPassEncoder

Render-pass-scoped helper. No effect or shader knowledge.

Current API:

```cpp
class RenderPassEncoder
{
    void SetVertexLayout(const VertexLayout* layout);
    void SetVertexBuffer(uint32_t slot, const Buffer& buffer,
                         uint32_t offset = 0);
    void SetIndexBuffer(const Buffer& buffer);

    // Effect passed directly -- binding API not wired yet.
    void Draw(Effect* effect,
              PrimitiveTopology topology,
              uint32_t startVertex,
              uint32_t vertexCount,
              uint32_t instanceCount = 0);

    void DrawIndexed(Effect* effect,
                     PrimitiveTopology topology,
                     uint32_t startIndex,
                     uint32_t indexCount,
                     uint32_t instanceCount = 0);

    void End();
};
```

Target API *(Phase 10)*:

```cpp
class RenderPassEncoder
{
    // Pass-level ParameterBlock -- bound once, held for all draws.
    // Space comes from block->GetLayout()->GetBindingSpace().
    void Bind(const BindingBlock* block);

    // Draw -- snapshots ec state into the command list.
    void Draw(EffectContext* ec,
              PrimitiveTopology topology,
              uint32_t vertexCount,
              uint32_t instanceCount = 1);

    void DrawIndexed(EffectContext* ec,
                     PrimitiveTopology topology,
                     uint32_t indexCount,
                     uint32_t instanceCount = 1);

    void SetVertexLayout(const VertexLayout* layout);
    void SetVertexBuffer(uint32_t slot, const Buffer& buffer,
                         uint32_t offset = 0);
    void SetIndexBuffer(const Buffer& buffer);

    void End();
};
```

### 4.3 CommandEncoder

Current API:

```cpp
class CommandEncoder
{
    RenderPassEncoder BeginRenderPass(const RenderPassDesc& desc);
    void Finish();
};
```

Target API *(Phase 10)*:

```cpp
class CommandEncoder
{
    // Compute dispatch -- no render pass needed.
    void Dispatch(EffectContext* ec,
                  uint32_t x, uint32_t y, uint32_t z);

    RenderPassEncoder BeginRenderPass(const RenderPassDesc& desc);

    void Finish();
};
```

---

## 5. USAGE EXAMPLES  *(Phase 9-10 target -- not yet compilable)*

### 5.1 Main pass -- thousands of draw calls

```cpp
// ParameterBlocks created once (at load / scene setup)
auto frameBlock = effectManager.CreateBlock("frame");
auto viewBlock  = effectManager.CreateBlock("view");

// Per-frame update
frameBlock["viewProj"] = viewProjMatrix;
frameBlock["time"]     = totalTime;
viewBlock["viewport"]  = viewport;

auto rpe = commandEncoder.BeginRenderPass(desc);

// Bind once -- space 0 and 1 held for all draws below
rpe.Bind(frameBlock.get());
rpe.Bind(viewBlock.get());

for (const auto& obj : scene.objects) {
    // Stack-allocated -- zero heap, snapshot at Draw
    EffectContext ec(obj.effect);
    ec.Bind(obj.matBlock.get());         // space 2, per-material
    ec.SetFloat4x4("model", obj.trs);   // GlobalUniforms
    rpe.Draw(&ec, Topology::TriangleList, obj.vertexCount);
    // ec goes out of scope -- fine, already snapshotted
}

rpe.End();
```

### 5.2 Post-process / compute

```cpp
EffectContext ec(brightPassEffect);
ec.SetTexture("g_input",  hdrSRV);
ec.SetUAV    ("g_output", brightUAV);
ec.SetFloat  ("g_threshold", 0.8f);
commandEncoder.Dispatch(&ec, groupsX, groupsY, 1);
```

### 5.3 Ping-pong -- same effect, two dispatches

```cpp
EffectContext ec(bloomEffect);

// Pass A
ec.SetTexture("src", texA);
ec.SetUAV    ("dst", texB);
commandEncoder.Dispatch(&ec, gx, gy, 1);  // snapshot A

// Reuse -- mutate and dispatch again
ec.SetTexture("src", texB);
ec.SetUAV    ("dst", texA);
commandEncoder.Dispatch(&ec, gx, gy, 1);  // snapshot B
```

---

## 6. DATA STRUCTURES

### 6.1 EffectContext internals  *(Phase 9 -- not yet implemented)*

```
EffectContext
    effect               : const Effect*
    globalUniforms       : GlobalUniformsState   // dynamic per-draw
    boundBlocks[MAX_BINDING_GROUPS]              // ParameterBlock refs

GlobalUniformsState
    layout               : const BindingBlockLayout*
    resources[MAX_BINDINGS] : const GpuResourceView*
    uniformData[]           : uint8_t[MAX_UNIFORM_BUFFER_SIZE]  // see Q4
    dirtyResources          : uint32_t
    dirtyUniforms           : bool

boundBlocks[space]       : const BindingBlock*   // null if not bound
```

At `Draw` / `Dispatch`: render layer walks `boundBlocks[]` and
`globalUniforms`, emits `SetBindGroup` commands, copies uniform bytes
to command list allocator.

### 6.2 BindingBlock (ParameterBlock state)  *(Phase 2 -- not yet implemented)*

**File:** `src/libs/render/BindingBlock.hpp`

Mutable state for a named ParameterBlock. Created via
`EffectManager::CreateBlock("name")`. May be long-lived (material)
or per-frame (frame constants).

```
BindingBlock
    layout      : const BindingBlockLayout*
    resources[] : const GpuResourceView*[]
    uniformData : uint8_t[]
    dirtyResources : uint32_t
    epoch          : uint32_t
```

No `cachedBindGroup` / `cachedEpoch` -- caching is backend-owned.

### 6.3 BindingBlockLayout (IMPLEMENTED)

**File:** `src/libs/render/BindingBlockLayout.hpp`

```
BindingBlockLayout
    name              : const char*
    bindingSpace      : uint32_t
    uniformBufferSize : uint32_t
    fields[]          : FieldDesc
    resources[]       : ResourceSlotDesc
    fieldMap          : flat_hash_map<HashType, uint32_t>   // O(1)
    resourceMap       : flat_hash_map<HashType, uint32_t>   // O(1)
```

### 6.4 SetBindGroup command (IMPLEMENTED)

**File:** `src/libs/gapi/commands/Binding.hpp`

```
Commands::SetBindGroup
    group       : uint32_t
    bindGroup   : IBindingGroup*
    uniformData : const std::byte*
    uniformSize : uint32_t
```

### 6.5 Effect / EffectManager

Current (implemented):

```cpp
// EffectManager builds blockLayouts[] and blockLayoutMap on Init().
// blockLayoutMap: nameHash -> index into blockLayouts[].
// Effect has no layout lookup yet.
```

Target *(Phase 5b + Phase 2)*:

```cpp
const BindingBlockLayout* Effect::FindLayout(HashType nameHash) const;

eastl::unique_ptr<BindingBlock>
    EffectManager::CreateBlock(const std::string& name);
```

---

## 7. DX12 BACKEND -- NO CACHE

Pass-level blocks (space 0, 1): root descriptor tables set once at
`rpe.Bind()`, not touched in draw loop.

Per-draw: at each `Draw` / `Dispatch`, for each dirty group in
`EffectContext`:

```
DX12::Submit(ec, cmdList):
    for space in ec->boundBlocks where block != null and dirty:
        for i in 0..resourceCount:
            handle = descriptorMgr.GetCpuHandle(block->resources[i])
            ringBuffer.Copy(cursor + i, handle)
        cmdList->SetGraphicsRootDescriptorTable(space,
            ringBuffer.GetGpuHandle(cursor))
        ringBuffer.Advance(resourceCount)

    // GlobalUniforms
    uniformBytes = cmdList.Allocate(globalUniforms.uniformData.size())
    memcpy(uniformBytes, globalUniforms.uniformData, size)
    cmdList.EmplaceCommand<SetBindGroup>(globalSpace, null,
                                         uniformBytes, size)
```

### 7.1 Descriptor ring buffer

```
DescriptorRingBuffer
    heap      : ID3D12DescriptorHeap*   // GPU-visible, shader-visible
    capacity  : uint32_t
    cursor    : uint32_t
    frameStart: uint32_t[MAX_GPU_FRAMES_BUFFERED]
```

Reset at `MoveToNextFrame`. Initial capacity 8192 descriptors.

---

## 8. WEBGPU BACKEND -- LRU CACHE

Pass-level blocks: `setBindGroup(space, group)` once, held by the
WebGPU render pass encoder for all subsequent draws.

Per-draw: hash view **pointer** array per group (pointers are stable
within a frame), LRU lookup or create `GPUBindGroup`.

### 8.1 Cache key

```cpp
uint64_t ComputeGroupKey(const GpuResourceView** views, uint32_t count)
{
    return xxHash3_64(views, count * sizeof(void*));
}
```

No ViewID system needed.

### 8.2 LRU cache

```
BindGroupCache<wgpu::BindGroup>
    cache   : flat_hash_map<uint64_t, wgpu::BindGroup*>
    lru     : LRUList<uint64_t>
    MAX_ENTRIES = 1024
```

### 8.3 Uniform ring buffer

```
UniformRingBuffer
    gpuBuffer : wgpu::Buffer    // Uniform | CopyDst
    capacity  : uint32_t        // initially 256 KB
    cursor    : uint32_t
    alignment : uint32_t        // minUniformBufferOffsetAlignment
```

Dynamic offset passed to `setBindGroup` per draw.

---

## 9. REUSABLE UTILITIES

```
src/libs/gapi/utils/
    BindingCache.hpp     -- LRU + flat_hash_map<uint64_t, T*>
    HashUtils.hpp        -- xxHash3 / CityHash64 wrappers

src/libs/gapi_dx12/
    DescriptorRingBuffer.hpp/.cpp

src/libs/gapi_webgpu/
    UniformRingBuffer.hpp/.cpp
    BindGroupCache.hpp           -- BindingCache<wgpu::BindGroup>
```

---

## 10. IMPLEMENTATION PHASES

### Completed (infrastructure)

| Phase | Name                       | Files                                   | Status      |
|-------|----------------------------|-----------------------------------------|-------------|
| 0a    | BindGroupLayoutImpl getter | gapi_webgpu/BindingGroupLayoutImpl      | DONE        |
| 0b    | InitBindingGroup + layout  | gapi/Device.hpp, gapi_webgpu/DeviceImpl | DONE        |
| 0c    | Uniform serialization      | shader_compiler, effect_library         | DONE        |
| 1     | BindingBlockLayout         | render/BindingBlockLayout.hpp/.cpp      | DONE        |
| 3     | EffectManager layout build | render/EffectManager.hpp/.cpp           | DONE        |
| 4     | SetBindGroup command       | gapi/commands/Binding.hpp               | DONE        |
| 5     | BindingContext (interim)   | --                                      | SKIP (->10) |
| 6     | RenderPassEncoder proxy    | --                                      | SKIP (->10) |
| 7     | WebGPU: SetBindGroup       | gapi_webgpu/CommandListImpl.cpp         | DONE*       |

\* Phase 7 is a stub: bind group plumbing works, uniform data upload
deferred to Phase 15 (uniform ring buffer). Currently passes null.

### Remaining

| Phase | Name                       | Files                                   | Depends on |
|-------|----------------------------|-----------------------------------------|------------|
| 2     | BindingBlock               | render/BindingBlock.hpp/.cpp (new)      | 1          |
| 5b    | Effect carries layouts     | render/Effect.hpp/.cpp (mod)            | 1, 3       |
|       |                            | Effect::FindLayout(nameHash)            |            |
|       |                            | EffectManager::CreateBlock(name)        |            |
| 8     | Hash utilities             | gapi/utils/HashUtils.hpp                | -          |
| 9     | EffectContext              | render/EffectContext.hpp/.cpp (new)     | 2, 5b      |
|       |                            | Set* -> GlobalUniforms only             |            |
|       |                            | Bind(BindingBlock*) for ParameterBlocks |            |
|       |                            | stack-allocatable, snapshot at Draw     |            |
| 10    | CommandEncoder refactor    | render/CommandEncoder.hpp/.cpp (mod)    | 9          |
|       |                            | Dispatch(ec*, x, y, z)                 |            |
|       |                            | RenderPassEncoder::Bind(BindingBlock*)  |            |
|       |                            | RenderPassEncoder::Draw(ec*, ...)       |            |
| 11    | DX12 ring buffer           | gapi_dx12/DescriptorRingBuffer.hpp/.cpp | -          |
| 12    | DX12 submit path           | gapi_dx12/CommandListImpl.cpp (mod)     | 11         |
| 13    | WebGPU LRU cache util      | gapi/utils/BindingCache.hpp             | 8          |
| 14    | WebGPU uniform ring buffer | gapi_webgpu/UniformRingBuffer.hpp/.cpp  | 7          |
| 15    | WebGPU submit path         | gapi_webgpu/CommandListImpl.cpp (mod)   | 13, 14     |
| 16    | Cleanup interim            | render/BindingBlock: remove epoch field | 10         |
| 17    | Static BindingSet API      | render/BindingSet.hpp/.cpp (new)        | 9          |
| 18    | Integration: demo3         | demo3/Application.cpp (mod)             | All        |

### Phase 9 - EffectContext
New class. Constructor takes `const Effect*`. `Set*` methods resolve name against GlobalUniforms
layout only (`effect->FindLayout(DEFAULT_BLOCK_HASH)`) -- single
O(1) lookup, no search across all layouts. `Bind(block)` stores
a raw pointer; space comes from `block->GetLayout()->GetBindingSpace()`.
Snapshot at `Draw`/`Dispatch`: render layer copies uniform bytes to
command list allocator, collects block pointers, emits SetBindGroup.

### Phase 10 - CommandEncoder refactor
Add `RenderPassEncoder::Bind(const BindingBlock*)` for pass-level
blocks. Change `Draw`/`DrawIndexed` signature from `Effect*` to
`EffectContext*`. Add `CommandEncoder::Dispatch(EffectContext*,
uint32_t, uint32_t, uint32_t)`.

### Phase 16 - Cleanup interim
Remove `epoch` field from `BindingBlock` if unused after Phase 10.
Fold any pass-level block tracking directly into `RenderPassEncoder`
instead of a separate object.

---

## 11. DECISIONS SUMMARY

| Question | Decision |
|----------|----------|
| EffectContext lifetime | Stack object; Draw snapshots state, context reusable after |
| Set* scope | GlobalUniforms only -- one block, no ambiguity |
| ParameterBlock binding | Explicit `Bind(block)` on EffectContext or RenderPassEncoder |
| Pass-level blocks | `rpe.Bind(block)` once; hardware holds for all draws |
| Draw-level blocks | `ec.Bind(block)` per draw |
| Space assignment | Shader compiler assigns and guarantees consistency |
| Runtime space validation | None -- compiler guarantees |
| Caching | Backend-owned only |
| DX12 binding | CopyDescriptors into ring buffer, no cache |
| WebGPU binding | LRU cache keyed by hash of view pointer array |
| ViewID system | Not needed |
| Uniform upload DX12 | CommandList linear allocator, per-draw |
| Uniform upload WebGPU | Dynamic uniform buffer with per-draw offset |

---

## 12. OPEN QUESTIONS

### Q1: BindingSet for truly immutable resources -- OPEN
For resources that never change (skybox cubemap, blue noise LUT):
pre-baked `BindingSet` skips cache entirely.
- DX12: persistent descriptor heap, direct GPU handle.
- WebGPU: `GPUBindGroup` created once, held by reference.
Phase 17. Low priority.

### Q2: EffectContext::Bind validation -- OPEN
Should `ec.Bind(block)` assert that the block's layout belongs to
the effect? Options:
- (A) Assert in debug builds only.
- (B) Silent no-op if layout not found in effect.
- (C) No validation (trust caller).
**Leaning:** (A).

### Q3: How to pass BindingGroupLayout to BindingGroup -- RESOLVED
Resolved in Phase 0b. `const BindingGroupLayout& layout` added to
`InitBindingGroup` in `IMultiThreadDevice`.

### Q4: EffectContext uniform buffer sizing -- OPEN
`GlobalUniformsState` stores `uniformData[]` inline for zero-heap
stack allocation. But `uniformBufferSize` is runtime-determined from
reflection. C++ forbids VLAs; options:
- (A) Fixed compile-time max (`MAX_UNIFORM_BUFFER_SIZE`). Simple,
  wastes stack if actual size is small.
- (B) `alignas` + `uint8_t buf[N]` template parameter on
  `EffectContext<N>`. Caller chooses size; verbose.
- (C) Heap allocation with small-buffer optimization (SBO). Adds
  allocator dependency, contradicts "zero heap" goal.
**Leaning:** (A) with a generous fixed max (e.g. 4 KB). GlobalUniforms
are per-draw transients; a fixed cap is acceptable.
