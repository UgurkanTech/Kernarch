#!/bin/bash

clear

docker compose -f "docker-compose.yml" up -d --build

if [ $? -ne 0 ]; then
    echo "Build failed. Exiting."
    exit 1
fi

cd output
disk_image="disk.img"

if [ -f "$disk_image" ]; then
    echo "The file $disk_image exists."
else
    echo "The file $disk_image does not exist. Creating it now..."
    qemu-img create -f raw $disk_image 512M
    echo "File $disk_image created."
fi

qemu-system-i386 -display default,show-cursor=on -m 1G -netdev user,id=mynet0 -device rtl8139,netdev=mynet0 -cdrom KernarchOS.iso -drive file=disk.img,format=raw,if=ide,index=0

cd ..