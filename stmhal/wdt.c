/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>

#include STM32_HAL_H

#include "py/runtime.h"
#include "wdt.h"

typedef struct _pyb_wdt_obj_t {
    mp_obj_base_t base;
} pyb_wdt_obj_t;

STATIC pyb_wdt_obj_t pyb_wdt = {{&pyb_wdt_type}};

STATIC mp_obj_t pyb_wdt_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 2, 2, false);

    mp_int_t id = mp_obj_get_int(args[0]);
    if (id != 0) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "WDT(%d) does not exist", id));
    }

    // timeout is in milliseconds
    mp_int_t timeout = mp_obj_get_int(args[1]);

    // compute prescaler
    uint32_t prescaler;
    for (prescaler = 0; prescaler < 6 && timeout >= 512; ++prescaler, timeout /= 2) {
    }

    // convert milliseconds to ticks
    timeout *= 8; // 32kHz / 4 = 8 ticks per millisecond (approx)
    if (timeout <= 0) {
        mp_raise_ValueError("WDT timeout too short");
    } else if (timeout > 0xfff) {
        mp_raise_ValueError("WDT timeout too long");
    }
    timeout -= 1;

    // set the reload register
    while (IWDG->SR & 2) {
    }
    IWDG->KR = 0x5555;
    IWDG->RLR = timeout;

    // set the prescaler
    while (IWDG->SR & 1) {
    }
    IWDG->KR = 0x5555;
    IWDG->PR = prescaler;

    // start the watch dog
    IWDG->KR = 0xcccc;

    return (mp_obj_t)&pyb_wdt;
}

STATIC mp_obj_t pyb_wdt_feed(mp_obj_t self_in) {
    (void)self_in;
    IWDG->KR = 0xaaaa;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_wdt_feed_obj, pyb_wdt_feed);

STATIC const mp_map_elem_t pyb_wdt_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_feed), (mp_obj_t)&pyb_wdt_feed_obj },
};

STATIC MP_DEFINE_CONST_DICT(pyb_wdt_locals_dict, pyb_wdt_locals_dict_table);

const mp_obj_type_t pyb_wdt_type = {
    { &mp_type_type },
    .name = MP_QSTR_WDT,
    .make_new = pyb_wdt_make_new,
    .locals_dict = (mp_obj_t)&pyb_wdt_locals_dict,
};