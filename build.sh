#!/bin/bash

# Stop and remove containers, networks, images, and volumes defined in docker-compose.yml
docker-compose down --rmi 'local'

# Clear Docker's build cache
#docker builder prune -af

# Remove old .iso
rm -f output/KernarchOS.iso

# Run docker-compose up
docker-compose up

qemu-system-x86_64 -cdrom output/KernarchOS.iso