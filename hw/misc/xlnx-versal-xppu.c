/*
 * QEMU model of the XPPU xppu
 *
 * Copyright (c) 2020 Xilinx Inc.
 *
 * Autogenerated by xregqemu.py 2020-01-13.
 * Written by: Joe Komlodi <komlodi@xilinx.com>
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
#include "sysemu/dma.h"
#include "qemu/log.h"
#include "hw/irq.h"
#include "qapi/error.h"
#include "qemu/log.h"
#include "migration/vmstate.h"
#include "hw/qdev-properties.h"

#include "hw/fdt_generic_util.h"
#include "hw/misc/xlnx-xppu.h"

#define TYPE_XILINX_XPPU "xlnx,versal-xppu"

#define XILINX_XPPU(obj) \
     OBJECT_CHECK(XPPU, (obj), TYPE_XILINX_XPPU)

/*
 * Register definitions shared between ZynqMP and Versal are in
 * the XPPU header file
 */
    FIELD(CTRL, HIDE_EN, 3, 1)
REG32(ITR, 0xc)
    FIELD(ITR, APER_PARITY, 7, 1)
    FIELD(ITR, APER_TZ, 6, 1)
    FIELD(ITR, APER_PERM, 5, 1)
    FIELD(ITR, MID_PARITY, 3, 1)
    FIELD(ITR, MID_RO, 2, 1)
    FIELD(ITR, MID_MISS, 1, 1)
    FIELD(ITR, INV_APB, 0, 1)
REG32(LOCK, 0x20)
    FIELD(LOCK, REGWRDIS, 0, 1)
REG32(DYNAMIC_RECONFIG_EN, 0xfc)
    FIELD(DYNAMIC_RECONFIG_EN, UPDATE, 0, 1)
REG32(DYNAMIC_RECONFIG_APER_ADDR, 0x150)
    FIELD(DYNAMIC_RECONFIG_APER_ADDR, ADDR, 0, 9)
REG32(DYNAMIC_RECONFIG_APER_PERM, 0x154)
    FIELD(DYNAMIC_RECONFIG_APER_PERM, PARITY, 28, 4)
    FIELD(DYNAMIC_RECONFIG_APER_PERM, TRUSTZONE, 27, 1)
    FIELD(DYNAMIC_RECONFIG_APER_PERM, PERMISSION, 0, 20)
REG32(ENABLE_PERM_CHECK_REG02, 0x158)
REG32(ENABLE_PERM_CHECK_REG03, 0x15C)
REG32(ENABLE_PERM_CHECK_REG04, 0x160)
REG32(ENABLE_PERM_CHECK_REG05, 0x164)
REG32(ENABLE_PERM_CHECK_REG06, 0x168)
REG32(ENABLE_PERM_CHECK_REG07, 0x16C)
REG32(ENABLE_PERM_CHECK_REG08, 0x170)
REG32(ENABLE_PERM_CHECK_REG09, 0x174)
REG32(ENABLE_PERM_CHECK_REG10, 0x178)
REG32(ENABLE_PERM_CHECK_REG11, 0x17C)
REG32(ENABLE_PERM_CHECK_REG12, 0x180)

static void isr_postw(RegisterInfo *reg, uint64_t val64)
{
    XPPU *s = XILINX_XPPU(reg->opaque);
    isr_update_irq(s);
}

static uint64_t ien_prew(RegisterInfo *reg, uint64_t val64)
{
    XPPU *s = XILINX_XPPU(reg->opaque);
    uint32_t val = val64;

    s->regs[R_IMR] &= ~val;
    isr_update_irq(s);
    return 0;
}

static uint64_t ids_prew(RegisterInfo *reg, uint64_t val64)
{
    XPPU *s = XILINX_XPPU(reg->opaque);
    uint32_t val = val64;

    s->regs[R_IMR] |= val;
    isr_update_irq(s);
    return 0;
}

