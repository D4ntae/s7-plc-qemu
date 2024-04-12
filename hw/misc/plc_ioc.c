#include "qemu/osdep.h"
#include "qapi/error.h" /* provides error_fatal() handler */
#include "hw/sysbus.h" /* provides all sysbus registering func */

#include "qemu/log.h"
#define TYPE_PLC_IOC "xlnx.plc_ioc" /* This type name is very important as the name you use after xlnx. will be the same name you use in the device tree binary later when adding this device. This type is used an indentifier for this device */
typedef struct PLCIOCState PLCIOCState; /* Forward declaration for the state struct which is explained below. */
DECLARE_INSTANCE_CHECKER(PLCIOCState, PLC_IOC, TYPE_PLC_IOC) /* This macro is needed to bind the id TYPE_PLC_IOC to the state object PLCIOState so that it can later be referenced. In a way its creating a constructor for the device that can be used with the PLC_IOC second argument of the macro */

/* Leave these values as they are. */
#define REG_ID 	0x0
#define CHIP_ID	0xBA000001

/* This struct describes your device and how it can be used. For example if you need some more complex logic in you device like defining register it will use to so in this struct. In is a abstract representation of your device. */
struct PLCIOCState {
	SysBusDevice parent_obj; // This is a required field by the Xilinx version of qemu.
	MemoryRegion iomem; // This is also a required field.
	uint64_t chip_id; // So is this.
    // uint_64 reg1; - This would add a register field to your device which you can then write to in the write function or read from in the read function.
};

/* This function defines the behaviour for when memory is read from your device. 
*   Arguments:
*   void *opaque - A pointer to your state structure. In this example a PLCIOCState*
*   hwaddr addr - An offset into your device. For example if your device is mapped at 0xfffa0000 and memory is read from 0xfffa0004, addr will be 4
*   unsigned int size - The size of memory requested
*
*   In this example no matter which part of the ioc is read we always return 0x100 beacause that is all that is needed for the emulator to continue running.
*/
static uint64_t plc_ioc_read(void *opaque, hwaddr addr, unsigned int size)
{
	switch (addr) {
	default:
		return 0x00000100;
	}
}

/* This function defines the behaviour for when memory is written to your device. 
*   Arguments:
*   void *opaque - A pointer to your state structure. In this example a PLCIOCState*
*   hwaddr addr - An offset into your device. For example if your device is mapped at 0xfffa0000 and memory is read from 0xfffa0004, addr will be 4
*   uint64_t val64 - The value written into memory
*   unsigned int size - The size of memory written
*
*   This example doesn't do anything other than print that memory was written to as there is no complex functionality needed. However some devices do require you to keep track of memory written and so you should probably keep 
*   track of that in some variable in your state object.
*/
static void plc_ioc_write(void *opaque, hwaddr addr, uint64_t val64, unsigned int size)
{
    PLCIOCState *s = opaque;
    unsigned char ch = val64;
    (void)s;
    (void)ch;
    qemu_log_mask(LOG_GUEST_ERROR, "%s: write: addr=0x%x v=0x%x\n",
                  __func__, (int)addr, (int)val64);
}

/* This structure describes your read and write functions. Add those to there respective fields. */
static const MemoryRegionOps plc_ioc_ops = {
	.read = plc_ioc_read,
    .write = plc_ioc_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
};

/* The init function is needed so that your device can be mapped correctly to memory. You can leave the code basically the same in every device you just need to change the type and size of your memory region. */
static void plc_ioc_instance_init(Object *obj)
{
	PLCIOCState *s = PLC_IOC(obj);

	/* Allocate memory map region. The last parameter defines the size of your memory region. */
	memory_region_init_io(&s->iomem, obj, &plc_ioc_ops, s, TYPE_PLC_IOC, 0x2c);
	sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);

	s->chip_id = CHIP_ID;
}

/* Create a new type to define the info related to our device. You can just adjust this struct to match your values. */
static const TypeInfo plc_ioc_info = {
	.name = TYPE_PLC_IOC,
	.parent = TYPE_SYS_BUS_DEVICE,
	.instance_size = sizeof(PLCIOCState),
	.instance_init = plc_ioc_instance_init,
};

/* This function can just be copied. */
static void plc_ioc_register_types(void)
{
    type_register_static(&plc_ioc_info);
}

/* This function call registers this device with QEMU core. */
type_init(plc_ioc_register_types)

/*
 * The create functions is basically the same for every device but is needed every time. It is also needed in the matching header file.
*/
DeviceState *plc_ioc_create(hwaddr addr)
{
	DeviceState *dev = qdev_new(TYPE_PLC_IOC);
	sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
	sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, addr);
	return dev;
} 
