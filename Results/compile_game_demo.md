# Compile Game — démo TerraDyne + M4

```
Build.bat TerraDyneOSS_Proto Win64 Development
-Project=...\TerraDyneOSS_Proto_270c311\TerraDyneOSS_Proto.uproject -WaitMutex
```

Result: **Succeeded** (relink après correctifs HUD).

Output: `Binaries/Win64/TerraDyneOSS_Proto.exe` (12/10/2026 01:39).

Correctifs compile hôte uniquement :
- `OnPaint` renommé `OnPaintMode` (conflit UUserWidget)
- `AddDynamic` des couches : bindings explicites (pas un tableau de pointeurs)
- `Slot` renommé (`CanvasSlot` / `BoxSlot`)
- include `TerraDyneDemoPaintStore.h` dans le HUD
