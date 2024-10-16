@echo off

cls

docker-compose -f "docker-compose.yml" up -d --build

if %errorlevel% neq 0 (
    echo Build failed. Exiting.
    exit /b %errorlevel%
)

set PATH=C:\msys64\ucrt64\bin;%PATH%
cd output
set "disk_image=disk.img"
if exist "%disk_image%" (
    echo The file %disk_image% exists.
) else (
    echo The file %disk_image% does not exist. Creating it now...
    qemu-img create -f raw %disk_image% 512M
    echo File %disk_image% created.
)

qemu-system-i386 -d pcall,cpu_reset -monitor stdio -serial file:serial_output.log -display default,show-cursor=on -m 2G -netdev user,id=mynet0 -device rtl8139,netdev=mynet0 -cdrom KernarchOS.iso -drive file=disk.img,format=raw,if=ide,index=0

cd ..