/*
 * obd.h
 *
 *  Created on: 2021-08-31
 *      Author: GDR
 */
#ifndef OBD_H_
#define OBD_H_

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"

extern cy_stc_canfd_context_t canfd_context;
extern const cy_stc_sysint_t irq_cfg;

void CANFD_RxMsgCallback(bool bRxFifoMsg, uint8_t u8MsgBufOrRxFifoNum, cy_stc_canfd_rx_buffer_t* pstcCanFDmsg);
void CanfdInterruptHandler(void);
cy_rslt_t  obd_can_init(void);
_Bool obd_engine_speed(uint32_t *rpm);
_Bool obd_vehicle_speed(uint32_t *kmh);
_Bool obd_ctrl_mod_voltage(uint32_t *volts);
_Bool obd_in_air_temp(int32_t *int_air);
_Bool obd_in_air_press(uint32_t *int_press);
_Bool obd_air_mass(uint32_t *air_flow);
_Bool obd_is_online(void);
_Bool turbo_boost(int32_t *boost);

#endif /* OBD_H_ */
