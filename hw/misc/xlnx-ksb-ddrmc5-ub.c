/*
 * QEMU model of the DDRMC5_UB DDRMC5 Microblaze ODS endpoint
 *
 * Copyright (c) 2022 Xilinx Inc.
 *
 * Autogenerated by xregqemu.py 2022-06-23.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "hw/register.h"
#include "qemu/bitops.h"
#include "qemu/log.h"
#include "migration/vmstate.h"

#ifndef XILINX_DDRMC5_UB_ERR_DEBUG
#define XILINX_DDRMC5_UB_ERR_DEBUG 0
#endif

#define TYPE_XILINX_DDRMC5_UB "xlnx.ddrmc5_ub"

#define XILINX_DDRMC5_UB(obj) \
     OBJECT_CHECK(DDRMC5_UB, (obj), TYPE_XILINX_DDRMC5_UB)

REG32(DDRMC_PCSR_MASK, 0x0)
    FIELD(DDRMC_PCSR_MASK, ON_DEMAND_LOCKDOWN_CRYPTO, 28, 1)
    FIELD(DDRMC_PCSR_MASK, MEM_CLEAR_TRIGGER_CRYPTO, 27, 1)
    FIELD(DDRMC_PCSR_MASK, BISR_TRIGGER_CRYPTO, 26, 1)
    FIELD(DDRMC_PCSR_MASK, BISR_TRIGGER, 25, 1)
    FIELD(DDRMC_PCSR_MASK, UB_INITSTATE, 24, 1)
    FIELD(DDRMC_PCSR_MASK, TEST_SAFE, 20, 1)
    FIELD(DDRMC_PCSR_MASK, SLVERREN, 19, 1)
    FIELD(DDRMC_PCSR_MASK, MEM_CLEAR_TRIGGER, 18, 1)
    FIELD(DDRMC_PCSR_MASK, SYS_RST_MASK, 15, 3)
    FIELD(DDRMC_PCSR_MASK, SLEEP, 10, 1)
    FIELD(DDRMC_PCSR_MASK, FABRICEN, 9, 1)
    FIELD(DDRMC_PCSR_MASK, HOLDSTATE, 7, 1)
    FIELD(DDRMC_PCSR_MASK, INITSTATE, 6, 1)
    FIELD(DDRMC_PCSR_MASK, ODISABLE, 2, 4)
    FIELD(DDRMC_PCSR_MASK, GATEREG, 1, 1)
    FIELD(DDRMC_PCSR_MASK, PCOMPLETE, 0, 1)
REG32(DDRMC_PCSR_CONTROL, 0x4)
    FIELD(DDRMC_PCSR_CONTROL, ON_DEMAND_LOCKDOWN_CRYPTO, 28, 1)
    FIELD(DDRMC_PCSR_CONTROL, MEM_CLEAR_TRIGGER_CRYPTO, 27, 1)
    FIELD(DDRMC_PCSR_CONTROL, BISR_TRIGGER_CRYPTO, 26, 1)
    FIELD(DDRMC_PCSR_CONTROL, BISR_TRIGGER, 25, 1)
    FIELD(DDRMC_PCSR_CONTROL, UB_INITSTATE, 24, 1)
    FIELD(DDRMC_PCSR_CONTROL, TEST_SAFE, 20, 1)
    FIELD(DDRMC_PCSR_CONTROL, SLVERREN, 19, 1)
    FIELD(DDRMC_PCSR_CONTROL, MEM_CLEAR_TRIGGER, 18, 1)
    FIELD(DDRMC_PCSR_CONTROL, SYS_RST_MASK, 15, 3)
    FIELD(DDRMC_PCSR_CONTROL, SLEEP, 10, 1)
    FIELD(DDRMC_PCSR_CONTROL, FABRICEN, 9, 1)
    FIELD(DDRMC_PCSR_CONTROL, HOLDSTATE, 7, 1)
    FIELD(DDRMC_PCSR_CONTROL, INITSTATE, 6, 1)
    FIELD(DDRMC_PCSR_CONTROL, ODISABLE, 2, 4)
    FIELD(DDRMC_PCSR_CONTROL, GATEREG, 1, 1)
    FIELD(DDRMC_PCSR_CONTROL, PCOMPLETE, 0, 1)
REG32(DDRMC_PCSR_STATUS, 0x8)
    FIELD(DDRMC_PCSR_STATUS, LOCKDOWN, 24, 1)
    FIELD(DDRMC_PCSR_STATUS, KAT_PASS, 23, 1)
    FIELD(DDRMC_PCSR_STATUS, KAT_DONE, 22, 1)
    FIELD(DDRMC_PCSR_STATUS, ZEROIZE_PASS, 21, 1)
    FIELD(DDRMC_PCSR_STATUS, ZEROIZE_DONE, 20, 1)
    FIELD(DDRMC_PCSR_STATUS, MEM_CLEAR_PASS_CRYPTO, 19, 1)
    FIELD(DDRMC_PCSR_STATUS, MEM_CLEAR_DONE_CRYPTO, 18, 1)
    FIELD(DDRMC_PCSR_STATUS, SCAN_CLEAR_PASS_CRYPTO, 17, 1)
    FIELD(DDRMC_PCSR_STATUS, SCAN_CLEAR_DONE_CRYPTO, 16, 1)
    FIELD(DDRMC_PCSR_STATUS, HARD_FAIL_OR, 11, 3)
    FIELD(DDRMC_PCSR_STATUS, HARD_FAIL_AND, 8, 3)
    FIELD(DDRMC_PCSR_STATUS, MEM_CLEAR_PASS, 7, 1)
    FIELD(DDRMC_PCSR_STATUS, MEM_CLEAR_DONE, 6, 1)
    FIELD(DDRMC_PCSR_STATUS, CALERROR, 5, 1)
    FIELD(DDRMC_PCSR_STATUS, CALDONE, 4, 1)
    FIELD(DDRMC_PCSR_STATUS, INCAL, 3, 1)
    FIELD(DDRMC_PCSR_STATUS, SCAN_CLEAR_PASS, 2, 1)
    FIELD(DDRMC_PCSR_STATUS, SCAN_CLEAR_DONE, 1, 1)
    FIELD(DDRMC_PCSR_STATUS, PCSRLOCK, 0, 1)
REG32(DDRMC_PCSR_LOCK, 0xc)
    FIELD(DDRMC_PCSR_LOCK, STATE, 0, 1)

#define DDRMC5_UB_R_MAX (R_DDRMC_PCSR_LOCK + 1)

typedef struct DDRMC5_UB {
    SysBusDevice parent_obj;
    MemoryRegion iomem;

    uint32_t regs[DDRMC5_UB_R_MAX];
    RegisterInfo regs_info[DDRMC5_UB_R_MAX];
} DDRMC5_UB;

static void ddrmc_pcsr_ctrl_postw(RegisterInfo *reg, uint64_t val)
{
    DDRMC5_UB *s = XILINX_DDRMC5_UB(reg->opaque);
    uint32_t val32 = val;

    s->regs[R_DDRMC_PCSR_STATUS] |=
        (val32 & R_DDRMC_PCSR_CONTROL_MEM_CLEAR_TRIGGER_MASK ?
                 (R_DDRMC_PCSR_STATUS_MEM_CLEAR_DONE_MASK |
                 R_DDRMC_PCSR_STATUS_MEM_CLEAR_PASS_MASK): 0)|
        (val32 & R_DDRMC_PCSR_CONTROL_MEM_CLEAR_TRIGGER_CRYPTO_MASK ?
                 R_DDRMC_PCSR_STATUS_MEM_CLEAR_DONE_CRYPTO_MASK |
                 R_DDRMC_PCSR_STATUS_MEM_CLEAR_PASS_CRYPTO_MASK: 0) |
        (val32 & R_DDRMC_PCSR_CONTROL_ON_DEMAND_LOCKDOWN_CRYPTO_MASK ?
                 R_DDRMC_PCSR_STATUS_LOCKDOWN_MASK: 0);
}

static void ddrmc_pcsr_lock_postw(RegisterInfo *reg, uint64_t val)
{
    DDRMC5_UB *s = XILINX_DDRMC5_UB(reg->opaque);

    s->regs[R_DDRMC_PCSR_STATUS] &= ~R_DDRMC_PCSR_STATUS_PCSRLOCK_MASK;

    if (val != 0xf9e8d7c6) {
        s->regs[R_DDRMC_PCSR_STATUS] |= val & R_DDRMC_PCSR_LOCK_STATE_MASK;
    }
}

static const RegisterAccessInfo ddrmc5_ub_regs_info[] = {
    {   .name = "DDRMC_PCSR_MASK",  .addr = A_DDRMC_PCSR_MASK,
        .rsvd = 0xe0e07900,
        .ro = 0xe07900,
    },{ .name = "DDRMC_PCSR_CONTROL",  .addr = A_DDRMC_PCSR_CONTROL,
        .reset = 0x100047e,
        .rsvd = 0xe0e07900,
        .ro = 0xe07900,
    },{ .name = "DDRMC_PCSR_STATUS",  .addr = A_DDRMC_PCSR_STATUS,
        .reset = 0x1 |
                 R_DDRMC_PCSR_STATUS_SCAN_CLEAR_DONE_MASK |
                 R_DDRMC_PCSR_STATUS_SCAN_CLEAR_PASS_MASK |
                 R_DDRMC_PCSR_STATUS_CALDONE_MASK |
                 R_DDRMC_PCSR_STATUS_SCAN_CLEAR_DONE_CRYPTO_MASK |
                 R_DDRMC_PCSR_STATUS_SCAN_CLEAR_PASS_CRYPTO_MASK |
                 R_DDRMC_PCSR_STATUS_ZEROIZE_DONE_MASK |
                 R_DDRMC_PCSR_STATUS_ZEROIZE_PASS_MASK |
                 R_DDRMC_PCSR_STATUS_KAT_DONE_MASK |
                 R_DDRMC_PCSR_STATUS_KAT_PASS_MASK,
        .rsvd = 0xfe00c000,
        .ro = 0x1ffffff,
        .post_write = ddrmc_pcsr_ctrl_postw,
    },{ .name = "DDRMC_PCSR_LOCK",  .addr = A_DDRMC_PCSR_LOCK,
        .reset = 0x1,
        .rsvd = 0xfffffffe,
        .post_write = ddrmc_pcsr_lock_postw,
    },
};

static void ddrmc5_ub_reset_enter(Object *obj, ResetType type)
{
    DDRMC5_UB *s = XILINX_DDRMC5_UB(obj);
    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(s->regs_info); ++i) {
        register_reset(&s->regs_info[i]);
    }
}

static const MemoryRegionOps ddrmc5_ub_ops = {
    .read = register_read_memory,
    .write = register_write_memory,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static void ddrmc5_ub_init(Object *obj)
{
    DDRMC5_UB *s = XILINX_DDRMC5_UB(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    RegisterInfoArray *reg_array;

    memory_region_init(&s->iomem, obj, TYPE_XILINX_DDRMC5_UB, DDRMC5_UB_R_MAX * 4);
    reg_array =
        register_init_block32(DEVICE(obj), ddrmc5_ub_regs_info,
                              ARRAY_SIZE(ddrmc5_ub_regs_info),
                              s->regs_info, s->regs,
                              &ddrmc5_ub_ops,
                              XILINX_DDRMC5_UB_ERR_DEBUG,
                              DDRMC5_UB_R_MAX * 4);
    memory_region_add_subregion(&s->iomem,
                                0x0,
                                &reg_array->mem);
    sysbus_init_mmio(sbd, &s->iomem);
}

static const VMStateDescription vmstate_ddrmc5_ub = {
    .name = TYPE_XILINX_DDRMC5_UB,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32_ARRAY(regs, DDRMC5_UB, DDRMC5_UB_R_MAX),
        VMSTATE_END_OF_LIST(),
    }
};

static void ddrmc5_ub_class_init(ObjectClass *klass, void *data)
{
    ResettableClass *rc = RESETTABLE_CLASS(klass);
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->vmsd = &vmstate_ddrmc5_ub;
    rc->phases.enter = ddrmc5_ub_reset_enter;
}

static const TypeInfo ddrmc5_ub_info = {
    .name          = TYPE_XILINX_DDRMC5_UB,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(DDRMC5_UB),
    .class_init    = ddrmc5_ub_class_init,
    .instance_init = ddrmc5_ub_init,
};

static void ddrmc5_ub_register_types(void)
{
    type_register_static(&ddrmc5_ub_info);
}

type_init(ddrmc5_ub_register_types)
