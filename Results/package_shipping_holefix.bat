@echo off
call "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" TerraDyneOSS_ProtoEditor Win64 Development -Project="C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311\TerraDyneOSS_Proto.uproject" -WaitMutex
if errorlevel 1 exit /b %ERRORLEVEL%
call "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311\TerraDyneOSS_Proto.uproject" -noP4 -platform=Win64 -clientconfig=Shipping -build -cook -stage -pak -archive -archivedirectory="C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311\Packaged\Win64ShippingHoleFix" -unattended -utf8output -nocompileuat
exit /b %ERRORLEVEL%