static void ctrl_postw(RegisterInfo *reg, uint64_t val64)
{
    XPPU *s = XILINX_XPPU(reg->opaque);
    update_mrs(s);
    check_mid_parities(s);
    isr_update_irq(s);
}

static void mid_postw(RegisterInfo *reg, uint64_t val64)
{
    XPPU *s = XILINX_XPPU(reg->opaque);
    check_mid_parity(s, val64);
    isr_update_irq(s);
}

static uint64_t lock_prew(RegisterInfo *reg, uint64_t val64)
{
    XPPU *s = XILINX_XPPU(reg->opaque);
    uint32_t regwrdis = ARRAY_FIELD_EX32(s->regs, LOCK, REGWRDIS);

    /* Once set, REGWRDIS cannot be cleared unless from SW or HW POR */
    return (regwrdis ? regwrdis : val64);
}

static void itr_postw(RegisterInfo *reg, uint64_t val64)
{
    XPPU *s = XILINX_XPPU(reg->opaque);

    /* ITR is WO, so ensure it's always 0 */
    s->regs[R_ITR] = 0;
    s->regs[R_ISR] |= (uint32_t)val64;
    isr_update_irq(s);
}

static const RegisterAccessInfo xppu_regs_info[] = {
    {   .name = "CTRL",  .addr = A_CTRL,
        .rsvd = 0xfffffff0,
        .ro = 0xfffffff0,
        .post_write = ctrl_postw,
    },{ .name = "ERR_STATUS1",  .addr = A_ERR_STATUS1,
        .ro = 0xffffffff,
    },{ .name = "ERR_STATUS2",  .addr = A_ERR_STATUS2,
        .rsvd = 0xfffffc00,
        .ro = 0xffffffff,
    },{ .name = "ITR",  .addr = A_ITR,
        .rsvd = 0xffffff10,
        .ro = 0xffffff10,
        .post_write = itr_postw,
    },{ .name = "ISR",  .addr = A_ISR,
        .rsvd = 0xffffff10,
        .ro = 0xffffff10,
        .w1c = 0xef,
        .post_write = isr_postw,
    },{ .name = "IMR",  .addr = A_IMR,
        .reset = 0xef,
        .rsvd = 0xffffff10,
        .ro = 0xffffffff,
    },{ .name = "IEN",  .addr = A_IEN,
        .rsvd = 0xffffff10,
        .ro = 0xffffff10,
        .pre_write = ien_prew,
    },{ .name = "IDS",  .addr = A_IDS,
        .rsvd = 0xffffff10,
        .ro = 0xffffff10,
        .pre_write = ids_prew,
    },{ .name = "LOCK",  .addr = A_LOCK,
        .pre_write = lock_prew,
    },{ .name = "M_MASTER_IDS",  .addr = A_M_MASTER_IDS,
        .reset = 0x14,
        .ro = 0xffffffff,
    },{ .name = "M_APERTURE_64KB",  .addr = A_M_APERTURE_64KB,
        .reset = 0x100,
        .ro = 0xffffffff,
    },{ .name = "M_APERTURE_1MB",  .addr = A_M_APERTURE_1MB,
        .reset = 0x10,
        .ro = 0xffffffff,
    },{ .name = "M_APERTURE_512MB",  .addr = A_M_APERTURE_512MB,
        .reset = 0x1,
        .ro = 0xffffffff,
    },{ .name = "BASE_64KB",  .addr = A_BASE_64KB,
        .reset = 0xff000000,
        .ro = 0xffffffff,
    },{ .name = "BASE_1MB",  .addr = A_BASE_1MB,
        .reset = 0xfe000000,
        .ro = 0xffffffff,
    },{ .name = "BASE_512MB",  .addr = A_BASE_512MB,
        .reset = 0xe0000000,
        .ro = 0xffffffff,
    },{ .name = "DYNAMIC_RECONFIG_EN",  .addr = A_DYNAMIC_RECONFIG_EN,
        .rsvd = 0xfffffffe,
    },{ .name = "MASTER_ID00",  .addr = A_MASTER_ID00,
        .reset = 0x03ff0238,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID01",  .addr = A_MASTER_ID01,
        .reset = 0x83ff0200,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID02",  .addr = A_MASTER_ID02,
        .reset = 0x03ff0204,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID03",  .addr = A_MASTER_ID03,
        .reset = 0x83f00260,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID04",  .addr = A_MASTER_ID04,
        .reset = 0x83ff0260,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID05",  .addr = A_MASTER_ID05,
        .reset = 0x03ff0261,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID06",  .addr = A_MASTER_ID06,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID07",  .addr = A_MASTER_ID07,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID08",  .addr = A_MASTER_ID08,
        .reset = 0x83ff0247,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID09",  .addr = A_MASTER_ID09,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID10",  .addr = A_MASTER_ID10,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID11",  .addr = A_MASTER_ID11,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID12",  .addr = A_MASTER_ID12,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID13",  .addr = A_MASTER_ID13,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID14",  .addr = A_MASTER_ID14,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID15",  .addr = A_MASTER_ID15,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID16",  .addr = A_MASTER_ID16,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID17",  .addr = A_MASTER_ID17,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID18",  .addr = A_MASTER_ID18,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "MASTER_ID19",  .addr = A_MASTER_ID19,
        .rsvd = 0x3c00fc00,
        .ro = 0x3c00fc00,
        .post_write = mid_postw,
    },{ .name = "DYNAMIC_RECONFIG_APER_ADDR",
        .addr = A_DYNAMIC_RECONFIG_APER_ADDR,
        .reset = 0xffffffff,
        .rsvd = 0xfffffe00,
    },{ .name = "DYNAMIC_RECONFIG_APER_PERM",
        .addr = A_DYNAMIC_RECONFIG_APER_PERM,
        .reset = 0xffffffff,
        .rsvd = 0x7f00000,
    },{ .name = "ENABLE_PERM_CHECK_REG02",
        .addr = A_ENABLE_PERM_CHECK_REG02,
        .reset = 0xffffffff,
    },{ .name = "ENABLE_PERM_CHECK_REG03",
        .addr = A_ENABLE_PERM_CHECK_REG03,
        .reset = 0xffffffff,
    },{ .name = "ENABLE_PERM_CHECK_REG04",
        .addr = A_ENABLE_PERM_CHECK_REG04,
        .reset = 0xffffffff,
    },{ .name = "ENABLE_PERM_CHECK_REG05",
        .addr = A_ENABLE_PERM_CHECK_REG05,
        .reset = 0xffffffff,
    },{ .name = "ENABLE_PERM_CHECK_REG06",
        .addr = A_ENABLE_PERM_CHECK_REG06,
        .reset = 0xffffffff,
    },{ .name = "ENABLE_PERM_CHECK_REG07",
        .addr = A_ENABLE_PERM_CHECK_REG07,
        .reset = 0xffffffff,
    },{ .name = "ENABLE_PERM_CHECK_REG08",
        .addr = A_ENABLE_PERM_CHECK_REG08,
        .reset = 0xffffffff,
    },{ .name = "ENABLE_PERM_CHECK_REG09",
        .addr = A_ENABLE_PERM_CHECK_REG09,
        .reset = 0xffffffff,
    },{ .name = "ENABLE_PERM_CHECK_REG10",
        .addr = A_ENABLE_PERM_CHECK_REG10,
        .reset = 0xffffffff,
    },{ .name = "ENABLE_PERM_CHECK_REG11",
        .addr = A_ENABLE_PERM_CHECK_REG11,
        .reset = 0xffffffff,
    },{ .name = "ENABLE_PERM_CHECK_REG12",
        .addr = A_ENABLE_PERM_CHECK_REG12,
        .reset = 0xfffff,
        .rsvd = 0xfff00000,
    }
};

