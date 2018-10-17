@echo off

copy sdram.bin sdram.app

echo 代码文件  -bfumode 1
echo 资源文件  -bfumode 2
echo 代码+资源 -bfumode 3

isd_download.exe -cfg isd_tools_sdram_sfc.cfg -input uboot.boot ver.bin sdram.app -resource res 004A 005A 006A 007A audlogo 32 -disk file -aline 4096 -bfumode 1

del upgrade\JL_AC*.bfu upgrade\*.bin

copy jl_isd.bfu upgrade\JL_AC5X.bfu

echo.
echo 升级文件在upgrade目录下，将upgrade目录下的所有文件copy到SD卡的根目录，插卡上电即可升级
echo.
pause