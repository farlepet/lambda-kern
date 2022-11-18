#include <lambda/export.h>
#include <err/panic.h>
#include <io/input.h>
#include <mm/alloc.h>
#include <string.h>

llist_t idevs;

static uint16_t get_next_devid(uint16_t driver) {
    int l = -1;
    input_dev_t     *idev;
    llist_iterator_t iter;
    llist_iterator_init(&idevs, &iter);
    while(llist_iterate(&iter, (void **)&idev)) {
        if((idev->id.s.driver == driver) &&
           (idev->id.s.device > l)) {
            l = idev->id.s.device;
        }
    }

    return (uint16_t)(l + 1);
}

void add_input_dev(input_dev_t *idev, uint16_t driver, char *name, uint8_t name_by_id, uint8_t id_to_alpha) {
    if(idev == NULL) {
        kpanic("add_input_dev: idev pointer is NULL!");
    }
    idev->list_item.data = idev;
    idev->id.s.driver = driver;
    idev->id.s.device = get_next_devid(driver);
    idev->state = 0;

    /* TODO: Possibly dynamically allocate name, placing directly after the input_dev_t struct. */
    memcpy(idev->name, name, strlen(name) + 1);

    if(name_by_id)
    {
        if(id_to_alpha) idev->name[strlen(name)] = (char)idev->id.s.device + 'a';
        else            idev->name[strlen(name)] = (char)idev->id.s.device;

        idev->name[strlen(name) + 1] = 0;
    }

    llist_append(&idevs, &idev->list_item);
}
EXPORT_FUNC(add_input_dev);


input_dev_t *get_idevice(uint16_t driver, uint16_t device) {
    uint32_t n = (uint32_t)(driver << 16 | device);

    input_dev_t     *idev;
    llist_iterator_t iter;
    llist_iterator_init(&idevs, &iter);
    while(llist_iterate(&iter, (void **)&idev)) {
        if(idev->id.n == n) {
            return idev;
        }
    }

    return NULL;
}
