
#ifndef TFT_H_
#define TFT_H_

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

extern uint16_t gauge_val;
extern uint32_t rpm;
extern uint32_t vlt;
extern int32_t air_tmp;
extern uint32_t map;
extern uint32_t maf;
extern int32_t boost;

void TFT_init(void);
void TFT_touch(void);
void TFT_display(void);

#endif /* TFT_H_ */
