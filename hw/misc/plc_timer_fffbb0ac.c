#include "qemu/osdep.h"
#include "qapi/error.h" /* provides error_fatal() handler */
#include "hw/sysbus.h" /* provides all sysbus registering func */
#include "hw/misc/plc_timer_fffbb0ac.h"
#include "qemu/timer.h"
#include "qemu/log.h"
#define TYPE_PLC_TIMER_fffbb0ac "xlnx.plc_timer_fffbb0ac"

typedef struct PLCTimer_fffbb0acState PLCTimer_fffbb0acState;
DECLARE_INSTANCE_CHECKER(PLCTimer_fffbb0acState, PLC_TIMER_fffbb0ac, TYPE_PLC_TIMER_fffbb0ac)

#define REG_ID 	0x0
struct PLCTimer_fffbb0acState {
	SysBusDevice parent_obj;
	MemoryRegion iomem;
};

#define TICKS_PER_MS 100

int start_time_ms = 0;
int timer_tick_count = 50;
int stop_start = 1;

static uint64_t plc_timer_fffbb0ac_read(void *opaque, hwaddr addr, unsigned int size)
{
    // timer starts countdown when we first read from its memory location
    if (start_time_ms == 0) {
        start_time_ms = qemu_clock_get_ms(QEMU_CLOCK_VIRTUAL);
    }
    int curr_time = qemu_clock_get_ms(QEMU_CLOCK_VIRTUAL);

    int ticks_left = timer_tick_count - (curr_time - start_time_ms) / TICKS_PER_MS;
    if (ticks_left < 0) {
        // for now this timer is one-shot only
        ticks_left = 0;
    }

	switch (addr) {
        case 0x0:  
            // ticks_left (decrementing value from timer_tick_count to 0)
            return ticks_left;
            break;
        case 0x4: 
            // setter
            return timer_tick_count;
            break;
        case 0x8: 
            // stop_start
            return stop_start;
            break; 
        case 0xc:
            // state (0 = active)
            if (ticks_left  > 0) return 0; // timer still running

            return 1; // timer finished
            break;
        default:
		    return curr_time - start_time_ms; // elapsed time in ms
		    break;
	}

	return 0;
}

static void plc_timer_fffbb0ac_write(void *opaque, hwaddr addr, uint64_t val64, unsigned int size)
{
    uint32_t value = val64;
    switch (addr) {
        case 0x4:
            timer_tick_count = value;
            start_time_ms = qemu_clock_get_ms(QEMU_CLOCK_VIRTUAL);
            break;
        case 0x8:
            stop_start = value;
            break;
        default:
            break;
    }

    qemu_log_mask(LOG_GUEST_ERROR, "%s: write: addr=0x%x v=0x%x\n",
                  __func__, (int)addr, (int)value);
}

static const MemoryRegionOps plc_timer_fffbb0ac_ops = {
	.read = plc_timer_fffbb0ac_read,
    .write = plc_timer_fffbb0ac_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
};


static void plc_timer_fffbb0ac_instance_init(Object *obj)
{
    PLCTimer_fffbb0acState *state = PLC_TIMER_fffbb0ac(obj);

	/* allocate memory map region */
	memory_region_init_io(&state->iomem, obj, &plc_timer_fffbb0ac_ops, state, TYPE_PLC_TIMER_fffbb0ac, 0x100);
	sysbus_init_mmio(SYS_BUS_DEVICE(obj), &state->iomem);
}

/* create a new type to define the info related to our device */
static const TypeInfo plc_timer_fffbb0ac_info = {
	.name = TYPE_PLC_TIMER_fffbb0ac,
	.parent = TYPE_SYS_BUS_DEVICE,
	.instance_size = sizeof(PLCTimer_fffbb0acState),
	.instance_init = plc_timer_fffbb0ac_instance_init,
};

static void plc_timer_fffbb0ac_register_types(void)
{
    type_register_static(&plc_timer_fffbb0ac_info);
}

type_init(plc_timer_fffbb0ac_register_types)

/*
 * Create the PLC TIMER_fffbb0ac device.
 */
DeviceState *plc_timer_fffbb0ac_create(hwaddr addr)
{
	DeviceState *dev = qdev_new(TYPE_PLC_TIMER_fffbb0ac);
	sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
	sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, addr);
	return dev;
} 
