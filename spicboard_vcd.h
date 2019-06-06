#ifndef SPICBOARD_VCD_H
#define SPICBOARD_VCD_H

#include <stdbool.h>

#include "sim_avr.h"

bool vcd_init(struct avr_t *);
bool vcd_start();
void vcd_stop();
void vcd_close();

#endif /* SPICBOARD_VCD_H */
