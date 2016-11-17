/*
 * GPIO connected graphic button.
 *
 * Copyright (c) 2016 Liviu Ionescu.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "qemu/log.h"

#include <hw/cortexm/button-gpio.h>

#include <hw/cortexm/helper.h>

#if defined(CONFIG_VERBOSE)
#include "verbosity.h"
#endif

/* ----- Public ------------------------------------------------------------ */

void button_gpio_create_from_info(Object *parent, ButtonGPIOInfo *info_array,
        BoardGraphicContext *graphic_context)
{
    ButtonGPIOInfo *info;

    for (info = info_array; info->name; info++) {
        /* Create the button. */

        Object *button = cm_object_new(parent, info->name, TYPE_BUTTON_GPIO);

        cm_object_property_set_bool(button, info->active_low, "active-low");

        if (info->w && info->h) {
            /* Compute corner coordinates from centre coordinate. */
            cm_object_property_set_int(button, info->x - (info->w / 2),
                    "x-left");
            cm_object_property_set_int(button, info->x + (info->w / 2),
                    "x-right");
            cm_object_property_set_int(button, info->y - (info->h / 2),
                    "y-top");
            cm_object_property_set_int(button, info->y + (info->h / 2),
                    "y-bottom");
        }

        // TODO: connect to GPIO? Find the proper place.
        cm_object_realize(button);

        cortexm_graphic_board_add_button(graphic_context, BUTTON_STATE(button));

#if defined(CONFIG_VERBOSE)
        if (verbosity_level >= VERBOSITY_DETAILED) {
            printf("Button:");
            printf(" '%s\'", info->name);
            if (info->w && info->h) {
                printf(" %d*%d @(%d,%d)", info->w, info->h, info->x, info->y);
            }
            printf(" active %s", info->active_low ? "low" : "high");
            if (info->gpio_path) {
                printf(" '%s',%d", info->gpio_path, info->port_bit);
            }
            printf("\n");
        }
#endif /* defined(CONFIG_VERBOSE) */
    }
}

/* ----- Private ----------------------------------------------------------- */

/* Action when the button is pushed down. */
static void button_gpio_down_callback(ButtonState *button)
{
    qemu_log_function_name();

    ButtonGPIOState *state = BUTTON_GPIO_STATE(button);

    button->value = state->active_low ? 0 : 1;

    // TODO: trigger interrupt.
}

/* Action when the button is released. */
static void button_gpio_up_callback(ButtonState *button)
{
    qemu_log_function_name();

    ButtonGPIOState *state = BUTTON_GPIO_STATE(button);

    button->value = state->active_low ? 1 : 0;

    // TODO: trigger interrupt.
}

static void button_gpio_instance_init_callback(Object *obj)
{
    qemu_log_function_name();

    ButtonGPIOState *state = BUTTON_GPIO_STATE(obj);

    cm_object_property_add_bool(obj, "active-low", &state->active_low);

    state->active_low = true;
}

// Currently not used.
static void button_gpio_reset_callback(DeviceState *dev)
{
    qemu_log_function_name();
}

static void button_gpio_realize_callback(DeviceState *dev, Error **errp)
{
    qemu_log_function_name();
}

static void button_gpio_class_init_callback(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->reset = button_gpio_reset_callback;
    dc->realize = button_gpio_realize_callback;

    ButtonClass *bt_class = BUTTON_CLASS(klass);
    bt_class->down = button_gpio_down_callback;
    bt_class->up = button_gpio_up_callback;
}

static const TypeInfo button_gpio_type_info = {
    .name = TYPE_BUTTON_GPIO,
    .parent = TYPE_BUTTON_GPIO_PARENT,
    .instance_size = sizeof(ButtonGPIOState),
    .instance_init = button_gpio_instance_init_callback,
    .class_init = button_gpio_class_init_callback,
    .class_size = sizeof(ButtonGPIOClass) };

static void button_gpio_type_init(void)
{
    type_register_static(&button_gpio_type_info);
}

type_init(button_gpio_type_init);

/* ------------------------------------------------------------------------- */
