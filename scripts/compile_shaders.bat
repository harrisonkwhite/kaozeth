shaderc.exe -f quad_vs.sc -o quad_vs.bin --type vertex --platform windows --profile s_5_0 --varyingdef varying.def.sc -i .\zeta_framework\external\bgfx.cmake\bgfx\src\

shaderc.exe -f quad_fs.sc -o quad_fs.bin --type fragment --platform windows --profile s_5_0 --varyingdef varying.def.sc -i .\zeta_framework\external\bgfx.cmake\bgfx\src\
