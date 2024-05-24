#include "qemu/osdep.h"
#include "qapi/error.h" /* provides error_fatal() handler */
#include "hw/sysbus.h" /* provides all sysbus registering func */
#include "hw/misc/plc_bool_helper.h"
#include "hw/hw.h"
#include "qapi/visitor.h"
#include "hw/qdev-properties.h"
#include "exec/memory.h"
#include "exec/address-spaces.h"

#include "qemu/log.h"
#define TYPE_PLC_BOOL_HELPER "xlnx.plc_bool_helper"
typedef struct PLCbool_helperState PLCbool_helperState;
DECLARE_INSTANCE_CHECKER(PLCbool_helperState, PLC_BOOL_HELPER, TYPE_PLC_BOOL_HELPER)

#define REG_ID 	0x0
#define CHIP_ID	0xBA000001

struct PLCbool_helperState {
	SysBusDevice parent_obj;
	MemoryRegion memory;
	uint64_t chip_id;
    uint8_t *storage;
};

static uint64_t plc_bool_helper_read(void *opaque, hwaddr addr, unsigned int size)
{
    PLCbool_helperState *s = opaque;
    uint32_t result = 0;
    qemu_log_mask(LOG_GUEST_ERROR, "%s: read: addr=0x%x\n",
                  __func__, (int)addr);
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


static void plc_bool_helper_write(void *opaque, hwaddr addr, uint64_t data, unsigned int size)
{
    PLCbool_helperState *s = opaque;

    qemu_log_mask(LOG_GUEST_ERROR, "%s: write: addr=0x%x v=0x%x\n",
                  __func__, (int)addr, (int)data);
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

static const MemoryRegionOps plc_bool_helper_ops = {
	.read = plc_bool_helper_read,
    .write = plc_bool_helper_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
    .impl = {
        .min_access_size = 1,
        .max_access_size = 8,
    }
};

static void plc_bool_helper_instance_init(Object *obj)
{
	PLCbool_helperState *s = PLC_BOOL_HELPER(obj);

	/* allocate memory map region */
    memory_region_init_io(&s->memory, obj, &plc_bool_helper_ops, s, TYPE_PLC_BOOL_HELPER, 0x4000);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->memory);
    s->storage = g_malloc(0x4000);

	s->chip_id = CHIP_ID;
}

/* create a new type to define the info related to our device */
static const TypeInfo plc_bool_helper_info = {
	.name = TYPE_PLC_BOOL_HELPER,
	.parent = TYPE_SYS_BUS_DEVICE,
	.instance_size = sizeof(PLCbool_helperState),
	.instance_init = plc_bool_helper_instance_init,
};

static void plc_bool_helper_register_types(void)
{
    type_register_static(&plc_bool_helper_info);
}

type_init(plc_bool_helper_register_types)

/*
 * Create the PLC bool_helper device.
 */
DeviceState *plc_bool_helper_create(hwaddr addr)
{
	DeviceState *dev = qdev_new(TYPE_PLC_BOOL_HELPER);
	sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
	sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, addr);
	return dev;
}
