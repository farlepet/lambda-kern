#include <io/input.h>
#include <string.h>

struct input_dev idevs[MAX_INPUT_DEVICES];

static uint16_t get_next_devid(uint16_t driver)
{
	int i = 0, l = -1;
	for(; i < MAX_INPUT_DEVICES; i++)
		if(idevs[i].valid != 0)
			if(idevs[i].id.s.driver == driver)
				if(l < idevs[i].id.s.device)
					l = idevs[i].id.s.device;

	return (uint16_t)(l + 1);
}

/**
 * Adds an input device to `input_dev`
 * 
 * @param driver the driver that the device will use
 * @param name the base name for the driver
 * @param name_by_id whether or not to append the device number to the name
 * @param id_to_alpha convert device id to a letter, only used if name_by_id == 1
 */
struct input_dev *add_input_dev(uint16_t driver, char *name, uint8_t name_by_id, uint8_t id_to_alpha)
{
	int i = 0;
	for(; i < MAX_INPUT_DEVICES; i++)
		if(idevs[i].valid == 0) break;
	if(i == MAX_INPUT_DEVICES)
	{
		return 0; // TODO: Produce an error
	}

	idevs[i].id.s.driver = driver;
	idevs[i].id.s.device = get_next_devid(driver);
	idevs[i].state = 0;
	idevs[i].valid = 1;

	memcpy(idevs[i].name, name, strlen(name) + 1);

	if(name_by_id)
	{
		if(id_to_alpha) idevs[i].name[strlen(name)] = (char)idevs[i].id.s.device + 'a';
		else            idevs[i].name[strlen(name)] = (char)idevs[i].id.s.device;

		idevs[i].name[strlen(name) + 1] = 0;
	}

	return &idevs[i];
}


struct input_dev *get_idevice(uint16_t driver, uint16_t device) {
	uint32_t n = (uint32_t)(driver << 16 | device);

	int i = 0;
	for(; i < MAX_INPUT_DEVICES; i++) {
		if(idevs[i].valid != 0)
			if(idevs[i].id.n == n)
				return &idevs[i];
	}

	return NULL;
}
