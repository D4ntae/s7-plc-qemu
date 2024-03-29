# S7 PLC EMULATION README

This is a fork of Xilinx's QEMU fork, used for running experiments on
Siemens PLC firmware in an emulated environment. This work was done as part
of a master's thesis project at the University of Zagreb, Faculty of Electrical Engineering and Computing, under the mentorship of Stjepan Groš, doc. dr. sc.
This repository builds on the fork by adding custom QEMU devices that the Siemens S7-1200 PLC communicates with via its firmware and bootloader.

#### Table of contents
- [Quickstart](#quickstart)
- [Build](#build)
- [Bootloader](#bootloader)
- [Firmware](#firmware)
- [QEMU Readme](#qemu_readme)

## Quickstart
#### Preface
All the scripts listed here expect the firmware version 4.5.2 (provided in the link) and the bootloader provided in the link. No guarantees are made that this will work for any other version. If you want it to work for a different version check out the in depth description of the setup and adjust it for you version.
#### 1. Clone this repo and install dependencies
```shell
# HTTPS
git clone https://github.com/D4ntae/s7-plc-qemu.git

# SSH
git clone git@github.com:D4ntae/s7-plc-qemu.git

```
Dependency list (might differ based on distribution)
> gcc g++ make ninja-build glib2-devel pixman-devel libgcrypt-devel qemu git
#### 2. Download the firmware
Official download link
> https://support.industry.siemens.com/cs/attachments/107539957/6ES7_212-1BE40-0XB0_V04.05.02.zip

To download the file you will need to make an account and agree to the download requirements.
Once downloaded decompress the file and use the .upd file for the next step.

#### 3. Extracting the firmware
Clone this repo 
> https://gitlab.com/lgrguric/siemens_lzp3

Use the provided python file to extract the firmware from the .upd file.
```shell
# Usage
python fw_decompress.py <input.upd> <output.fw>

# Example
python fw_decompress.py '6ES7 214-1AG40-0XB0 V04.01.02.upd' 4.5.2.fw
```

#### 4. Downloading the bootloader
The download link for the already extracted raw bootload 
> https://mega.nz/file/zjQTibzB#km4ryb6QBF8qnA8mk50FdkTwyZSyCo5efLDW43ZR1dI

If you want more info on extracting the bootloader from the PLC directly take a look at the [Bootloader](#bootloader) section.

#### 5. Setting up the firmware file
There is quite an extensive setup needed to get the firmware ready for emulation and this setup is explained more in depth in the [Firmware](#firmware) section. However there is a convenience script proveded in the helpers directory of this repository `setup_fw.sh`. The use the script pass in the path to the raw firmware file as the first argument. This will create a ready to use firmware file in the ./binaries directory and modify the necessary files to run the emulator. 
```shell
# Usage
helpers/setup_fw.sh <path_to_raw_firmware_file>

# Example
helpers/setup_fw.sh ~/Downloads/4.5.2.fw
```

**Note**: The script works non desctructively and as such does a lot of copying of files. So it might seem like its hanging at certain parts but thats normal let it work.

#### 6. Setting up the bootloader file
As with the firmware there is setup required to get the bootloader ready for emulation. The in depth breakdown can be seen in the [Bootloader](#bootloader) section. Once again there is a convenience script `setup_bl.sh` that requires the path to the raw bootloader file (either from the link or the PLC). The ready to use bootloader file will be located in the ./binaries directory. 

```shell
# Usage
helpers/setup_bl.sh <path_to_raw_bootloader_file>

# Example
helpers/setup_bl.sh ~/Downloads/4.5.2.boot
```

**Note**: The script works non desctructively and as such does a lot of copying of files. So it might seem like its hanging at certain parts but thats normal let it work.

#### 7. Build the emulator 
Run the below commands or use the helper script `build_emulator.sh`.
``` shell
mkdir build && cd build 
../configure --target-list="aarch64-softmmu" --enable-fdt --disable-kvm --disable-xen --enable-gcrypt --disable-werror 
make -j$(nproc)
```

#### 8. Run the emulator
To execute the emulator run the below command or use the helper script `run_emulator.sh`. A detailed explanation can be found in the [Running the emulator](#running_the_emulator) section.
```shell
./build/aarch64-softmmu/qemu-system-aarch64 \
    -M arm-generic-fdt \
    -serial mon:stdio \
    -device loader,file=./binaries/bootloader,addr=0x0 \
    -device loader,file=./binaries/4.5.2.fw.cut,addr=0x40000,cpu-num=4,force-raw=true \
    -device loader,addr=0XFF5E023C,data=0x80088fde,data-len=4 \
    -device loader,addr=0xff9a0000,data=0x80000218,data-len=4 \
    -hw-dtb ./binaries/board-zynqmp-zcu1285.dtb \
    -m 4G -singlestep -d in_asm,nochain -s 2>&1 | tee -a log.txt
```

#### 9. Understanding the output
If everything went according to plan the you should see in your terminal instructions executing and a qemu window pop up. These instruction are also saved to a file log.txt. Instructions in the address range from 0-0x15000 are generally in the bootloader except in the later stages where the instructions from 0-0x8000 are from exec_in_lomem. Instructions in the range > 0x40000 are firmware instructions. If you see the emulator going to and instruction range beginning with 0xff that means an exception has occured in execution.

## Bootloader
#### Getting the bootloader from the chip

First, it is required to get the bootloader of the PLC itself. This can be quite tricky, but doable using a
device such as [Bus Pirate](http://dangerousprototypes.com/docs/Bus_Pirate>) and an appropriate chip programming software, e.g. [flashrom](https://www.flashrom.org/Bus_Pirate>). Since flashrom doesn’t include support for the S7-1200 bootloader chip, one needs to modify the code to include it. It is recommended to first just add the chip ID in the flashchips.h file so the chip is recognized. Once that is done, one only needs to copy a similar chip’s configuration and add an entry in the flaschips.c file. Lastly, a very specific modification needs to be made. The parameter chunk_size in the spi_read_chunked function needs to be changed to the constant 256. Once this is done, one can connect Bus Pirate to the chip and read the contents using flashrom.

```shell
sudo ./flashrom -p buspirate_spi:dev=/dev/ttyUSB0,spispeed=1M --read bootloader.bin --chip <CHIP_NAME>
```

The argument dev should be the USB port on which Bus Pirate is connected to the PC. If something goes wrong, the argument -VVV can be added to get a detailed overview of what happened

#### Settting up the bootloader for emulation
###### Cutting the bootloaders size
The bootloader is padded with 0s to the size of 0x80000 (512KB) bytes. However the firmware stars its execution at the memory address of 0x40000 which means we need to cut the padding from the bootloader and reduce its size down to 0x40000 bytes so that it doesn't overlap with the firmware.
```shell
dd if=bootloader of=bootloader.cut bs=1 count=262144
```

###### Reversing the byte order
Our qemu emulator only supports little endian instructions however the processor in the PLC is an ARMv7 Cortex R4 which is big endian. To adjust for this inconvenience we swap all the bytes of the bootloader to be little endian instead. This will probably effect some data that is 2 byte instead of 4 byte aligned but this will be fixed in the future if it becomes a problem. To do this you can use this [open source program](https://github.com/rgrahamh/byte-swapper) or use the binary in helpers/byte_swap.
```shell
# Usage
./byte_swap <input_file> <block_size_bits>

# Example
./byte_swap bootloader 32
```

###### Replacing broken instructions
As the qemu emulator we have is based on the Cortex R5 and our bootloader is written for the Cortex R4 there are processor specific instructions which break execution when reached. Such instructions are the MCR and MRC instructions which deal with the coproccesor ([ARM Documentation](https://developer.arm.com/documentation/dui0068/b/ARM-Instruction-Reference/ARM-coprocessor-instructions/MCR--MCR2--MCRR)). These instructions will be overwritten with NOP (mov r0, r0, r0) instructions so that the emulation can continue. The locations of these instructions shouldn't change however the locations given are for the version provided at the [link](https://mega.nz/file/zjQTibzB#km4ryb6QBF8qnA8mk50FdkTwyZSyCo5efLDW43ZR1dI) and are located at 0x230, 0x23c and 0x244. Another instruction that breaks is the setend be instruction which sets the processor to big endian mode. This is an instruction our processor cannot execute and so it needs to be NOPed. The instruction is located at 0x1b8.
The command to replace the values.
```shell
offsets=(0x1b8 0x230 0x23c 0x244);
for i in ${offsets[@]}
do
    printf '\x00\x00\x00\x00' | dd of=bootloader seek=$((i)) bs=1 conv=notrunc
done
```


## Firmware
#### Firmware structure
The contents of the firmware on the NAND chip can be separated into 3 distinct areas - the first 0x40 bytes containing the header with firmware metadata, the following 0x8000 bytes which make up the .exec_in_lomem section, and the rest of the firmware, the size of which varies, but is around 0x1100000 bytes. On the actual PLC, its bootloader has the role of copying the firmware from the NAND chip on the PLC to RAM and ITCM. From these 3 areas, only the latter 2 will be copied to RAM and ITCM. For emulation purposes, RAM and ITCM can be understood to be one and the same and shall be collectively referred to as simply RAM. The following image shows how the bootloader copies the contents of the chip to RAM.

![alt text](https://raw.githubusercontent.com/D4ntae/s7-plc-qemu/master/assets/memory_layout.jpeg "Memory Layout")

We need to emulate this (that is, copying the firmware to RAM) behavior. To do this, one needs to separate the different parts of the firmware update file - throw out the first 0x40 bytes (the header), save the next 0x8000 bytes in one file (exec_in_lomem), and finally save the rest in another file (main_firmware). The exec_in_lomem section is manually copied over a part of the bootloader code at address 0x0 once the bootloader itself is finished with its execution. The main_firmware section, which contains most of the firmware logic, is loaded to address 0x40000 using QEMU’s device loader.

#### Setting up the firmware for emulation
###### Seperating the firmware into two files
As stated in the Firmware structure section we need to split the firmware into two files, the exec_in_lomem file and the main firmware file. We also need to remove the header from the firmware file.
```shell
# Remove header
dd if=firmware skip=64 bs=1 of=firmware.no_head

# Remove exec_in_lomem part
dd if=firmware.no_head count=32768 bs=1 of=exec_in_lomem.lo

# Only the main firmware
dd if=firmware.no_head skip=32768 bs=1 of=main.fw
```

###### Updating the path to the exec_in_lomem file
The exec_in_lomem file is explained above and now we need to setup the device that will copy it into memory. That device is defined in hw/misc/plc_80280000.c and the define macro EXEC_IN_LOMEM_FILENAME needs to be set to the full path to the exec_in_lomem file.
```c
#define EXEC_IN_LOMEM_FILE '/home/user/s7-plc-qemu/binaries/exec_in_lomem.lo'
```


<a id="running_the_emulator"><a/>
## Running the emulator
#### OLD README, WILL BE REMOVED SOON 

Download the bootloader and firmware binaries from this link:
<https://mega.nz/folder/Sr5D0BaK#d6AvUZgDgI69LmYE0qvVwA> and put them in
the root directory of this repo. The update the file
hw/misc/plc_80280000.c and change the EXEC_IN_LOMEM_FILENAME macro to
the full path to the exec_in_lomem.fw binary.

Cut bootloader size to 0x40000 beacause otherwise the memory regions
overlap and the bootloader is not loaded. .. code-block:: shell

> dd if=bootloader.rev of=bootloader.rev.cut bs=1 count=262144

From the firmware file the first 64 bytes need to be cut. In the
bootloader file on addresses 0x230, 0x23c, 0x244 4 bytes need to be
replaced with 0.

Instruction at 0x368 in exec_in_lomem.rev was changed to DE0000EA. This
is probably bad but works for now.

To run the emulator run the command .. code-block:: shell

> ./build/aarch64-softmmu/qemu-system-aarch64 -M arm-generic-fdt -serial
> mon:stdio -device
> loader,file=./binaries/4.5.2.fw-no-start.rev,addr=0x40000,cpu-num=4,force-raw=true
> -device loader,file=./binaries/4.5.2.boot.rev,addr=0x0 -device
> loader,addr=0XFF5E023C,data=0x80088fde,data-len=4 -device
> loader,addr=0xff9a0000,data=0x80000218,data-len=4 -hw-dtb
> ./binaries/board-zynqmp-zcu1285.dtb -m 4G -singlestep -d
> in_asm,nochain -s \> log.txt

<a id="qemu_readme"></a>
# QEMU README

QEMU is a generic and open source machine & userspace emulator and
virtualizer.

QEMU is capable of emulating a complete machine in software without any
need for hardware virtualization support. By using dynamic translation,
it achieves very good performance. QEMU can also integrate with the Xen
and KVM hypervisors to provide emulated hardware while allowing the
hypervisor to manage the CPU. With hypervisor support, QEMU can achieve
near native performance for CPUs. When QEMU emulates CPUs directly it is
capable of running operating systems made for one machine (e.g. an ARMv7
board) on a different machine (e.g. an x86_64 PC board).

QEMU is also capable of providing userspace API virtualization for Linux
and BSD kernel interfaces. This allows binaries compiled against one
architecture ABI (e.g. the Linux PPC64 ABI) to be run on a host using a
different architecture ABI (e.g. the Linux x86_64 ABI). This does not
involve any hardware emulation, simply CPU and syscall emulation.

QEMU aims to fit into a variety of use cases. It can be invoked directly
by users wishing to have full control over its behaviour and settings.
It also aims to facilitate integration into higher level management
layers, by providing a stable command line interface and monitor API. It
is commonly invoked indirectly via the libvirt library when using open
source applications such as oVirt, OpenStack and virt-manager.

QEMU as a whole is released under the GNU General Public License,
version 2. For full licensing details, consult the LICENSE file.

## Documentation

Documentation can be found hosted online at
<https://www.qemu.org/documentation/>. The documentation for the current
development version that is available at
<https://www.qemu.org/docs/master/> is generated from the `docs/` folder
in the source tree, and is built by
[Sphinx](https://www.sphinx-doc.org/en/master/).

## Building

QEMU is multi-platform software intended to be buildable on all modern
Linux platforms, OS-X, Win32 (via the Mingw64 toolchain) and a variety
of other UNIX targets. The simple steps to build QEMU are:

``` shell
mkdir build
cd build
../configure
make
```

Additional information can also be found online via the QEMU website:

-   <https://wiki.qemu.org/Hosts/Linux>
-   <https://wiki.qemu.org/Hosts/Mac>
-   <https://wiki.qemu.org/Hosts/W32>

## Submitting patches

The QEMU source code is maintained under the GIT version control system.

``` shell
git clone https://gitlab.com/qemu-project/qemu.git
```

When submitting patches, one common approach is to use 'git
format-patch' and/or 'git send-email' to format & send the mail to the
<qemu-devel@nongnu.org> mailing list. All patches submitted must contain
a 'Signed-off-by' line from the author. Patches should follow the
guidelines set out in the [style
section](https://www.qemu.org/docs/master/devel/style.html) of the
Developers Guide.

Additional information on submitting patches can be found online via the
QEMU website

-   <https://wiki.qemu.org/Contribute/SubmitAPatch>
-   <https://wiki.qemu.org/Contribute/TrivialPatches>

The QEMU website is also maintained under source control.

``` shell
git clone https://gitlab.com/qemu-project/qemu-web.git
```

-   <https://www.qemu.org/2017/02/04/the-new-qemu-website-is-up/>

A 'git-publish' utility was created to make above process less
cumbersome, and is highly recommended for making regular contributions,
or even just for sending consecutive patch series revisions. It also
requires a working 'git send-email' setup, and by default doesn't
automate everything, so you may want to go through the above steps
manually for once.

For installation instructions, please go to

-   <https://github.com/stefanha/git-publish>

The workflow with 'git-publish' is:

``` shell
$ git checkout master -b my-feature
$ # work on new commits, add your 'Signed-off-by' lines to each
$ git publish
```

Your patch series will be sent and tagged as my-feature-v1 if you need
to refer back to it in the future.

Sending v2:

``` shell
$ git checkout my-feature # same topic branch
$ # making changes to the commits (using 'git rebase', for example)
$ git publish
```

Your patch series will be sent with 'v2' tag in the subject and the git
tip will be tagged as my-feature-v2.

## Bug reporting

The QEMU project uses GitLab issues to track bugs. Bugs found when
running code built from QEMU git or upstream released sources should be
reported via:

-   <https://gitlab.com/qemu-project/qemu/-/issues>

If using QEMU via an operating system vendor pre-built binary package,
it is preferable to report bugs to the vendor's own bug tracker first.
If the bug is also known to affect latest upstream code, it can also be
reported via GitLab.

For additional information on bug reporting consult:

-   <https://wiki.qemu.org/Contribute/ReportABug>

## ChangeLog

For version history and release notes, please visit
<https://wiki.qemu.org/ChangeLog/> or look at the git history for more
detailed information.

## Contact

The QEMU community can be contacted in a number of ways, with the two
main methods being email and IRC

-   [mailto:qemu-devel@nongnu.org](mailto:qemu-devel@nongnu.org)
-   <https://lists.nongnu.org/mailman/listinfo/qemu-devel>
-   \#qemu on irc.oftc.net

Information on additional methods of contacting the community can be
found online via the QEMU website:

-   <https://wiki.qemu.org/Contribute/StartHere>
