#!/bin/sh
# Helper script for building a cross-compiling GCC toolchain

SCRIPT=$(basename $0)

if [[ $# -ne 2 ]]; then
    echo "Usage: $SCRIPT <prefix> <target>"
    echo " e.g.: $SCRIPT /opt/lambda-gcc i686-elf"
    exit 1
fi

PREFIX="$1"
TARGET="$2"
# Needed by GCC build
export PATH="$PATH:$PREFIX/bin:$PATH"

#
# Download
#

BINUTILS_RELEASE=2.38.90
BINUTILS_ARCHIVE=binutils-$BINUTILS_RELEASE.tar.xz
BINUTILS_DIR=binutils-$BINUTILS_RELEASE
if [ -f $BINUTILS_ARCHIVE ]; then
    echo "Binutils already downloaded."
else
    if ! curl "ftp://sourceware.org/pub/binutils/snapshots/$BINUTILS_ARCHIVE" -o $BINUTILS_ARCHIVE; then
        echo "FATAL: Could not download binutils!"
        exit 1
    fi
fi

GCC_RELEASE=12.2.0
GCC_ARCHIVE=gcc-$GCC_RELEASE.tar.xz
GCC_DIR=gcc-$GCC_RELEASE
if [ -f $GCC_ARCHIVE ]; then
    echo "GCC already downloaded."
else
    if ! curl "ftp://ftp.gnu.org/gnu/gcc/gcc-$GCC_RELEASE/$GCC_ARCHIVE" -o $GCC_ARCHIVE; then
        echo "FATAL: Could not download GCC!"
        exit 1
    fi
fi


#
# Extract
#

if [ -d $BINUTILS_DIR ]; then
    echo "binutils already extracted."
else
    if ! tar xvf $BINUTILS_ARCHIVE; then
        echo "FATAL: Could not extract binutils!"
        exit 1
    fi
fi

if [ -d $GCC_DIR ]; then
    echo "GCC already extracted."
else
    if ! tar xvf $GCC_ARCHIVE; then
        echo "FATAL: Could not extract GCC!"
        exit 1
    fi
fi



#
# Build + Install
#

if [ -f $PREFIX/bin/$TARGET-ld ]; then
    echo "binutils already built."
else
    if [ -d $BINUTILS_DIR/build ]; then
        echo "Nuking partial binutils build."
        rm -r $BINUTILS_DIR/build
    fi

    mkdir $BINUTILS_DIR/build
    cd    $BINUTILS_DIR/build
    if ! ../configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls; then
        echo "FATAL: Could not configure binutils!"
        exit 1
    fi
    if ! make -j $(nproc); then
        echo "FATAL: Could not build binutils!"
        exit 1
    fi
    if ! make install; then
        echo "FATAL: Could not install binutils!"
        exit 1
    fi
fi

if [ -f $PREFIX/bin/$TARGET-gcc ]; then
    echo "GCC already built."
else
    if [ -d $GCC_DIR/build ]; then
        echo "Nuking partial GCC build."
        rm -r $GCC_DIR/build
    fi

    mkdir $GCC_DIR/build
    cd    $GCC_DIR/build
    if ! ../configure --target=$TARGET --prefix="$PREFIX" --disable-nls --without-headers --enable-languages=c,c++; then
        echo "FATAL: Could not configure GCC!"
        exit 1
    fi

    if ! make all-gcc -j $(nproc); then
        echo "FATAL: Could not build GCC!"
        exit 1
    fi
    if ! make all-target-libgcc -j $(nproc); then
        echo "FATAL: Could not build GCC!"
        exit 1
    fi

    if ! make install-gcc; then
        echo "FATAL: Could not install GCC!"
        exit 1
    fi
    if ! make install-target-libgcc; then
        echo "FATAL: Could not install GCC!"
        exit 1
    fi
fi

echo "Cross-compiler build complete!"

