# Compilation — TerraDyne OSS 270c311 on UE 5.8.3

## Commands

Editor (failed before compile, Live Coding lock):
```
"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" TerraDyneOSS_ProtoEditor Win64 Development "-Project=C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311\TerraDyneOSS_Proto.uproject" -WaitMutex
```
Exit 6. Message: Unable to build while Live Coding is active.
Lock owner: UnrealEditor.exe GardenFervor.uproject + LiveCodingConsole.exe -Group=UE_GardenFervor_0x21a5ea8c
GardenFervor editor was not closed or killed.

Game:
```
"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" TerraDyneOSS_Proto Win64 Development "-Project=C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311\TerraDyneOSS_Proto.uproject" -WaitMutex
```
Exit 6. Result: Failed (OtherCompilationError). Duration ~232 s. 56 actions scheduled.

## Toolchain (from UBT Game log)

- UE: 5.8.3 (Build.version PatchVersion 3, CL 58210709)
- DotNet SDK bundled: 10.0 win-x64
- ISPC 1.24.0
- Visual Studio 18 Community, MSVC 14.44.35229 (path 14.44.35207)
- Windows SDK 10.0.22621.0

## Fatal errors (intact OSS copy, no plugin edit)

1. TerraDyneChunk.cpp:13 — C1083 Shaders/TerraDyneSimulationShader.h not found
2. TerraDyneBasicTests.cpp:6 — C1083 Examples/TerraDyneIntegrationExamples.h not found
3. TerraDyneWorldFrameworkTests.cpp:5 — C1083 Examples/TerraDyneIntegrationExamples.h not found

These three paths are absent from git tree 270c311 (commit message: Strip Fab features).
No other Cxxxx errors appeared in this Game log.
No 5.8 API incompatibility was reached for TerraDyneChunk.cpp (failed at include).
Other listed .cpp units were compiled after the fatals; this log does not print per-file success.

## Plugin not modified

No stub, no file exclusion, no EngineVersion bump.
