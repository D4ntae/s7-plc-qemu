#include "qemu/osdep.h"
#include "qapi/error.h" /* provides error_fatal() handler */
#include "hw/sysbus.h" /* provides all sysbus registering func */
#include "hw/misc/plc_ddram.h"

#include "qemu/log.h"
#define TYPE_PLC_DDRAM "xlnx.plc_ddram"
typedef struct PLCDDRAMState PLCDDRAMState;
DECLARE_INSTANCE_CHECKER(PLCDDRAMState, plc_ddram, TYPE_plc_ddram)

#define REG_ID 	0x0
#define CHIP_ID	0xBA000001

struct PLCDDRAMState {
	SysBusDevice parent_obj;
	MemoryRegion iomem;
	uint64_t chip_id;
};

static uint64_t plc_ddram_read(void *opaque, hwaddr addr, unsigned int size)
{
	switch (addr) {
	default:
		return 0x00004000;
	}
}

static void plc_ddram_write(void *opaque, hwaddr addr, uint64_t val64, unsigned int size)
{
    PLCDDRAMState *s = opaque;
    uint32_t value = val64;
    unsigned char ch = value;
    (void)s;
    (void)ch;
    qemu_log_mask(LOG_GUEST_ERROR, "%s: write: addr=0x%x v=0x%x\n",
                  __func__, (int)addr, (int)value);
}

static const MemoryRegionOps plc_ddram_ops = {
	.read = plc_ddram_read,
    .write = plc_ddram_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
};



static void plc_ddram_instance_init(Object *obj)
{
	PLCDDRAMState *s = plc_ddram(obj);

	/* allocate memory map region */
	memory_region_init_io(&s->iomem, obj, &plc_ddram_ops, s, TYPE_plc_ddram, 0x2c);
	sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);

	s->chip_id = CHIP_ID;
}

/* create a new type to define the info related to our device */
static const TypeInfo plc_ddram_info = {
	.name = TYPE_plc_ddram,
	.parent = TYPE_SYS_BUS_DEVICE,
	.instance_size = sizeof(PLCDDRAMState),
	.instance_init = plc_ddram_instance_init,
};

static void plc_ddram_register_types(void)
{
    type_register_static(&plc_ddram_info);
}

type_init(plc_ddram_register_types)

/*
 * Create the PLC DDRAM device.
 */
DeviceState *plc_ddram_create(hwaddr addr)
{
	DeviceState *dev = qdev_new(TYPE_plc_ddram);
	sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
	sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, addr);
	return dev;
} 
