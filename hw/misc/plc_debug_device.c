#include "qemu/osdep.h"
#include "qapi/error.h" /* provides error_fatal() handler */
#include "hw/sysbus.h" /* provides all sysbus registering func */
#include "hw/misc/plc_debug_device.h"

#include "exec/memory.h"
#include "hw/loader.h"

#include "hw/boards.h"
#include "qemu/log.h"
#include "qemu/error-report.h"
#include "monitor/monitor.h"
#include "qemu/typedefs.h"
#include "exec/cpu-common.h"
#include "exec/address-spaces.h"
#include "target/arm/cpu-qom.h"
#include "qapi/qmp/qerror.h"
#include "qapi/qmp/qdict.h"

#define TYPE_PLC_DEBUG_DEVICE "xlnx.plc_debug_device"
typedef struct PLCDebugDeviceState PLCDebugDeviceState;
DECLARE_INSTANCE_CHECKER(PLCDebugDeviceState, PLC_DEBUG_DEVICE, TYPE_PLC_DEBUG_DEVICE)

#define REG_ID    0x0
#define CHIP_ID    0xBA000001

#define CORTEX_R5_CPU_NUM 4
struct PLCDebugDeviceState {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    uint64_t chip_id;
    int flag;
};

static uint64_t plc_debug_device_read(void *opaque, hwaddr addr, unsigned int size) {
    qemu_log_mask(LOG_GUEST_ERROR, "%s: read: addr=0x%x\n",
                  __func__, (int) addr);

    return 1;
}

static void plc_debug_device_write(void *opaque, hwaddr addr, uint64_t val64, unsigned int size) {
    PLCDebugDeviceState * s = opaque;
    uint32_t value = val64;
    switch (value) {
        case 1: {

        }
    }

    //qemu_log_mask(LOG_GUEST_ERROR, "R0=%d", state->env_ptr->regs[0]);
    
    qemu_log_mask(LOG_GUEST_ERROR, "%s: write: addr=0x%x v=0x%x\n",
                  __func__, (int) addr, (int) value);
}

static const MemoryRegionOps plc_debug_device_ops = {
        .read = plc_debug_device_read,
        .write = plc_debug_device_write,
        .endianness = DEVICE_NATIVE_ENDIAN,
};

static void plc_debug_device_instance_init(Object *obj) {
    PLCDebugDeviceState * s = PLC_DEBUG_DEVICE(obj);

    /* allocate memory map region */
    memory_region_init_io(&s->iomem, obj, &plc_debug_device_ops, s, TYPE_PLC_DEBUG_DEVICE, 0x4);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);

    s->chip_id = CHIP_ID;
}

/* create a new type to define the info related to our device */
static const TypeInfo plc_debug_device_info = {
        .name = TYPE_PLC_DEBUG_DEVICE,
        .parent = TYPE_SYS_BUS_DEVICE,
        .instance_size = sizeof(PLCDebugDeviceState),
        .instance_init = plc_debug_device_instance_init,
};

static void plc_debug_device_register_types(void) {
    type_register_static(&plc_debug_device_info);
}

type_init(plc_debug_device_register_types);

/*
 * Create the PLC Debug device.
 */
DeviceState *plc_debug_device_create(hwaddr addr) {
    DeviceState *dev = qdev_new(TYPE_PLC_DEBUG_DEVICE);
    sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, addr);
    return dev;
} 
