Lambda Kernel
=============

Lambda OS is a hobby operating system developed by Peter Farley. Lambda OS
is designed to be portable to multiple systems, and currently supports
x86-based PCs, ARMv7 (Cortex-A9) support is in progress, and x86_64 and RISCV
(RV64I) support is planned.

**NOTE**: This repository only contains the kernel portion of Lambda OS. For a
usable implementation of the kernel, see the lambda-os repository.

Platform support:
 - x86
   - x86 PC (platform with the most support)
 - ARMv7
   - Cortex-A9
     - Versatile Express A9 (early stages; partially dropped)
   - Cortex-A53
     - Broadcom BCM2836/7 (Raspberry Pi) (early stages)

Building
--------

To be able to build the Lambda kernel, you need the following tools:
 - `gcc`
   - If using `gcc`, you must use a cross-compiler. There is a convenience script for building one: `scripts/build-cross-compiler.sh`
   - TODO: reintroduce clang support
 - `make`
 - `eu-readelf` (elfutils)
 - `menuconfig` or similar, for easy configuration
   - Not required, `.config` file can be modified by hand
   - If no `.config` file is present, `.defconfig` is used instead
   - For a standalone Kconfig-compatible `menuconfig`, see [Kconfiglib](https://github.com/ulfalizer/Kconfiglib)


To build kernel:
 - (If fisrt time) Clone the `lambda-kern` code repository
 - (If not first time) Run `git pull`
 - If you wish to make changes to the configuration: `menuconfig` (or similar)
   - Default config targets x86
 - Make: `make CROSS_COMPILE=<gcc prefix>`
   - `CROSS_COMPILE` is not necessary when using Clang
     - Clang can be enabled via CONFIG_BUILD_USE_CLANG in the config
     - Using clang with ARM is currently not fully supported
   - To speed up compilation, add `-j<threads>` argument to the `make` command
