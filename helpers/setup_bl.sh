#!/usr/bin/env bash

file="$1"
dir=$(dirname "$0")
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." &>/dev/null && pwd)"
offsets=(0x1b8 0x230 0x23c 0x244);
if [[ -z "$file" ]]; then
    echo "Usage: setup_bl.sh <path_to_bootloader>"
    exit 1
fi

echo "[i] Cutting bootloader size to 0x40000"
dd if="$file" of="/tmp/bootloader.cut" bs=1 count=262144
if [ $? -eq 0 ];then
    echo "[+] Completed"
else
    echo "[-] Error cutting bootloader size"
    exit 1
fi

echo "[i] Converting to little endian"
$dir/byte_swap /tmp/bootloader.cut 32
if [ $? -eq 0 ];then
    echo "[+] Completed"
else
    echo "[-] Error converting to little endian"
    rm /tmp/bootloader.cut
    exit 1
fi

echo "[i] Removing processor specific instructions"
for i in ${offsets[@]}
do
    printf '\x00\x00\x00\x00' | dd of=/tmp/bootloader.cut.rev seek=$((i)) bs=1 conv=notrunc
done

if [ $? -eq 0 ];then
    echo "[+] Completed"
else
    echo "[-] Error removing instructions."
    rm /tmp/bootloader.cut /bootloader.cut.rev
    exit 1
fi

mv /tmp/bootloader.cut.rev $ROOT_DIR/binaries/bootloader
if [ $? -eq 0 ];then
    echo "[+] Completed"
else
    echo "[-] Error moving file."
    rm /tmp/bootloader.cut /bootloader.cut.rev
    exit 1
fi

echo "[i] Cleaning up..."
rm /tmp/bootloader.cut 
if [ $? -eq 0 ];then
    echo "[+] Finished."
    echo "[i] The bootloader file is ready in binaries/bootloader"
else
    echo "[-] Error cleaning up."
    exit 1
fi
