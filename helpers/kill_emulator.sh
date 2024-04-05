#!/usr/bin/env bash

pid=$(pgrep -f "qemu-system-aarch64")
ps -p $pid
echo "Are you sure you want to kill the above process? [Y/n]"
read choice
if [[ "$choice" == "Y" || "$choice" == "y" ]]; then
    kill -9 $pid
    echo "[+] Terminated emulator."
fi
