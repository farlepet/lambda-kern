#ifndef INPUT_H
#define INPUT_H

#include <types.h>

#define MAX_INPUT_DEVICES 64 //!< Maximum number of registered input devices

union input_id
{
	u32 n;
	struct id
	{
		u16 driver;
		u16 device;
	} s;
};

enum event_type
{
	EVENT_KEYPRESS, //!< A key was pressed (ex: keyboard)
	EVENT_CHAR,     //!< A character was received (ex: serial)
	EVENT_HWUPDATE, //!< The hardware was updated
	EVENT_MOVEMENT  //!< Something was moved (ex: mouse)
};

enum builtin_idrivers //!< Input drivers recognized by the kernel
{
	IDRIVER_KEYBOARD = 0, //!< Keyboard driver
	IDRIVER_MOUSE,        //!< Mouse driver
	IDRIVER_SERIAL        //!< Serial driver
};

enum keyb_idrv_state
{
	KEYB_STATE_SHIFT = 1,
	KEYB_STATE_CTRL  = 2,
	KEYB_STATE_ALT   = 4,
	KEYB_STATE_SUPER = 5
};


struct input_event
{
	union input_id origin; //!< From where the event originated
	enum event_type type;  //!< What kind of event was it
	u32 data;              //!< The data for the event (ex: the keycode)
};


struct input_dev
{
	union input_id id;

	u32 state;         //!< State of the device, different for every device type

	char name[64];     //!< When mounted, it will be at /dev/name

	u8  valid;         //!< Whether or not this slot is being used
};

extern struct input_dev idevs[MAX_INPUT_DEVICES];


/**
 * Adds an input device to `input_dev`
 * 
 * @param driver the driver that the device will use
 * @param name the base name for the driver
 * @param name_by_id whether or not to append the device number to the name
 * @param id_to_alpha convert device id to a letter, only used if name_by_id == 1
 */
struct input_dev *add_input_dev(u16 driver, char *name, u8 name_by_id, u8 id_to_alpha);


struct input_dev *get_idevice(u16 driver, u16 device);

#endif
