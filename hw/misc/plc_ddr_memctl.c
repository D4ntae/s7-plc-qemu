#include "qemu/osdep.h"
#include "qapi/error.h" /* provides error_fatal() handler */
#include "hw/sysbus.h" /* provides all sysbus registering func */
#include "qemu/log.h"
 
#define TYPE_DDR_MEMCTL "xlnx.plc_ddr_memctl"
typedef struct DdrMemctlState DdrMemctlState;
DECLARE_INSTANCE_CHECKER(DdrMemctlState, PLC_DDR_MEMCTL, TYPE_DDR_MEMCTL)

#define CHIP_ID	0xBA000001

struct DdrMemctlState {
	SysBusDevice parent_obj;
	MemoryRegion iomem;
	uint64_t chip_id;
};

static uint64_t ddr_read(void *opaque, hwaddr addr, unsigned int size) {
    switch (addr) {
        case 0xc:
            return 0;
        default:
            return 0x20000000 | 0x400;
    }
}

static void ddr_write(void *opaque, hwaddr addr, uint64_t val64, unsigned int size) {
    /*dummy code for future development*/
    DdrMemctlState *s = opaque;
    uint32_t value = val64;
    unsigned char ch = value;
    (void)s;
    (void)ch;
    qemu_log_mask(LOG_GUEST_ERROR, "%s: write: addr=0x%x v=0x%x\n",
                  __func__, (int)addr, (int)value);
}

static const MemoryRegionOps ddr_ops = {
    .read = ddr_read,
    .write = ddr_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void ddr_memctl_init(Object *obj) {
	DdrMemctlState *s = PLC_DDR_MEMCTL(obj);

	/* allocate memory map region */
	memory_region_init_io(&s->iomem, obj, &ddr_ops, s, TYPE_DDR_MEMCTL, 0x100);
	sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);

	s->chip_id = CHIP_ID;
}

static const TypeInfo plc_ddr_memctl = {
	.name = TYPE_DDR_MEMCTL,
	.parent = TYPE_SYS_BUS_DEVICE,
	.instance_size = sizeof(DdrMemctlState),
	.instance_init = ddr_memctl_init,
};

static void plc_ddr_memctl_register_types(void)
{
    type_register_static(&plc_ddr_memctl);
}

type_init(plc_ddr_memctl_register_types)
/*
 * Create the PLC DDR MEMCTL device.
*/
DeviceState *plc_ddr_memctl_create(hwaddr addr)
{
	DeviceState *dev = qdev_new(TYPE_DDR_MEMCTL);
	sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
	sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, addr);
	return dev;
}

