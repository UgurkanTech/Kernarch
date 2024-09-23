# Use an official Debian-based image as a base
FROM debian:bullseye-slim

# Install required packages
RUN apt-get update && apt-get install -y --no-install-recommends \
	nasm:i386 \
	gcc:i386 \
	binutils:i386 \
	grub-pc-bin \
	xorriso \
	make \
	mtools \
	g++:i386 \
	libc6:i386 \
	libc6-dev:i386 \
	&& rm -rf /var/lib/apt/lists/*

# Set up a working directory
WORKDIR /usr/src/kernarch

# Copy the project files into the container
COPY ./KernarchOS .

# Build the bootloader and kernel
RUN make -j$(nproc) all

# Export iso file out of the container
CMD ["cp", "/usr/src/kernarch/kernarch.iso", "/output/KernarchOS.iso"]

# Keep from exiting for debugging
#CMD tail -f /dev/null