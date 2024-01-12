#include "qemu/osdep.h"
#include "qapi/error.h" /* provides error_fatal() handler */
#include "hw/sysbus.h" /* provides all sysbus registering func */
#include "hw/misc/plc_fw_copy.h"
#include "qemu/error-report.h"
#include "hw/loader.h"
#include "exec/memory.h"
#include "hw/boards.h"


#include "qemu/log.h"

#define TYPE_PLC_FW_COPY "xlnx.plc_fw_copy"
typedef struct PLCFwCopyState PLCFwCopyState;
DECLARE_INSTANCE_CHECKER(PLCFwCopyState, PLC_FW_COPY,
TYPE_PLC_FW_COPY)

#define REG_ID    0x0
#define CHIP_ID    0xBA000001


struct PLCFwCopyState {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    uint64_t chip_id;
};

static uint64_t plc_fw_copy_read(void *opaque, hwaddr addr, unsigned int size) {
    return 0x00004000;
}

static void plc_fw_copy_write(void *opaque, hwaddr addr, uint64_t val64, unsigned int size) {
    PLCFwCopyState * s = opaque;
    uint32_t value = val64;
    unsigned char ch = value;
    (void) s;
    (void) ch;
    qemu_log_mask(LOG_GUEST_ERROR, "%s: write: addr=0x%x v=0x%x\n",
                  __func__, (int) addr, (int) value);
}

static const MemoryRegionOps plc_fw_copy_ops = {
        .read = plc_fw_copy_read,
        .write = plc_fw_copy_write,
        .endianness = DEVICE_NATIVE_ENDIAN,
};


static void plc_fw_copy_instance_init(Object *obj) {
    PLCFwCopyState * s = PLC_FW_COPY(obj);

    /* allocate memory map region */
    memory_region_init_io(&s->iomem, obj, &plc_fw_copy_ops, s, TYPE_PLC_FW_COPY, 0x2c);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);

    s->chip_id = CHIP_ID;
}

/* create a new type to define the info related to our device */
static const TypeInfo plc_fw_copy_info = {
        .name = TYPE_PLC_FW_COPY,
        .parent = TYPE_SYS_BUS_DEVICE,
        .instance_size = sizeof(PLCFwCopyState),
        .instance_init = plc_fw_copy_instance_init,
};

static void plc_fw_copy_register_types(void) {
    type_register_static(&plc_fw_copy_info);
}

type_init(plc_fw_copy_register_types)

/*
 * Create the PLC FW COPY device.
 */
DeviceState *plc_fw_copy_create(hwaddr addr) {
    DeviceState *dev = qdev_new(TYPE_PLC_FW_COPY);
    sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, addr);
    return dev;
} 
