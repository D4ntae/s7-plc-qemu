#!/usr/bin/env bash

file="$1"
dir=$(dirname "${BASH_SOURCE[0]}")
offsets=(0xcc 0xd4);
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." &>/dev/null && pwd)"
if [[ -z "$file" ]]; then
    echo "Usage: setup_fw.sh <path_to_firmware>"
    exit 1
fi

echo "[i] Removing header"
dd if="$file" of="/tmp/firmware.cut" bs=1 skip=64
if [ $? -eq 0 ];then
    echo "[+] Completed"
else
    echo "[-] Error removing header"
    exit 1
fi

echo "[i] Converting to little endian"
$dir/byte_swap /tmp/firmware.cut 32
if [ $? -eq 0 ];then
    echo "[+] Completed"
else
    echo "[-] Error converting to little endian"
    rm /tmp/firmware.cut
    exit 1
fi

echo "[i] Creating exec_in_lomem file"
dd if=/tmp/firmware.cut.rev of=/tmp/exec_in_lomem.lo count=30720 bs=1

echo "[i] Fixing the lsr instruction"
printf '\x04\x00\x00\xEA' | dd of=/tmp/exec_in_lomem.lo seek=368 bs=1 conv=notrunc

if [ $? -eq 0 ];then
    echo "[+] Completed"
else
    echo "[-] Error creating file."
    rm /tmp/firmware.cut.rev /tmp/firmware.cut
    exit 1
fi

echo "[i] Creating main firmware file"
dd if=/tmp/firmware.cut.rev of=/tmp/main.fw skip=32768 bs=1

if [ $? -eq 0 ];then
    echo "[+] Completed"
else
    echo "[-] Error creating file."
    rm /tmp/firmware.cut.rev /tmp/firmware.cut /tmp/exec_in_lomem.lo
    exit 1
fi

echo "[i] Fixing mcr/mrc instructions"
for i in ${offsets[@]}
do
    printf '\x00\x00\x00\x00' | dd of=/tmp/main.fw seek=$((i)) bs=1 conv=notrunc
done

echo "[i] Moving files to correct directory."
mv /tmp/main.fw $ROOT_DIR/binaries/main.fw
mv /tmp/exec_in_lomem.lo $ROOT_DIR/binaries/exec_in_lomem.lo
if [ $? -eq 0 ];then
    echo "[+] Completed"
else
    echo "[-] Error moving file."
    rm /tmp/firmware.cut.rev /tmp/firmware.cut /tmp/exec_in_lomem.lo /tmp/main.fw
    exit 1
fi

echo "[i] Cleaning up..."
rm /tmp/firmware.cut.rev /tmp/firmware.cut 
if [ $? -eq 0 ];then
    echo "[+] The firmware files are ready in binaries/main.fw and binaries/exec_in_lomem.lo"
else
    echo "[-] Error cleaning up."
    exit 1
fi

echo "[i] Updating driver file with correct file path"
sed -i "s|#define EXEC_IN_LOMEM_FILENAME .*|#define EXEC_IN_LOMEM_FILENAME \"$ROOT_DIR/binaries/exec_in_lomem.lo\"|" $ROOT_DIR/hw/misc/plc_80280000.c

if [ $? -eq 0 ];then
    echo "[+] Finished setting up firmware."
else
    echo "[-] Error updating file."
    exit 1
fi
