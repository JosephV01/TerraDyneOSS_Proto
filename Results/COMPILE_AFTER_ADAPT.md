# Compilation after isolated adaptations

UE 5.8.3 / MSVC 14.44.35229 / Win SDK 10.0.22621.0
Variant: OSS 270c311 + CPU-only stub. Not Fab. Not GardenFervor.

## Game Win64 Development

Command:
Build.bat TerraDyneOSS_Proto Win64 Development -Project=...\TerraDyneOSS_Proto.uproject -WaitMutex

Result: Succeeded, exit 0, ~96 s
Output: ...\Binaries\Win64\TerraDyneOSS_Proto.exe
C1083/Cxxxx: none in this log.

## Editor Win64 Development

Command:
Build.bat TerraDyneOSS_ProtoEditor Win64 Development -Project=...\TerraDyneOSS_Proto.uproject -WaitMutex

Result: Succeeded, exit 0, ~280 s
Linked UnrealEditor-TerraDyne.dll, UnrealEditor-TerraDyneOSS_Proto.dll, UnrealEditor-TerraDyneEditor.dll
C1083/Cxxxx: none in this log.

GardenFervor editor was not running during this Editor compile.
Two UnrealEditor processes were already attached to the isolated .uproject; they were not killed.

Compilation success does not validate CPU runtime behavior.
