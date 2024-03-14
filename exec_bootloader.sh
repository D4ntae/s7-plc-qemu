./build/aarch64-softmmu/qemu-system-aarch64  \
-M arm-generic-fdt \
-serial mon:stdio \
-device loader,file=./binaries/4.5.2.boot.rev,cpu-num=4 \
-device loader,addr=0XFF5E023C,data=0x80088fde,data-len=4 \
-device loader,addr=0xff9a0000,data=0x80000218,data-len=4 \
-hw-dtb ./binaries/board-zynqmp-zcu1285.dtb \
-m 4G -singlestep -d in_asm 2>&1 | tee -a siemens-asm-rev.log