static void xppu_reset(DeviceState *dev)
{
    XPPU *s = XILINX_XPPU(dev);
    unsigned int i;

    static const uint32_t pmc_base_resets[] = {
        0xf1000000,
        0xf0000000,
        0xc0000000
    };
    static const uint32_t pmc_npi_base_resets[] = {
        0xf6000000,
        0xf7000000,
        0x0
    };
    static const uint32_t pmc_mid_resets[] = {
        0x03ff0238,
        0x83ff0200,
        0x03ff0204,
        0x83f00260,
        0x83ff0260,
        0x03ff0261,
        0x0,
        0x0,
        0x83ff0247
    };

    for (i = 0; i < ARRAY_SIZE(s->regs_info); ++i) {
        /* PMC and PMC NPI XPPUs have different bases than LPD XPPU */
        if (i >= R_BASE_64KB && i <= R_BASE_512MB) {
            if (s->region == XPPU_REGION_PMC) {
                s->regs[i] = pmc_base_resets[(i - R_BASE_64KB)];
            } else if (s->region == XPPU_REGION_PMC_NPI) {
                s->regs[i] = pmc_npi_base_resets[(i - R_BASE_64KB)];
            } else {
                register_reset(&s->regs_info[i]);
            }
        /* PMC and PMC NPI XPPUs have different MID resets than LPD XPPU */
        } else if ((i >= R_MASTER_ID00 && i <= R_MASTER_ID05) ||
                    i == R_MASTER_ID08) {
            if (s->region != XPPU_REGION_LPD) {
                s->regs[i] = pmc_mid_resets[i - R_MASTER_ID00];
            } else {
                register_reset(&s->regs_info[i]);
            }
        /* Everything else can be reset normally */
        } else {
            register_reset(&s->regs_info[i]);
        }
    }
    update_mrs(s);
    isr_update_irq(s);
}

