#ifndef PCI_H
#define PCI_H

#include <stdint.h>

struct pci_config_space {
    uint16_t vendor;
    uint16_t device;
    uint16_t command;
    uint16_t status;
    uint8_t revision_id;
    uint8_t prog_if;
    uint8_t subclass;
    uint8_t class_code;
    uint8_t cache_line_size;
    uint8_t latency_timer;
    uint8_t header_type;
    uint8_t bist;

    uint8_t capabilities_pointer;

    uint8_t interrupt_line;
    uint8_t interrupt_pin;
};

void check_all_buses(void);

#endif
