#ifndef INPUT_H
#define INPUT_H

#include <types.h>

#include <data/cbuff.h>

#define MAX_INPUT_DEVICES 64 //!< Maximum number of registered input devices

union input_id
{
	uint32_t n;
	struct id
	{
		uint16_t driver;
		uint16_t device;
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
	uint32_t data;              //!< The data for the event (ex: the keycode)
};


struct input_dev
{
	union input_id id;       /** Device ID */
	uint32_t       state;    /** State of the device, different for every device type */
	char           name[64]; /** When mounted, it will be at /dev/name */
	uint8_t        valid;    /** Whether or not this slot is being used */
	cbuff_t       *iev_buff; /** Input event buffer */
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
struct input_dev *add_input_dev(uint16_t driver, char *name, uint8_t name_by_id, uint8_t id_to_alpha);


struct input_dev *get_idevice(uint16_t driver, uint16_t device);

#endif
