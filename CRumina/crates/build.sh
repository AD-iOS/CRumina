#!/bin/sh -

# set -x # debug

if [ -f '/etc/profile' ]; then
    source /etc/profile
    export jb=''
elif [ -f '/var/jb/etc/profile' ]; then
    source /var/jb/etc/profile
    export jb='/var/jb'
else
    echo 'Where the fuck "profile"?' 1>&2
fi

dpkgarch=$(dpkg --print-architecture)
if [ "$dpkgarch" = "iphoneos-arm64e" ]; then
    rootfs="/rootfs"
else
    rootfs=""
fi

if [ "$dpkgarch" = "iphoneos-arm64e" ]; then
    if [ -L "$rootfs/var/jb" ] && [ ! -e "$rootfs/var/jb" ]; then
        ln -sf / $rootfs/var/jb
    fi
fi

b="build"

setarch() {
    if [[ "$dpkgarch" =~ ^iphoneos-(arm|arm64|arm64e)$ ]]; then
        if [ "$1" = "crmpack" ]; then
            TARGET="aarch64-apple-iphoneos-crmpack"
        elif [ "$1" = "crmvm" ]; then
            TARGET="aarch64-apple-iphoneos-crmvm"
        elif [ "$1" = "cruminac" ]; then
            TARGET="aarch64-apple-iphoneos-cruminac"
        else
            TARGET="aarch64-apple-iphoneos-undefined"
        fi
    else
        if [ "$1" = "crmpack" ]; then
            TARGET="$(arch)-$(uname)-crmpack"
        elif [ "$1" = "crmvm" ]; then
            TARGET="$(arch)-$(uname)-crmvm"
        elif [ "$1" = "cruminac" ]; then
            TARGET="$(arch)-$(uname)-cruminac"
        else
            TARGET="$(arch)-$(uname)-undefined"
        fi
    fi
}

cxx=$(which c++ 2>/dev/null) || { echo "C++ compiler not found"; exit 1; }

cxxparam_include="-I../include -I../../include -I../../../include -Iinclude -I../../../../include -I$jb/usr/include"

all_build() {
    $cxx $cxxparam_include -c ../../../src/*.cc ../../../src/builtin/*.cc main.cc || return 1
}

if [ "$(uname)" = "Darwin" ]; then
    darwin_framework=(
        -framework CoreFoundation
        -framework CoreServices
        -framework Foundation
    )
else
    darwin_framework=()
fi

build_obj() {
    setarch $1
    $cxx \
      $cxxparam_include \
      ${darwin_framework[@]} \
      -L$jb/usr/lib \
      -lgmp -lgmpxx \
      *.o \
      -o "$TARGET"
      ln -s ./"$TARGET" $1
}

main() {
    build() {
        cd $1/src/
        # pwd
        if [ -d obj ]; then
            rm -rf obj
            mkdir obj
        else
            mkdir obj
        fi

        all_build

        mv *.o obj/

        cd obj/

        build_obj crmpack

        if [[ "$dpkgarch" =~ ^iphoneos-(arm64|arm64e)$ ]]; then
            ldid -S../../ens/ens.xml -Hsha256 -Hsha1 -M "$TARGET"
        fi

        # pwd
        cd ../../../
        # pwd
    }

    build rmpack
    build rmvm
    build ruminac

    # build_obj crmvm
    # build_obj cruminac
}

main 2>&1 | tee debug.log

chown 501:501 debug.log

