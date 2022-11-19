/*
 * qdev GPIO helpers
 *
 *  Copyright (c) 2009 CodeSourcery
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "hw/qdev-core.h"
#include "hw/irq.h"
#include "qapi/error.h"
#include "hw/pm_debug.h"

static NamedGPIOList *qdev_get_named_gpio_list(DeviceState *dev,
                                               const char *name)
{
    NamedGPIOList *ngl;

    QLIST_FOREACH(ngl, &dev->gpios, node) {
        /* NULL is a valid and matchable name. */
        if (g_strcmp0(name, ngl->name) == 0) {
            return ngl;
        }
    }

    ngl = g_malloc0(sizeof(*ngl));
    ngl->name = g_strdup(name);
    QLIST_INSERT_HEAD(&dev->gpios, ngl, node);
    return ngl;
}

void qdev_init_gpio_in_named_with_opaque(DeviceState *dev,
                                         qemu_irq_handler handler,
                                         void *opaque,
                                         const char *name, int n)
{
    int i;
    NamedGPIOList *gpio_list = qdev_get_named_gpio_list(dev, name);

    assert(gpio_list->num_out == 0 || !name);
    gpio_list->in = qemu_extend_irqs(gpio_list->in, gpio_list->num_in, handler,
                                     opaque, n);

    if (!name) {
        name = "unnamed-gpio-in";
    }

    /* Xilinx: For the FDT Generic GPIO magic we need this to be a wild card
     * and not the usual numbered GPIOs.
     */
    gchar *propname = g_strdup_printf("%s[*]", name);

    for (i = gpio_list->num_in; i < gpio_list->num_in + n; i++) {
        object_property_add_child(OBJECT(dev), propname,
                                  OBJECT(gpio_list->in[i]));
    }

    g_free(propname);
    gpio_list->num_in += n;
}

void qdev_init_gpio_in(DeviceState *dev, qemu_irq_handler handler, int n)
{
    qdev_init_gpio_in_named(dev, handler, NULL, n);
}

void qdev_init_gpio_out_named(DeviceState *dev, qemu_irq *pins,
                              const char *name, int n)
{
    int i;
    NamedGPIOList *gpio_list = qdev_get_named_gpio_list(dev, name);

    assert(gpio_list->num_in == 0 || !name);

    if (!name) {
        name = "unnamed-gpio-out";
    }
    memset(pins, 0, sizeof(*pins) * n);

    /* Xilinx: For the FDT Generic GPIO magic we need this to be a wild card
     * and not the usual numbered GPIOs.
     */
    gchar *propname = g_strdup_printf("%s[*]", name);

    for (i = 0; i < n; ++i) {
        object_property_add_link(OBJECT(dev), propname, TYPE_IRQ,
                                 (Object **)&pins[i],
                                 object_property_allow_set_link,
                                 OBJ_PROP_LINK_STRONG);
    }
    g_free(propname);
    gpio_list->num_out += n;
}

void qdev_init_gpio_out(DeviceState *dev, qemu_irq *pins, int n)
{
    qdev_init_gpio_out_named(dev, pins, NULL, n);
}

qemu_irq qdev_get_gpio_in_named(DeviceState *dev, const char *name, int n)
{
    char *propname = g_strdup_printf("%s[%d]",
                                     name ? name : "unnamed-gpio-in", n);
    return (qemu_irq)object_property_get_link(OBJECT(dev), propname, NULL);
}

qemu_irq qdev_get_gpio_in(DeviceState *dev, int n)
{
    return qdev_get_gpio_in_named(dev, NULL, n);
}

qemu_irq qdev_get_gpio_out_named(DeviceState *dev, const char *name, int n)
{
    char *propname = g_strdup_printf("%s[%d]",
                                     name ? name : "unnamed-gpio-out", n);
    return (qemu_irq)object_property_get_link(OBJECT(dev), propname, NULL);
}

qemu_irq qdev_get_gpio_out(DeviceState *dev, int n)
{
    return qdev_get_gpio_out_named(dev, NULL, n);
}

