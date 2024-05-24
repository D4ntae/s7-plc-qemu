dir=$(dirname ${BASH_SOURCE[0]})
$dir/../build/aarch64-softmmu/qemu-system-aarch64 \
    -M arm-generic-fdt \
    -device loader,file=./binaries/main.fw,addr=0x40000,cpu-num=4,force-raw=true \
    -device loader,file=./binaries/bootloader,addr=0x0,cpu-num=4,force-raw=true \
    -device loader,addr=0XFF5E023C,data=0x80088fde,data-len=4 \
    -device loader,addr=0xff9a0000,data=0x80000218,data-len=4 \
    -display none \
    -monitor stdio \
    -hw-dtb ./binaries/board-updated.dtb \
    -qmp tcp:localhost:4444,server,nowait \
    -S \
    -m 4G -singlestep -d in_asm,nochain,int,guest_errors -s 2>&1 | tee log.txt


