@echo off

set input="%1"
set output="%2"

if %input%=="" goto error
if %output%=="" goto error

set studio="C:\Program Files\Geomagic\Foundation 2013\StudioCORE.exe"

%studio% -input "%input%" -output "%output%" geomagic.py -ExitOnMacroEnd

goto exit

:error
echo "Usage: run-geomagic.bat <input> <output>"

:exit
