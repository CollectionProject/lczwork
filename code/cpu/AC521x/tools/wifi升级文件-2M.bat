@echo off

copy sdram.bin sdram.app

echo �����ļ�  -bfumode 1
echo ��Դ�ļ�  -bfumode 2
echo ����+��Դ -bfumode 3

isd_download.exe -cfg isd_tools_sdram_sfc.cfg -input uboot.boot ver.bin sdram.app -resource res 004A 005A 006A 007A audlogo 32 -disk file -aline 4096 -bfumode 1

del upgrade\JL_AC*.bfu upgrade\*.bin

copy jl_isd.bfu upgrade\JL_AC5X.bfu

echo.
echo �����ļ���upgradeĿ¼�£���upgradeĿ¼�µ������ļ�copy��SD���ĸ�Ŀ¼���忨�ϵ缴������
echo.
pause