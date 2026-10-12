@echo off
"C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311\TerraDyneOSS_Proto.uproject" /Game/Demo/L_TerraDyneGardenDemo -game -unattended -nopause -nosplash -nullrhi -LowerSelfTest -AbsLog="C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311\Results\lower_selftest_editor.log"
exit /b %ERRORLEVEL%
