/***************************************************************************
 *
 * Multitouch X driver
 * Copyright (C) 2008 Henrik Rydberg <rydberg@euromail.se>
 *
 * Licensed under the Academic Free License version 2.1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 **************************************************************************/

#include "mtouch.h"

////////////////////////////////////////////////////////////////////////////

static int device_init(LocalDevicePtr local)
{
    struct MTouch *mt = local->private;
    local->fd = xf86OpenSerial(local->options);
    if (local->fd < 0) {
	    xf86Msg(X_ERROR, "multitouch: cannot configure device\n");
	    return local->fd;
    }
    if (configure_mtouch(mt, local->fd))
	    return -1;
    xf86CloseSerial(local->fd);
    return 0;
}

////////////////////////////////////////////////////////////////////////////

static int device_on(LocalDevicePtr local)
{
    struct MTouch *mt = local->private;
    local->fd = xf86OpenSerial(local->options);
    if (local->fd < 0) {
	    xf86Msg(X_ERROR, "multitouch: cannot open device\n");
	    return local->fd;
    }
    if (init_mtouch(mt))
	    return -1;
    return 0;
}

////////////////////////////////////////////////////////////////////////////

static void device_off(LocalDevicePtr local)
{
	if(local->fd >= 0)
		xf86CloseSerial(local->fd);
}

////////////////////////////////////////////////////////////////////////////

static void device_close(LocalDevicePtr local)
{
}

////////////////////////////////////////////////////////////////////////////

/* called for each full received packet from the touchpad */
static void read_input(LocalDevicePtr local)
{
    struct MTouch *mt = local->private;

    xf86Msg(X_INFO, "read_input called\n");

    if (local->fd >= 0) {
	    while (!read_hwdata(&mt->hw, local->fd)) {
		    // do all the good stuff here
	    }
    }
}

////////////////////////////////////////////////////////////////////////////

static Bool device_control(DeviceIntPtr dev, int mode)
{
	LocalDevicePtr local = dev->public.devicePrivate;
	switch (mode) {
	case DEVICE_INIT:
		xf86Msg(X_INFO, "device control: init\n");
		if (device_init(local))
			return !Success;
		return Success;
	case DEVICE_ON:
		xf86Msg(X_INFO, "device control: on\n");
		if (device_on(local))
			return !Success;
		return Success;
	case DEVICE_OFF:
		xf86Msg(X_INFO, "device control: off\n");
		device_off(local);
		return Success;
	case DEVICE_CLOSE:
		xf86Msg(X_INFO, "device control: close\n");
		device_close(local);
		return Success;
	default:
		xf86Msg(X_INFO, "device control: default\n");
		return BadValue;
    }
}


////////////////////////////////////////////////////////////////////////////

static InputInfoPtr preinit(InputDriverPtr drv, IDevPtr dev, int flags)
{
	struct MTouch *mt;
	InputInfoPtr local = xf86AllocateInput(drv, 0);
	if (!local)
		goto error;
	mt = xcalloc(1, sizeof(struct MTouch));
	if (!mt)
		goto error;

	local->name = dev->identifier;
	local->type_name = XI_TOUCHPAD;
	local->device_control = device_control;
	local->read_input = read_input;
	local->private = mt;
	local->private_flags = 0;
	local->flags = XI86_POINTER_CAPABLE | XI86_SEND_DRAG_EVENTS;
	local->conf_idev = dev;
	local->always_core_feedback = 0;

	xf86CollectInputOptions(local, NULL, NULL);
	xf86OptionListReport(local->options);

	local->flags |= XI86_CONFIGURED;
 error:
	return local;
}

static void uninit(InputDriverPtr drv, InputInfoPtr local, int flags)
{
	xfree(local->private);
	xf86DeleteInput(local, 0);
}

////////////////////////////////////////////////////////////////////////////

static InputDriverRec MULTITOUCH = {
    1,
    "multitouch",
    NULL,
    preinit,
    uninit,
    NULL,
    0
};

static XF86ModuleVersionInfo VERSION = {
    "multitouch",
    MODULEVENDORSTRING,
    MODINFOSTRING1,
    MODINFOSTRING2,
    XORG_VERSION_CURRENT,
    0, 1, 0,
    ABI_CLASS_XINPUT,
    ABI_XINPUT_VERSION,
    MOD_CLASS_XINPUT,
    {0, 0, 0, 0}
};

static pointer setup(pointer module, pointer options, int *errmaj, int *errmin)
{
    xf86AddInputDriver(&MULTITOUCH, module, 0);
    return module;
}

XF86ModuleData multitouchModuleData = {&VERSION, &setup, NULL };

////////////////////////////////////////////////////////////////////////////