void qdev_connect_gpio_out_named(DeviceState *dev, const char *name, int n,
                                 qemu_irq pin)
{
    Error *errp = NULL;
    qemu_irq irq;
    if (!pin) {
        return;
    }
    char *propname = g_strdup_printf("%s[%d]",
                                     name ? name : "unnamed-gpio-out", n);

    irq = (qemu_irq)object_property_get_link(OBJECT(dev), propname, NULL);
    if (irq) {
        char *splitter_name;
        irq = qemu_irq_split(irq, pin);
        /* ugly, be a sure-fire way to get a unique name */
        splitter_name = g_strdup_printf("%s-split-%p", propname, irq);
        object_property_add_child(OBJECT(dev), splitter_name,OBJECT(irq));
    } else {
        irq = pin;
    }
    if (irq  && !OBJECT(irq)->parent) {
        /* We need a name for object_property_set_link to work.  If the
         * object has a parent, object_property_add_child will come back
         * with an error without doing anything.  If it has none, it will
         * never fail.  So we can just call it with a NULL Error pointer.
         */
        object_property_add_child(container_get(qdev_get_machine(),
                                                "/unattached"),
                                  "non-qdev-gpio[*]", OBJECT(pin));
    }
    object_property_set_link(OBJECT(dev), propname, OBJECT(irq), &errp);
    if (errp) {
        qemu_log_mask(LOG_FDT, "FAILED to connect %s.%s <-> %s\n",
                       object_get_canonical_path(OBJECT(dev)), propname,
                       object_get_canonical_path(OBJECT(pin)));
    }
    g_free(propname);
}

qemu_irq qdev_get_gpio_out_connector(DeviceState *dev, const char *name, int n)
{
    g_autofree char *propname = g_strdup_printf("%s[%d]",
                                     name ? name : "unnamed-gpio-out", n);

    qemu_irq ret = (qemu_irq)object_property_get_link(OBJECT(dev), propname,
                                                      NULL);

    return ret;
}

/* disconnect a GPIO output, returning the disconnected input (if any) */

static qemu_irq qdev_disconnect_gpio_out_named(DeviceState *dev,
                                               const char *name, int n)
{
    char *propname = g_strdup_printf("%s[%d]",
                                     name ? name : "unnamed-gpio-out", n);

    qemu_irq ret = (qemu_irq)object_property_get_link(OBJECT(dev), propname,
                                                      NULL);
    if (ret) {
        object_property_set_link(OBJECT(dev), propname, NULL, NULL);
    }
    g_free(propname);
    return ret;
}

qemu_irq qdev_intercept_gpio_out(DeviceState *dev, qemu_irq icpt,
                                 const char *name, int n)
{
    qemu_irq disconnected = qdev_disconnect_gpio_out_named(dev, name, n);
    qdev_connect_gpio_out_named(dev, name, n, icpt);
    return disconnected;
}

void qdev_connect_gpio_out(DeviceState *dev, int n, qemu_irq pin)
{
    qdev_connect_gpio_out_named(dev, NULL, n, pin);
}

void qdev_pass_gpios(DeviceState *dev, DeviceState *container,
                     const char *name)
{
    int i;
    NamedGPIOList *ngl = qdev_get_named_gpio_list(dev, name);

    for (i = 0; i < ngl->num_in; i++) {
        const char *nm = ngl->name ? ngl->name : "unnamed-gpio-in";
        char *propname = g_strdup_printf("%s[%d]", nm, i);

        object_property_add_alias(OBJECT(container), propname,
                                  OBJECT(dev), propname);
        g_free(propname);
    }
    for (i = 0; i < ngl->num_out; i++) {
        const char *nm = ngl->name ? ngl->name : "unnamed-gpio-out";
        char *propname = g_strdup_printf("%s[%d]", nm, i);

        object_property_add_alias(OBJECT(container), propname,
                                  OBJECT(dev), propname);
        g_free(propname);
    }
    QLIST_REMOVE(ngl, node);
    QLIST_INSERT_HEAD(&container->gpios, ngl, node);
}
