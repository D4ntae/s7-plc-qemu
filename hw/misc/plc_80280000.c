#include "qemu/osdep.h"
#include "qapi/error.h" /* provides error_fatal() handler */
#include "hw/sysbus.h" /* provides all sysbus registering func */
#include "hw/misc/plc_80280000.h"

#include "exec/memory.h"
#include "hw/loader.h"

#include "hw/boards.h"
#include "qemu/log.h"
#include "qemu/error-report.h"

#define TYPE_PLC_80280000 "xlnx.plc_80280000"
typedef struct PLC80280000State PLC80280000State;
DECLARE_INSTANCE_CHECKER(PLC80280000State, PLC_80280000, TYPE_PLC_80280000)

#define REG_ID    0x0
#define CHIP_ID    0xBA000001

#define EXEC_IN_LOMEM_FILENAME '/home/dantae/repos/s7-plc-qemu/binaries/exec_in_lomem.lo'
#define CORTEX_R5_CPU_NUM 4
#define MAX_FILE_SIZE 30720
int copied = 0;
struct PLC80280000State {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    uint64_t chip_id;
};

static uint64_t plc_80280000_read(void *opaque, hwaddr addr, unsigned int size) {
    if (!copied) {
        copied = 1;
        AddressSpace *as = &address_space_memory;

        FILE *f;
        char buf[MAX_FILE_SIZE];
        int n = 0;

        f = fopen(EXEC_IN_LOMEM_FILENAME, "rb");
        if (f) {
            n = fread(buf, sizeof(char), MAX_FILE_SIZE, f);
        } else {
            error_printf_unless_qmp("[plc_fw_copy.c] [FAIL] failed loading file %s\n", EXEC_IN_LOMEM_FILENAME);
        }

        if (n > 0) {
            error_printf_unless_qmp("[plc_fw_copy.c] [SUCCESS] loaded file %s:  %s\n", EXEC_IN_LOMEM_FILENAME, buf);
        }

        for (int i = 0; i < MAX_FILE_SIZE; i++) {
            error_printf_unless_qmp("%#x ", buf[i]);
            if ((i + 1) % 16 == 0) error_printf_unless_qmp("\n");
        }

        MemTxResult res;
        MemTxAttrs attrs = {.unspecified = 0,
                .secure = 0,
                .user = 0,
                .debug = 0,
                .requester_id = 0,
        };

        res = address_space_rw(as, 0x0, attrs, buf, MAX_FILE_SIZE, true);
        if (res == MEMTX_OK) {
            error_printf_unless_qmp("[plc_fw_copy.c] [SUCCESS] wrote file to addr. space %s:  %s\n", as->name, buf);
        }

    }

    switch (addr) {
        default:
            return 0x00004000;
    }
}

static void plc_80280000_write(void *opaque, hwaddr addr, uint64_t val64, unsigned int size) {
    PLC80280000State * s = opaque;
    uint32_t value = val64;
    unsigned char ch = value;
    (void) s;
    (void) ch;
    qemu_log_mask(LOG_GUEST_ERROR, "%s: write: addr=0x%x v=0x%x\n",
                  __func__, (int) addr, (int) value);
}

static const MemoryRegionOps plc_80280000_ops = {
        .read = plc_80280000_read,
        .write = plc_80280000_write,
        .endianness = DEVICE_NATIVE_ENDIAN,
};


static void plc_80280000_instance_init(Object *obj) {
    PLC80280000State * s = PLC_80280000(obj);

    /* allocate memory map region */
    memory_region_init_io(&s->iomem, obj, &plc_80280000_ops, s, TYPE_PLC_80280000, 0x2c);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);

    s->chip_id = CHIP_ID;
}

/* create a new type to define the info related to our device */
static const TypeInfo plc_80280000_info = {
        .name = TYPE_PLC_80280000,
        .parent = TYPE_SYS_BUS_DEVICE,
        .instance_size = sizeof(PLC80280000State),
        .instance_init = plc_80280000_instance_init,
};

static void plc_80280000_register_types(void) {
    type_register_static(&plc_80280000_info);
}

type_init(plc_80280000_register_types)

/*
 * Create the PLC 80280000 device.
 */
DeviceState *plc_80280000_create(hwaddr addr) {
    DeviceState *dev = qdev_new(TYPE_PLC_80280000);
    sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, addr);
    return dev;
} 
