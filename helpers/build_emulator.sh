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
make -j$(nproc)
echo "[+] Completed"
