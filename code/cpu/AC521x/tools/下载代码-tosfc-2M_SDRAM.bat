
rem copy version ver.bin

rem ��Ԥ��������������������copy /B sdram.bin+sfcext.bin sdram.app

copy sdram.bin sdram.app

isd_download.exe -cfg isd_tools_sdram_sfc.cfg -input  uboot.boot ver.bin sdram.app  -resource res 004A 005A 006A 007A audlogo 32 -disk norflash -div 2 -dev dv15 -boot 0xf02000  -reboot  -aline 4096 

