@echo off
rem Do not edit! This batch file is created by CASIO fx-9860G SDK.


if exist NOTEC.G1A  del NOTEC.G1A

cd debug
if exist FXADDINror.bin  del FXADDINror.bin
"C:\Casio_SDK\OS\SH\Bin\Hmake.exe" Addin.mak
cd ..
if not exist debug\FXADDINror.bin  goto error

"C:\Casio_SDK\Tools\MakeAddinHeader363.exe" "D:\Projects\Casio\NOTE-C"
if not exist NOTEC.G1A  goto error
echo Build has completed.
goto end

:error
echo Build was not successful.

:end

