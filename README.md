lambda-os
=========

Hobby x86 operating system created in C

Documentation
-------------

Source code documentation can be found here: http://farlepet.github.io/lambda-os

Building
--------

To build Lambda OS you need the following tools:
 * x86 or x86_64 computer (unless using a cross compiler)
 * `clang` and `llvm` or `gcc`
 * `nasm`
 * `make`
 * `grub`
 * Ubuntu/apt-get based systems: `grub-pc-bin`
 * `grub-mkrescue`
 * `xorriso`

Build process:
 * Clone the git repository (If first time): `git clone https://github.com/farlepet/lambda-os.git`
 * Pull updates (If not first time): `git pull origin master`
 * Build (llvm/clang): `make`
 * Build (gcc) `make CC=gcc`
