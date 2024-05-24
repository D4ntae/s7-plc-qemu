#include "qemu/osdep.h"
#include "qapi/error.h" /* provides error_fatal() handler */
#include "hw/sysbus.h" /* provides all sysbus registering func */
#include "qemu/log.h"
 
#define TYPE_FL_MEMCTL "xlnx.plc_fl_memctl"
typedef struct FlMemctlState FlMemctlState;
DECLARE_INSTANCE_CHECKER(FlMemctlState, PLC_FL_MEMCTL, TYPE_FL_MEMCTL)

#define CHIP_ID	0xBA000001

struct FlMemctlState {
	SysBusDevice parent_obj;
	MemoryRegion iomem;
	uint64_t chip_id;
};

static uint64_t fl_read(void *opaque, hwaddr addr, unsigned int size) {
    qemu_log_mask(LOG_GUEST_ERROR, "%s: read: addr=0x%x\n",
                  __func__, (int)addr);
    return 0x0;
}

static void fl_write(void *opaque, hwaddr addr, uint64_t val64, unsigned int size) {
    /*dummy code for future development*/
    FlMemctlState *s = opaque;
    uint32_t value = val64;
    unsigned char ch = value;
    (void)s;
    (void)ch;
    qemu_log_mask(LOG_GUEST_ERROR, "%s: write: addr=0x%x v=0x%x\n",
                  __func__, (int)addr, (int)value);
}

static const MemoryRegionOps fl_ops = {
    .read = fl_read,
    .write = fl_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void fl_memctl_init(Object *obj) {
	FlMemctlState *s = PLC_FL_MEMCTL(obj);

	/* allocate memory map region */
	memory_region_init_io(&s->iomem, obj, &fl_ops, s, TYPE_FL_MEMCTL, 0x1000);
	sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);

	s->chip_id = CHIP_ID;
}

static const TypeInfo plc_fl_memctl = {
	.name = TYPE_FL_MEMCTL,
	.parent = TYPE_SYS_BUS_DEVICE,
	.instance_size = sizeof(FlMemctlState),
	.instance_init = fl_memctl_init,
};

static void plc_fl_memctl_register_types(void)
{
    type_register_static(&plc_fl_memctl);
}

type_init(plc_fl_memctl_register_types)
/*
 * Create the PLC DDR MEMCTL device.
*/
DeviceState *plc_fl_memctl_create(hwaddr addr)
{
	DeviceState *dev = qdev_new(TYPE_FL_MEMCTL);
	sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
	sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, addr);
	return dev;
}

