/*
 * QEMU model of the CPM_PCSR This block implements the standard PCSR interface for Reset/Initialization of CPM
 *
 * Copyright (c) 2019 Xilinx Inc.
 *
 * Autogenerated by xregqemu.py 2019-07-17.
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
#include "hw/qdev-properties.h"

#ifndef XILINX_CPM_PCSR_ERR_DEBUG
#define XILINX_CPM_PCSR_ERR_DEBUG 0
#endif

#define TYPE_XILINX_CPM_PCSR "xlnx.versal_cpm_pcsr"

#define XILINX_CPM_PCSR(obj) \
     OBJECT_CHECK(CPM_PCSR, (obj), TYPE_XILINX_CPM_PCSR)

REG32(MASK, 0x0)
    FIELD(MASK, TEST_SAFE_WEN, 20, 1)
    FIELD(MASK, SLVERREN_WEN, 19, 1)
    FIELD(MASK, MEM_CLEAR_TRIGGER_WEN, 18, 1)
    FIELD(MASK, SYS_RST_MASK3_WEN, 17, 1)
    FIELD(MASK, SYS_RST_MASK2_WEN, 16, 1)
    FIELD(MASK, SYS_RST_MASK1_WEN, 15, 1)
    FIELD(MASK, PWRDN_WEN, 14, 1)
    FIELD(MASK, DISNPICLK_WEN, 13, 1)
    FIELD(MASK, APBEN_WEN, 12, 1)
    FIELD(MASK, SCAN_CLEAR_TRIGGER_WEN, 11, 1)
    FIELD(MASK, STARTCAL_WEN, 10, 1)
    FIELD(MASK, FABRICEN_WEN, 9, 1)
    FIELD(MASK, TRISTATE_WEN, 8, 1)
    FIELD(MASK, HOLDSTATE_WEN, 7, 1)
    FIELD(MASK, INITSTATE_WEN, 6, 1)
    FIELD(MASK, ODISABLE3_WEN, 5, 1)
    FIELD(MASK, ODISABLE2_WEN, 4, 1)
    FIELD(MASK, ODISABLE1_WEN, 3, 1)
    FIELD(MASK, ODISABLE0_WEN, 2, 1)
    FIELD(MASK, GATEREG_WEN, 1, 1)
    FIELD(MASK, PCOMPLETE_WEN, 0, 1)
REG32(PCR, 0x4)
    FIELD(PCR, TEST_SAFE, 20, 1)
    FIELD(PCR, SLVERREN, 19, 1)
    FIELD(PCR, MEM_CLEAR_TRIGGER, 18, 1)
    FIELD(PCR, SYS_RST_MASK3, 17, 1)
    FIELD(PCR, SYS_RST_MASK2, 16, 1)
    FIELD(PCR, SYS_RST_MASK1, 15, 1)
    FIELD(PCR, PWRDN, 14, 1)
    FIELD(PCR, DISNPICLK, 13, 1)
    FIELD(PCR, APBEN, 12, 1)
    FIELD(PCR, SCAN_CLEAR_TRIGGER, 11, 1)
    FIELD(PCR, STARTCAL, 10, 1)
    FIELD(PCR, FABRICEN, 9, 1)
    FIELD(PCR, TRISTATE, 8, 1)
    FIELD(PCR, HOLDSTATE, 7, 1)
    FIELD(PCR, INITSTATE, 6, 1)
    FIELD(PCR, ODISABLE3, 5, 1)
    FIELD(PCR, ODISABLE2, 4, 1)
    FIELD(PCR, ODISABLE1, 3, 1)
    FIELD(PCR, ODISABLE0, 2, 1)
    FIELD(PCR, GATEREG, 1, 1)
    FIELD(PCR, PCOMPLETE, 0, 1)
REG32(PSR, 0x8)
    FIELD(PSR, HARD_FAIL_AND, 11, 3)
    FIELD(PSR, HARD_FAIL_OR, 8, 3)
    FIELD(PSR, MEM_CLEAR_PASS, 7, 1)
    FIELD(PSR, MEM_CLEAR_DONE, 6, 1)
    FIELD(PSR, CALERROR, 5, 1)
    FIELD(PSR, CALDONE, 4, 1)
    FIELD(PSR, INCAL, 3, 1)
    FIELD(PSR, SCAN_CLEAR_PASS, 2, 1)
    FIELD(PSR, SCAN_CLEAR_DONE, 1, 1)
    FIELD(PSR, PCSRLOCK, 0, 1)
REG32(LOCK, 0xc)

#define CPM_PCSR_R_MAX (R_LOCK + 1)

typedef struct CPM_PCSR {
    SysBusDevice parent_obj;
    MemoryRegion iomem;

    uint32_t regs[CPM_PCSR_R_MAX];
    RegisterInfo regs_info[CPM_PCSR_R_MAX];
} CPM_PCSR;

static void cpm_pcsr_lock_postw(RegisterInfo *reg, uint64_t val)
{
    CPM_PCSR *s = reg->opaque;

    if (val == 0xf9e8d7c6) {
        s->regs[R_PSR] &= ~R_PSR_PCSRLOCK_MASK;
    } else {
        s->regs[R_PSR] |= R_PSR_PCSRLOCK_MASK;
    }
}

static const RegisterAccessInfo cpm_pcsr_regs_info[] = {
    {   .name = "MASK",  .addr = A_MASK,
        .reset = 0x1fe,
        .rsvd = 0xffe00000,
    },{ .name = "PCR",  .addr = A_PCR,
        .reset = 0x1fe,
        .rsvd = 0xffe00000,
    },{ .name = "PSR",  .addr = A_PSR,
        .reset = R_PSR_PCSRLOCK_MASK |
                 R_PSR_SCAN_CLEAR_DONE_MASK |
                 R_PSR_SCAN_CLEAR_PASS_MASK |
                 R_PSR_CALDONE_MASK |
                 R_PSR_MEM_CLEAR_DONE_MASK |
                 R_PSR_MEM_CLEAR_PASS_MASK,
        .rsvd = 0xffffc000,
        .ro = 0xffffffff,
    },{ .name = "LOCK",  .addr = A_LOCK,
        .reset = 0x1,
        .post_write = cpm_pcsr_lock_postw,
    }
};

static void cpm_pcsr_reset(DeviceState *dev)
{
    CPM_PCSR *s = XILINX_CPM_PCSR(dev);
    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(s->regs_info); ++i) {
        register_reset(&s->regs_info[i]);
    }

}

static const MemoryRegionOps cpm_pcsr_ops = {
    .read = register_read_memory,
    .write = register_write_memory,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static void cpm_pcsr_realize(DeviceState *dev, Error **errp)
{
    /* Delete this if you don't need it */
}

