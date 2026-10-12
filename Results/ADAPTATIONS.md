# Isolated adaptations — 270c311 CPU-only compile variant

Base plugin copy was identical to OSS 270c311 before these edits.
GardenFervor was not modified.

## Decision: GPU stub returning false is acceptable

`ApplyLocalIdempotentEdit` (`TerraDyneChunk.cpp` 1233-1239):
- If `bUseGPU` and not Paint: `bModifiedHeight = ApplyBrushGPU(...)`.
- If `!bModifiedHeight`: CPU Raise/Lower/Smooth/Flatten/Paint on the layer buffer.

`ApplyBrushGPU` 1353-1355: `DispatchTerraDyneBrushSimulation` false → return false
before any HeightBuffer write in that function.

Therefore a stub that returns false uses the same fallback as a real dispatch failure.
CPU layer edits then run. Mesh/collision flags are set from `bModifiedHeight` after the CPU path.

`SetupGPU` does not call Dispatch. Host `bEnableGPUBrushes=false` makes `bUseGPU` stay false
so ApplyBrushGPU is not entered at runtime. The stub exists only so Chunk.cpp compiles.

## History

`b7f8c32` (0.3) Chunk.cpp has no `Shaders/TerraDyneSimulationShader.h` include.
Public 0.3 tree has no `Shaders/` and no `Examples/TerraDyneIntegrationExamples.h`.
Those files cannot be restored from public history. 0.3 was not used as a substitute.

## Tests

UBT 5.8 ModuleRules has no per-file exclude.
`TERRADYNE_OSS_ISOLATED_EXCLUDE_EXAMPLE_TESTS=1` wraps:
- TerraDyneBasicTests.cpp (uses stripped example actor classes + missing shader usf check)
- TerraDyneWorldFrameworkTests.cpp (uses SaveLoad/Biome example actors)

Other Private/Tests/*.cpp remain in the build.
This is not a TerraDyne official test-suite result.
