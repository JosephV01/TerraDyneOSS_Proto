@echo off
call "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" TerraDyneOSS_ProtoEditor Win64 Development -Project="C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311\TerraDyneOSS_Proto.uproject" -WaitMutex
exit /b %ERRORLEVEL%
