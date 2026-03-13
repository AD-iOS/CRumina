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

if [[ "$dpkgarch" =~ ^iphoneos-(arm|arm64|arm64e)$ ]]; then
    TARGET="aarch64-apple-iphoneos-crumina-cli"
else
    TARGET="$(arch)-$(uname)-crumina-cli"
fi

# rm -rf "$b" && mkdir -p "$b" || exit 1

cxx=$(which c++ 2>/dev/null) || { echo "C++ compiler not found"; exit 1; }

cd src/

main_build() {
    $cxx -I../include -I../../include -Iinclude -I$jb/usr/include -c *.cc main/*.cc || return 1
# 
#     mv *.o "$b"/ 2>/dev/null || {
#         echo "No object files generated"
#         return 1
#     }
}

builtin_build() {
    $cxx -I../../include -I../include -Iinclude -I$jb/usr/include -c *.cc || return 1
#     mv *.o "$b"/ 2>/dev/null || {
#         echo "No object files generated"
#         return 1
#     }
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
    $cxx \
      -I../include -I../../include \
      -I../../../include -Iinclude \
      -I$jb/usr/include \
      ${darwin_framework[@]} \
      -L$jb/usr/lib \
      -lgmp -lgmpxx \
      *.o \
      -o "$TARGET"
      ln -s ./"$TARGET" crumina-cli
}

main() {
    if [ -d obj ]; then
        rm -rf obj
        mkdir obj
    else
        mkdir obj
    fi

    main_build
    mv *.o obj/

    cd 'builtin'/

    ln -sf ../obj

    builtin_build

    mv *.o obj/

    cd ../obj/

    # cd obj/

    build_obj

    if [[ "$dpkgarch" =~ ^iphoneos-(arm64|arm64e)$ ]]; then
        ldid -S../../ens/ens.xml -Hsha256 -Hsha1 -M "$TARGET"
    fi
}

main 2>&1 | tee debug.log

chown 501:501 debug.log

# test
# cd obj
# build_obj