@echo off

set mls="C:\Program Files\VCG\MeshLab\meshlabserver.exe"
set studio="C:\Program Files\Geomagic\Foundation 2013\StudioCORE.exe"

set input_mesh="%1"
set output_mesh_without_extension="%2"
set output_mesh="%2".ply

if "%1"=="" goto error_exit
if "%2"=="" goto error_exit

echo %1
echo %2

%mls% -i %input_mesh% -o geomagic-tmp-input.ply -s decimate.mlx

%studio% geomagic.py -ExitOnMacroEnd

%mls% -i geomagic-tmp-output.ply -o %output_mesh% -s uv.mlx -om wt

%mls% -i %output_mesh% %input_mesh% -o %output_mesh% -om wt -s transfer-color-to-texture.mlx

%mls% -i %output_mesh% -o %output_mesh_without_extension%.obj -om wt
%mls% -i %output_mesh% -o %output_mesh_without_extension%.wrl -om wt

goto exit

:error_exit
echo "Usage: decimate-and-make-texture.bat input output"

:exit