static bool xppu_ap_access(void *opaque, hwaddr addr, uint64_t *value, bool rw,
                           unsigned size, MemTxAttrs attr)
{
    XPPUAperture *ap = opaque;
    XPPU *s = ap->parent;
    uint32_t ram_offset;
    uint32_t apl;
    bool valid;
    bool isr_free;
    bool xppu_enabled = ARRAY_FIELD_EX32(s->regs, CTRL, ENABLE);

    assert(xppu_enabled);

    addr += ap->base;

    /* If any of bits ISR[7:1] are set, we cant store new faults.  */
    isr_free = (s->regs[R_ISR] & 0xf6) == 0;

    ram_offset = addr & ap->extract_mask;
    ram_offset >>= ap->extract_shift;

    ram_offset += ap->ram_base;
    apl = s->perm_ram[ram_offset];
    valid = xppu_ap_check(s, attr, rw, apl);

    if (!valid) {
        if (isr_free) {
            ARRAY_FIELD_DP32(s->regs, ISR, APER_PERM, true);
            s->regs[R_ERR_STATUS1] = addr >> 12;
            ARRAY_FIELD_DP32(s->regs, ERR_STATUS2, AXI_ID,
                             attr.requester_id);
        }

        isr_update_irq(s);
    } else {
        /* The access is accepted, let it through.  */
        *value = cpu_to_le64(*value);
        address_space_rw(&s->as, addr, attr, (uint8_t *) value, size, rw);
        *value = le64_to_cpu(*value);
    }

    /* If HIDE_EN is set, we always say the transaction was valid */
    if (ARRAY_FIELD_EX32(s->regs, CTRL, HIDE_EN)) {
        if (!rw) {
            *value = 0;
        }
        return true;
    }

    return valid;
}

static MemTxResult xppu_ap_read(void *opaque, hwaddr addr, uint64_t *value,
                                unsigned size, MemTxAttrs attr)
{
    if (xppu_ap_access(opaque, addr, value, false, size, attr)) {
        return MEMTX_OK;
    } else {
        return MEMTX_ERROR;
    }
}