static void cpm_pcsr_init(Object *obj)
{
    CPM_PCSR *s = XILINX_CPM_PCSR(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    RegisterInfoArray *reg_array;

    memory_region_init(&s->iomem, obj, TYPE_XILINX_CPM_PCSR, CPM_PCSR_R_MAX * 4);
    reg_array =
        register_init_block32(DEVICE(obj), cpm_pcsr_regs_info,
                              ARRAY_SIZE(cpm_pcsr_regs_info),
                              s->regs_info, s->regs,
                              &cpm_pcsr_ops,
                              XILINX_CPM_PCSR_ERR_DEBUG,
                              CPM_PCSR_R_MAX * 4);
    memory_region_add_subregion(&s->iomem,
                                0x0,
                                &reg_array->mem);
    sysbus_init_mmio(sbd, &s->iomem);
}

static const VMStateDescription vmstate_cpm_pcsr = {
    .name = TYPE_XILINX_CPM_PCSR,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32_ARRAY(regs, CPM_PCSR, CPM_PCSR_R_MAX),
        VMSTATE_END_OF_LIST(),
    }
};

static void cpm_pcsr_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->reset = cpm_pcsr_reset;
    dc->realize = cpm_pcsr_realize;
    dc->vmsd = &vmstate_cpm_pcsr;
}

static const TypeInfo cpm_pcsr_info = {
    .name          = TYPE_XILINX_CPM_PCSR,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(CPM_PCSR),
    .class_init    = cpm_pcsr_class_init,
    .instance_init = cpm_pcsr_init,
};

static void cpm_pcsr_register_types(void)
{
    type_register_static(&cpm_pcsr_info);
}

type_init(cpm_pcsr_register_types)
