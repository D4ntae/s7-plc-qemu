# S7 PLC EMULATION README

This is a fork of Xilinx's QEMU fork, used for running experiments on
Siemens PLC firmware in an emulated environment. This work was done as part
of a master's thesis project at the University of Zagreb, Faculty of Electrical Engineering and Computing, under the mentorship of Stjepan Groš, doc. dr. sc.
This repository builds on the fork by adding custom QEMU devices that the Siemens S7-1200 PLC communicates with via its firmware and bootloader.

#### Table of contents
- [Quickstart](#quickstart)
- [Build](#build)
- [Using the emulator](#using_the_emu)
- [Bootloader](#bootloader)
- [Firmware](#firmware)
- [Contributing](#contributing)
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

#### 4. Getting the bootloader
If you want the bootloader file contact me directly.

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
To execute the emulator run the below command or use the helper script `run_emulator.sh`. A detailed explanation can be found in the [Using the emulator](#using_the_emulator) section.
```shell
./build/aarch64-softmmu/qemu-system-aarch64 \
    -M arm-generic-fdt \
    -serial mon:stdio \
    -device loader,file=./binaries/bootloader,addr=0x0 \
    -device loader,file=./binaries/4.5.2.fw.cut,addr=0x40000,cpu-num=4,force-raw=true \
    -device loader,addr=0XFF5E023C,data=0x80088fde,data-len=4 \
    -device loader,addr=0xff9a0000,data=0x80000218,data-len=4 \
    -hw-dtb ./binaries/board-zynqmp-zcu1285.dtb \
    - display none \
    -m 4G -singlestep -d in_asm,nochain -s 2>&1 | tee -a log.txt
```

#### 9. Understanding the output
If everything went according to plan the you should see in your terminal instructions executing in a terminal. These instruction are also saved to a file log.txt. Instructions in the address range from 0-0x15000 are generally in the bootloader except in the later stages where the instructions from 0-0x8000 are from exec_in_lomem. Instructions in the range > 0x40000 are firmware instructions. If you see the emulator going to and instruction range beginning with 0xff that means an exception has occured in execution.

<a id="using_the_emu"><a/>
## Using the emulator
#### Useful info on using the emulator
Sometimes certain instructions apear to hang the emulator. When this happens it means there is an error but you need to let the emulator run for a bit before you see this error.

If you see a bunch of instructions executing that start with 0xff that means an exception has occured at the last instruction not beginning with 0xff.
#### Changing logging settings based on your needs
The logfile is a plaintext file that shows arm instructions (assuming you are using the run_emulator script) that are executed on the emulator. If you want to change this behaviour (add new output or remove some output) you need to change the flags in the run_emulator script. The flags you need to change involve the `-d asm,nochain` part of the execution flags of qemu. All the possible flags can be found in the table below.
###### Table of possible debug options
| Option | Description |
| ---- | ---- |
|out_asm | show generated host assembly code for each compiled TB |
| in_asm |  show target assembly code for each compiled TB |
| op |  show micro ops for each compiled TB |
| op_opt | show micro ops after optimization |
| op_ind | show micro ops before indirect lowering |
| int | show interrupts/exceptions in short format |
| exec | show trace before each executed TB (lots of logs) |
| cpu | show CPU registers before entering a TB (lots of logs) |
| fpu | include FPU registers in the 'cpu' logging |
| mmu | log MMU-related activities |
| pcall | x86 only: show protected mode far calls/returns/exceptions |
| cpu_reset | show CPU state before CPU resets |
| unimp | log unimplemented functionality |
| guest_errors | log when the guest OS does something invalid (eg accessing a non-existent register) |
| page | dump pages at beginning of user mode emulation |
| nochain | do not chain compiled TBs so that exec and cpu show complete traces |
| plugin |  output from TCG plugins|
| strace | log every user-mode syscall|
| tid | open a separate log file per thread; filename must contain '%d' |
| vpu | include VPU registers in the 'cpu' logging |

#### Using the other available helper scripts
**Note**: This section is subject to change as progress is made on the emulator. We will do our best to keep it up to date.

###### log_to_function_calls.sh
This script expects a default debug log (see section above for more info) and extracts all the function calls from the log file in the format of \<address>: \<function_being_called_address>.

###### kill_emulator.sh 
This scripts lets you kill the emulator if it can't be killed with sending a SIGINT (ctrl + c).

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

###### Updating the exec_in_lomem file
This fix needs to happen for an unknown reason and the fix is almost surely a bad idea. Howerver the exec_in_lomem file crashes at 0x368 at an instruction lsr r0, r0, r4 for some reason. The fix is to change that instruction to a b#380 which skips 5 instructions and continues execution. This makes the firmware continues executing. Currently we don't know why this is happening or why the jump fixes it.

###### Removing broken instructions
Just like the bootloader the firmware has mcr/mrc instructions which break the emulation. The locations for these are 0xcc and 0xd4 in the firmware.
```shell
offsets=(0xcc 0xd4);
for i in ${offsets[@]}
do
    printf '\x00\x00\x00\x00' | dd of=bootloader seek=$((i)) bs=1 conv=notrunc
done
```

## Contributing
#### Adding a new qemu device
Development of the emulator requires the ability to add new devices to QEMU. What this means is being able to model a periferal device (timer, lcd display) or an internal component of the firmware such as the Vector Interrupt Table. Below are steps describing how to add a new device to qemu along with appropriate examples. If something is not clear from the instructions you can consult the official [Xilinx documentation](https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/861569267/QEMU+Device+Model+Development#QEMUDeviceModelDevelopment-Writingyourowndevicemodel). Or [this example](https://github.com/qemu/qemu/compare/master...vppillai:qemu:theButterRobot) of adding a device to qemu (be warned there are some differences in the Xilinx version which is what we are using.)
###### Step 1 - Writing code for the device
We will be using the example of the IOC (Input/Output Controller) from the PLC which is required in some parts of the emulation.  All the code for devices you want to add needs to be in the `hw/misc` directory. Take a look at the `hw/misc/plc_ioc.c` file. This is the file that implements all the funcitionality needed for a device. There are code comments in this file that will explain all the structures and functions in it. For a fully functioning device you need to implement in some shape or form all the structures and functions in this file. After you did that take a look at the `hw/misc/plc_ioc.h` file. You can just copy this file with the name of name of the create function changed.
###### Step 2 - Changing the config files
The files you need to change are as follows. In `hw/misc/Kconfig` add a config option for your new device just like the ones for other PLC components. In `hw/arm/Kconfig` in the config ZYNQ section add a select statement for your new device. **This name needs to be equal to the one used in `hw/misc/Kconfig`**. Last you need to edit `hw/misc/meson.build` and add an entry (around line 305) that is the same as all the other plc entries but for your own device. For the when section just enter 'CONFIG_\<name used in kconfig>'. All that these config files are doing is telling the compiler to include your file in the compilation process.
###### Step 3 - Modifying the Device Tree Binary (DTB)
The Device Tree Binary is a file that describes to emulator how the memory map of the system works and what to access in each memory location. To edit the DTB you can you can edit the `binaries/output.dst` file. Find the section which maps all the custom PLC memory (you can search for the word plc). In that section you will see mapping that look like this:
```
// name_of_map@start_address
plc_ioc@0xfffbc000 {
	compatible = "xlnx,plc_ioc"; // this line needs to look like "xlnx,<name_used_in_type_in_c_file>"
	reg = <0x00 0xfffbc000 0x00 0x2c 0x00>; // The second value is the start address, the fourth is the size of the region
	phandle = <0xc3>; // this line is automatically made by the compiler, remove it in your code
};
```
After you created your new section you need to compile the DTB. To do this you will need `dtc`. `dtc` Can be installed on debian based systems with the command `sudo apt install device-tree-compiler` and on arch based distros with the `sudo pacman -S dtc` command.
To compile the DTB run the command:
```bash
dtc -I dts -O dtb -o board-updated.dtb output.dst
```
###### Step 4 - Rebuilding the emulator
Now you can just navigate to the build directory of the emulator and run `make -j$(nproc)` to rebuild the emulator. After it has finished building you should see your device being used in the emulator when its memory region is accessed.

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
