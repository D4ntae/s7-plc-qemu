#include "qemu/osdep.h"
#include "qapi/error.h" /* provides error_fatal() handler */
#include "hw/sysbus.h" /* provides all sysbus registering func */

#include "qemu/log.h"
#define TYPE_PLC_IOC "xlnx.plc_ioc"
typedef struct PLCIOCState PLCIOCState;
DECLARE_INSTANCE_CHECKER(PLCIOCState, PLC_IOC, TYPE_PLC_IOC)

#define REG_ID 	0x0
#define CHIP_ID	0xBA000001

struct PLCIOCState {
	SysBusDevice parent_obj;
	MemoryRegion iomem;
	uint64_t chip_id;
};

static uint64_t plc_ioc_read(void *opaque, hwaddr addr, unsigned int size)
{
	switch (addr) {
	default:
		return 0x00000100;
	}
}
static void plc_ioc_write(void *opaque, hwaddr addr, uint64_t val64, unsigned int size)
{
    PLCIOCState *s = opaque;
    unsigned char ch = val64;
    (void)s;
    (void)ch;
    qemu_log_mask(LOG_GUEST_ERROR, "%s: write: addr=0x%x v=0x%x\n",
                  __func__, (int)addr, (int)val64);
}

static const MemoryRegionOps plc_ioc_ops = {
	.read = plc_ioc_read,
    .write = plc_ioc_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
};



static void plc_ioc_instance_init(Object *obj)
{
	PLCIOCState *s = PLC_IOC(obj);

	/* allocate memory map region */
	memory_region_init_io(&s->iomem, obj, &plc_ioc_ops, s, TYPE_PLC_IOC, 0x2c);
	sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);

	s->chip_id = CHIP_ID;
}

/* create a new type to define the info related to our device */
static const TypeInfo plc_ioc_info = {
	.name = TYPE_PLC_IOC,
	.parent = TYPE_SYS_BUS_DEVICE,
	.instance_size = sizeof(PLCIOCState),
	.instance_init = plc_ioc_instance_init,
};

static void plc_ioc_register_types(void)
{
    type_register_static(&plc_ioc_info);
}

type_init(plc_ioc_register_types)

/*
 * Create the PLC IOC device.
*/
DeviceState *plc_ioc_create(hwaddr addr)
{
	DeviceState *dev = qdev_new(TYPE_PLC_IOC);
	sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
	sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, addr);
	return dev;
} 
