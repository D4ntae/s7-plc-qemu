#include "qemu/osdep.h"
#include "qapi/error.h" /* provides error_fatal() handler */
#include "hw/sysbus.h" /* provides all sysbus registering func */
#include "hw/misc/plc_ddram.h"
#include "hw/hw.h"
#include "qapi/visitor.h"
#include "hw/qdev-properties.h"
#include "exec/memory.h"
#include "exec/address-spaces.h"

#include "qemu/log.h"
#define TYPE_PLC_DDRAM "xlnx.plc_ddram"
typedef struct PLCDDRAMState PLCDDRAMState;
DECLARE_INSTANCE_CHECKER(PLCDDRAMState, PLC_DDRAM, TYPE_PLC_DDRAM)

#define REG_ID 	0x0
#define CHIP_ID	0xBA000001

struct PLCDDRAMState {
	SysBusDevice parent_obj;
	MemoryRegion memory;
	uint64_t chip_id;
    uint32_t *storage;
};

static uint64_t plc_ddram_read(void *opaque, hwaddr addr, unsigned int size)
{
    PLCDDRAMState *s = opaque;
    uint32_t result = 0;
    qemu_log_mask(LOG_GUEST_ERROR, "%s: read: addr=0x%x\n",
                 __func__, (int)addr);
    return 0x420;
    if (addr == 0x02508814) {
        return 0x420;
    }
    switch (size) {
        case 1:
            result = ((uint8_t *)s->storage)[addr];
            break;
        case 2:
            result = ((uint16_t *)s->storage)[addr];
            break;
        case 4:
            result = ((uint32_t *)s->storage)[addr];
            break;
        case 8:
            result = ((uint64_t *)s->storage)[addr];
            break;
        default:
            g_error("Unsupported size for RAM read: %u", size);
    }
    return result;
	switch (addr) {
	default:
		return 0x00004000;
	}
}

static void plc_ddram_write(void *opaque, hwaddr addr, uint64_t data, unsigned int size)
{
    PLCDDRAMState *s = opaque;

    if (addr == 0x02508814) {
        qemu_log_mask(LOG_GUEST_ERROR, "%s: write: addr=0x%x v=0x%x\n",
                  __func__, (int)addr, (int)data);
    }
    switch (size) {
        case 1:
            ((uint8_t *)s->storage)[addr] = data;
            break;
        case 2:
            ((uint16_t *)s->storage)[addr] = data;
            break;
        case 4:
            ((uint32_t *)s->storage)[addr] = data;
            break;
        case 8:
            ((uint64_t *)s->storage)[addr] = data;
            break;
        default:
            g_error("Unsupported size for RAM write: %u", size);
    }
}

static const MemoryRegionOps plc_ddram_ops = {
	.read = plc_ddram_read,
    .write = plc_ddram_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
    .impl = {
        .min_access_size = 1,
        .max_access_size = 8,
    }
};

static void plc_ddram_instance_init(Object *obj)
{
	PLCDDRAMState *s = PLC_DDRAM(obj);

	/* allocate memory map region */
    memory_region_init_io(&s->memory, obj, &plc_ddram_ops, s, TYPE_PLC_DDRAM, 0x3ff8000);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->memory);
    s->storage = g_malloc(0x3ff8000);

	s->chip_id = CHIP_ID;
}

/* create a new type to define the info related to our device */
static const TypeInfo plc_ddram_info = {
	.name = TYPE_PLC_DDRAM,
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
	DeviceState *dev = qdev_new(TYPE_PLC_DDRAM);
	sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
	sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, addr);
	return dev;
} 
