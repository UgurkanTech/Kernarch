# Use an official Debian-based image as a base
FROM --platform=linux/i386 debian:bullseye

# Install required packages
RUN apt-get update && apt-get install -y \
	nasm \
	gcc \
	binutils \
	grub-pc-bin \
	xorriso \
	make \
	mtools \
	g++ \
	build-essential \
	gcc-multilib \
	g++-multilib \
	&& rm -rf /var/lib/apt/lists/*

# Set up a working directory
WORKDIR /usr/src/kernarch

# Copy the project files into the container
COPY ./KernarchOS .

# Build the bootloader and kernel
RUN make all

# Create an ISO image
#RUN make iso

# Default command to run the OS in QEMU
CMD ["cp", "/usr/src/kernarch/kernarch.iso", "/output/KernarchOS.iso"]

#CMD tail -f /dev/null