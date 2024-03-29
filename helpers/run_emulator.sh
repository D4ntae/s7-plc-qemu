dir=$(dirname ${BASH_SOURCE[0]})
$dir/../build/aarch64-softmmu/qemu-system-aarch64 \
    -M arm-generic-fdt \
    -serial mon:stdio \
    -device loader,file=./binaries/main.fw,addr=0x40000,cpu-num=4,force-raw=true \
    -device loader,file=./binaries/bootloader,addr=0x0,cpu-num=4,force-raw=true \
    -device loader,addr=0XFF5E023C,data=0x80088fde,data-len=4 \
    -device loader,addr=0xff9a0000,data=0x80000218,data-len=4 \
    -hw-dtb ./binaries/board-zynqmp-zcu1285.dtb \
    -m 4G -singlestep -d in_asm,nochain -s 2>&1 | tee log.txt


