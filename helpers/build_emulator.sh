#!/usr/bin/env bash

file="$1"
dir=$(dirname "$0")
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." &>/dev/null && pwd)"
echo "[i] Cleaning old build..."
rm -rf $ROOT_DIR/build
echo "[i] Starting build"
mkdir $ROOT_DIR/build
cd $ROOT_DIR/build
../configure --target-list="aarch64-softmmu" --enable-fdt --disable-kvm --disable-xen --enable-gcrypt --disable-werror 
# Problem with qemu keybinds and how the arabic keybind works: https://patchew.org/QEMU/20230620162024.1132013-1-peter.maydell@linaro.org/
sed -i -e "s/'ar': '-l ar'/'ar': '-l ara'/" $ROOT_DIR/pc-bios/keymaps/meson.build
make -j$(nproc)
echo "[+] Completed"