static MemTxResult xppu_ap_write(void *opaque, hwaddr addr, uint64_t value,
                                unsigned size, MemTxAttrs attr)
{
    if (xppu_ap_access(opaque, addr, &value, true, size, attr)) {
        return MEMTX_OK;
    } else {
        return MEMTX_ERROR;
    }
}

static const MemoryRegionOps xppu_ap_ops = {
    .read_with_attrs = xppu_ap_read,
    .write_with_attrs = xppu_ap_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static XPPU *xppu_from_mr(void *mr_accessor)
{
    RegisterInfoArray *reg_array = mr_accessor;
    Object *obj;

    assert(reg_array != NULL);

    obj = reg_array->mem.owner;
    assert(obj);

    return XILINX_XPPU(obj);
}

static MemTxResult xppu_read(void *opaque, hwaddr addr, uint64_t *val,
                             unsigned size, MemTxAttrs attr)
{
    XPPU *s = xppu_from_mr(opaque);

    return xppu_read_common(opaque, s, addr, val, size, attr);
}

static MemTxResult xppu_write(void *opaque, hwaddr addr, uint64_t val,
                              unsigned size, MemTxAttrs attr)
{
    XPPU *s = xppu_from_mr(opaque);
    bool locked;

    locked = ARRAY_FIELD_EX32(s->regs, LOCK, REGWRDIS);
    /* If the XPPU is locked, only ITR and ISR can be accessed */
    if (locked) {
        if ((addr != A_ISR) && (addr != A_ITR)) {
            qemu_log_mask(LOG_GUEST_ERROR, "%s: accessing locked register "\
                          "0x%"HWADDR_PRIx"\n",
                          object_get_canonical_path(OBJECT(s)), addr);
            return MEMTX_ERROR;
        }
    }

    return xppu_write_common(opaque, s, addr, val, size, attr);
}

static const MemoryRegionOps xppu_ops = {
    .read_with_attrs = xppu_read,
    .write_with_attrs = xppu_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static MemTxResult xppu_perm_ram_read(void *opaque, hwaddr addr, uint64_t *val,
                                      unsigned size, MemTxAttrs attr)
{
    XPPU *s = XILINX_XPPU(opaque);

    return xppu_perm_ram_read_common(s, addr, val, size, attr);
}

static MemTxResult xppu_perm_ram_write(void *opaque, hwaddr addr, uint64_t val,
                                       unsigned size, MemTxAttrs attr)
{
    XPPU *s = XILINX_XPPU(opaque);
    bool locked;

    locked = ARRAY_FIELD_EX32(s->regs, LOCK, REGWRDIS);
    /* If the XPPU is locked, only ITR and ISR can be accessed */
    if (locked) {
        if ((addr != A_ISR) && (addr != A_ITR)) {
            qemu_log_mask(LOG_GUEST_ERROR, "%s: accessing locked register "\
                          "0x%"HWADDR_PRIx"\n",
                          object_get_canonical_path(OBJECT(s)), addr);
            return MEMTX_ERROR;
        }
    }

    return xppu_perm_ram_write_common(s, addr, val, size, attr);
}

static const MemoryRegionOps xppu_perm_ram_ops = {
    .read_with_attrs = xppu_perm_ram_read,
    .write_with_attrs = xppu_perm_ram_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static void xppu_realize(DeviceState *dev, Error **errp)
{
    XPPU *s = XILINX_XPPU(dev);

    address_space_init(&s->as, s->mr ? s->mr : get_system_memory(),
                       object_get_canonical_path(OBJECT(dev)));
}

static void xppu_init(Object *obj)
{
    XPPU *s = XILINX_XPPU(obj);

    switch (s->region) {
    case XPPU_REGION_LPD:
    case XPPU_REGION_PMC:
        s->num_ap = 3;
        break;
    case XPPU_REGION_PMC_NPI:
        s->num_ap = 2;
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR, "Unknown XPPU region %d\n",
                      s->region);
        s->num_ap = 0;
        break;
    }
    s->ap = g_new(XPPUAperture, s->num_ap);

    xppu_init_common(s, obj, TYPE_XILINX_XPPU, &xppu_ops, &xppu_perm_ram_ops,
                     xppu_regs_info, ARRAY_SIZE(xppu_regs_info));
}

static bool xppu_parse_reg(FDTGenericMMap *obj, FDTGenericRegPropInfo reg,
                           Error **errp)
{
    XPPU *s = XILINX_XPPU(obj);
    bool found = true;
    XPPUApertureInfo ap_info;

    static const XPPUGranule granules[] = {
        GRANULE_64K,
        GRANULE_1M,
        GRANULE_512M
    };
    static const uint64_t xppu_lpd_bases[] = {
        0xff000000,
        0xfe000000,
        0xe0000000,
    };
    static const uint64_t xppu_pmc_bases[] = {
        0xf1000000,
        0xf0000000,
        0xc0000000,
    };
    static const uint64_t xppu_pmc_npi_bases[] = {
        0xf6000000,
        0xf7000000
    };
    static const uint64_t masks[] = {
        0xff << 16, /* 64K, bits 23:16.  */
        0x0f << 20, /* 1MB, bits 23:20.  */
        0, /* No extraction.  */
    };
    static const unsigned int shifts[] = {
        16, /* 64K, bits 23:16.  */
        20, /* 1MB, bits 23:20.  */
        0, /* No extraction.  */
    };
    static const uint32_t ram_bases[] = {
        0x0,
        0x180,
        0x190,
    };

    ap_info.masks = masks;
    ap_info.shifts = shifts;
    ap_info.ram_bases = ram_bases;
    ap_info.granules = granules;
    switch (s->region) {
    case XPPU_REGION_LPD:
        ap_info.bases = xppu_lpd_bases;

        break;
    case XPPU_REGION_PMC:
        ap_info.bases = xppu_pmc_bases;

        break;
    case XPPU_REGION_PMC_NPI:
        ap_info.bases = xppu_pmc_npi_bases;

        break;
    default:
        qemu_log("%s: Could not find information for XPPU region %d\n",
                 object_get_canonical_path(OBJECT(s)), s->region);
        found = false;

        break;
    }

    if (found) {
        return xppu_parse_reg_common(s, TYPE_XILINX_XPPU, reg, obj, &ap_info,
                                     &xppu_ap_ops, errp);
    }

    return false;
}

static const VMStateDescription vmstate_xppu = {
    .name = TYPE_XILINX_XPPU,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32_ARRAY(regs, XPPU, XPPU_R_MAX),
        VMSTATE_END_OF_LIST(),
    }
};

static Property xppu_properties[] = {
    DEFINE_PROP_UINT8("region", XPPU, region, XPPU_REGION_LPD),
    DEFINE_PROP_END_OF_LIST(),
};

static void xppu_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    FDTGenericMMapClass *fmc = FDT_GENERIC_MMAP_CLASS(klass);

    dc->reset = xppu_reset;
    device_class_set_props(dc, xppu_properties);
    dc->realize = xppu_realize;
    dc->vmsd = &vmstate_xppu;
    fmc->parse_reg = xppu_parse_reg;
}

static const TypeInfo xppu_info = {
    .name          = TYPE_XILINX_XPPU,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(XPPU),
    .class_init    = xppu_class_init,
    .instance_init = xppu_init,
    .interfaces    = (InterfaceInfo[]) {
        { TYPE_FDT_GENERIC_MMAP },
        { },
    },

};

static void xppu_register_types(void)
{
    type_register_static(&xppu_info);
}

type_init(xppu_register_types)
