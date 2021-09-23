/*
 * obd.c
 *
 *  Created on: 2021-08-31
 *      Author: GDR
 */
#include "obd.h"

/*CANFD Interrupt Configuration*/
const cy_stc_sysint_t irq_cfg =
{
	canfd_0_interrupts0_0_IRQn,
    0UL
};

/*CANFD Interrupt Configuration*/
cy_stc_canfd_context_t canfd_context;

/*Global variable for the reference to the CAN message*/
uint32_t *rx_address = NULL;

/* CANFD interrupt handler */
void CanfdInterruptHandler(void)
{
    /* Just call the IRQ handler with the current channel number and context */
    Cy_CANFD_IrqHandler(CANFD0, 0, &canfd_context);
}

/* CANFD reception callback */
void CANFD_RxMsgCallback(bool bRxFifoMsg, uint8_t u8MsgBufOrRxFifoNum, cy_stc_canfd_rx_buffer_t* pstcCanFDmsg)
{
    (void)bRxFifoMsg;
    (void)u8MsgBufOrRxFifoNum;

    if(0 == pstcCanFDmsg->r0_f->rtr)
    {	/*Place the reference to the message */
    	if(!rx_address)
    	{
    		rx_address = pstcCanFDmsg->data_area_f;
    	}
    }
}

cy_rslt_t  obd_can_init(void)
{
	cy_rslt_t  result = CY_CANFD_SUCCESS;

	result = Cy_CANFD_Init (CANFD0, 0, &CAN_FD_config, &canfd_context);
    if(CY_CANFD_SUCCESS != result )
    {return result;}

    /* Hook the interrupt service routine and enable the interrupt */
    (void) Cy_SysInt_Init(&irq_cfg, &CanfdInterruptHandler);
    NVIC_EnableIRQ(canfd_0_interrupts0_0_IRQn);

	return result;
}

_Bool msg_request(cy_stc_canfd_tx_buffer_t *tx_msg, uint32_t **rx_data, uint32_t timeout)
{
	cy_en_canfd_status_t result;
	rx_address = NULL;

	result = Cy_CANFD_UpdateAndTransmitMsgBuffer(CANFD0, 0u, tx_msg, 0u, &canfd_context);
    if (result != CY_CANFD_SUCCESS)
    {
    	return false;
    }

    while(timeout)
    {
    	if(rx_address)
    	{
    		*rx_data = rx_address;
    		return true;
    	}

    	CyDelay(1);
    	timeout--;
    }
	return false;
}

_Bool obd_is_online(void)
{
	_Bool result = false;
	uint8_t data[8] = {0x02,0x01,0x00,0x55,0x55,0x55,0x55,0x55};
	uint8_t *rx = NULL;

    /* Prepare data to send */
    memcpy(CAN_FD_dataBuffer_0, data, 8);

    result = msg_request(&CAN_FD_txBuffer_0, (uint32_t**)&rx, 100);

	return result;
}

_Bool obd_engine_speed(uint32_t *rpm)
{
	_Bool result = false;
	uint8_t data[8] = {0x02,0x01,0x0C,0x55,0x55,0x55,0x55,0x55};
	uint8_t *rx = NULL;

    /* Prepare data to send */
    memcpy(CAN_FD_dataBuffer_0, data, 8);

    result = msg_request(&CAN_FD_txBuffer_0, (uint32_t**)&rx, 100);
    if(result)
    {
    	*rpm = (rx[3]*256 + rx[4])/4;
    }

	return result;
}

_Bool obd_vehicle_speed(uint32_t *kmh)
{
	_Bool result = false;
	uint8_t data[8] = {0x02,0x01,0x0D,0x55,0x55,0x55,0x55,0x55};
	uint8_t *rx = NULL;

    /* Prepare data to send */
    memcpy(CAN_FD_dataBuffer_0, data, 8);

    result = msg_request(&CAN_FD_txBuffer_0, (uint32_t**)&rx, 100);
    if(result)
    {
    	*kmh = rx[3];
    }

	return result;
}

_Bool obd_ctrl_mod_voltage(uint32_t *volts)
{
	_Bool result = false;
	uint8_t data[8] = {0x02,0x01,0x42,0x55,0x55,0x55,0x55,0x55};
	uint8_t *rx = NULL;

    /* Prepare data to send */
    memcpy(CAN_FD_dataBuffer_0, data, 8);

    result = msg_request(&CAN_FD_txBuffer_0, (uint32_t**)&rx, 100);
    if(result)
    {
    	*volts = (rx[3]*256 + rx[4])/1000;
    }

	return result;
}

_Bool obd_in_air_temp(int32_t *int_air)
{
	_Bool result = false;
	uint8_t data[8] = {0x02,0x01,0x0F,0x55,0x55,0x55,0x55,0x55};
	uint8_t *rx = NULL;

    /* Prepare data to send */
    memcpy(CAN_FD_dataBuffer_0, data, 8);

    result = msg_request(&CAN_FD_txBuffer_0, (uint32_t**)&rx, 100);
    if(result)
    {
    	*int_air = rx[3] - 40;
    }

	return result;
}

_Bool obd_in_air_press(uint32_t *int_press)
{
	_Bool result = false;
	uint8_t data[8] = {0x02,0x01,0x0B,0x55,0x55,0x55,0x55,0x55};
	uint8_t *rx = NULL;

    /* Prepare data to send */
    memcpy(CAN_FD_dataBuffer_0, data, 8);

    result = msg_request(&CAN_FD_txBuffer_0, (uint32_t**)&rx, 100);
    if(result)
    {
    	*int_press = rx[3];
    }

	return result;
}

_Bool obd_air_mass(uint32_t *air_flow)
{
	_Bool result = false;
	uint8_t data[8] = {0x02,0x01,0x0B,0x55,0x55,0x55,0x55,0x55};
	uint8_t *rx = NULL;

    /* Prepare data to send */
    memcpy(CAN_FD_dataBuffer_0, data, 8);

    result = msg_request(&CAN_FD_txBuffer_0, (uint32_t**)&rx, 100);
    if(result)
    {
    	*air_flow = (rx[3]*256 + rx[4])/100;
    }

	return result;
}

_Bool turbo_boost(int32_t *boost)
{
	_Bool result = false;
	uint8_t request_1[8] = {0x02,0x01,0x0B,0x55,0x55,0x55,0x55,0x55};
	uint8_t request_2[8] = {0x02,0x01,0x33,0x55,0x55,0x55,0x55,0x55};
	uint8_t *rx = NULL;
	uint32_t map = 0;

    /* Prepare data to send */
    memcpy(CAN_FD_dataBuffer_0, request_1, 8);

    result = msg_request(&CAN_FD_txBuffer_0, (uint32_t**)&rx, 100);
    if(result)
    {
    	map = rx[3] * 1000;
        /* Prepare data to send */
        memcpy(CAN_FD_dataBuffer_0, request_2, 8);
        result = msg_request(&CAN_FD_txBuffer_0, (uint32_t**)&rx, 100);
        if(result)
        {
        	*boost = map - rx[3] * 1000;
        	return true;
        }
        else
        {
        	*boost = map - 101325;
        	return true;
        }
    }

	return result;
}

