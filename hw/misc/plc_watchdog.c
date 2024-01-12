#include "qemu/osdep.h"
#include "qapi/error.h" /* provides error_fatal() handler */
#include "hw/sysbus.h" /* provides all sysbus registering func */
#include "hw/misc/plc_watchdog.h"
#include "qemu/timer.h"
#include "qemu/log.h"
#define TYPE_PLC_WATCHDOG "xlnx.plc_watchdog"

typedef struct PLCWatchdogState PLCWatchdogState;
DECLARE_INSTANCE_CHECKER(PLCWatchdogState, PLC_WATCHDOG, TYPE_PLC_WATCHDOG)

#define REG_ID 	0x0

struct PLCWatchdogState {
	SysBusDevice parent_obj;
	MemoryRegion iomem;
};

static uint64_t plc_watchdog_read(void *opaque, hwaddr addr, unsigned int size)
{
    return qemu_clock_get_ms(QEMU_CLOCK_VIRTUAL);
}

static void plc_watchdog_write(void *opaque, hwaddr addr, uint64_t val64, unsigned int size)
{
    PLCWatchdogState *s = opaque;
    uint32_t value = val64;
    unsigned char ch = value;
    (void)s;
    (void)ch;
    qemu_log_mask(LOG_GUEST_ERROR, "%s: write: addr=0x%x v=0x%x\n",
                  __func__, (int)addr, (int)value);
}

static const MemoryRegionOps plc_watchdog_ops = {
	.read = plc_watchdog_read,
    .write = plc_watchdog_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
};


static void plc_watchdog_instance_init(Object *obj)
{
    PLCWatchdogState *state = PLC_WATCHDOG(obj);

	/* allocate memory map region */
	memory_region_init_io(&state->iomem, obj, &plc_watchdog_ops, state, TYPE_PLC_WATCHDOG, 0x100);
	sysbus_init_mmio(SYS_BUS_DEVICE(obj), &state->iomem);
}

/* create a new type to define the info related to our device */
static const TypeInfo plc_watchdog_info = {
	.name = TYPE_PLC_WATCHDOG,
	.parent = TYPE_SYS_BUS_DEVICE,
	.instance_size = sizeof(PLCWatchdogState),
	.instance_init = plc_watchdog_instance_init,
};

static void plc_watchdog_register_types(void)
{
    type_register_static(&plc_watchdog_info);
}

type_init(plc_watchdog_register_types)

/*
 * Create the PLC WATCHDOG device.
 */
DeviceState *plc_watchdog_create(hwaddr addr)
{
	DeviceState *dev = qdev_new(TYPE_PLC_WATCHDOG);
	sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
	sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, addr);
	return dev;
} 
