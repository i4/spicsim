#ifndef SPICBOARD_VCD_H
#define SPICBOARD_VCD_H

#include <stdbool.h>

bool vcd_init(void);
bool vcd_start(void);
void vcd_stop(void);
void vcd_close(void);

#endif /* SPICBOARD_VCD_H */
