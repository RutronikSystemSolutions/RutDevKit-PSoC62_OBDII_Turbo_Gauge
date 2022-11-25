/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the RutDevKit-PSoC62_OBDII_Turbo_Gauge
*              Application for ModusToolbox.
*
* Related Document: See README.md
*
*
*  Created on: 2021-05-27
*  Company: Rutronik Elektronische Bauelemente GmbH
*  Address: Jonavos g. 30, Kaunas 44262, Lithuania
*  Author: GDR
*
*******************************************************************************
* (c) 2019-2021, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*
* Rutronik Elektronische Bauelemente GmbH Disclaimer: The evaluation board
* including the software is for testing purposes only and,
* because it has limited functions and limited resilience, is not suitable
* for permanent use under real conditions. If the evaluation board is
* nevertheless used under real conditions, this is done at one’s responsibility;
* any liability of Rutronik is insofar excluded
*******************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "cycfg_qspi_memslot.h"
#include "cy_serial_flash_qspi.h"
#include "EVE_target.h"
#include "tft.h"
#include "obd.h"

#define QSPI_BUS_FREQUENCY_HZ   (50000000lu)

void handle_error(void);

int main(void)
{
	cy_rslt_t result;
	volatile uint8_t display_delay = 0;


    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
    	handle_error();
    }

    /*Enable debug output via KitProg UART*/
    result = cy_retarget_io_init( KITPROG_TX, KITPROG_RX, CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }
    printf("\x1b[2J\x1b[;H");
    printf("OBDII 4D Systems Display Application.\n\r");

    /*Initialize LEDs*/
    result = cyhal_gpio_init( LED1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {handle_error();}
    result = cyhal_gpio_init( LED2, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {handle_error();}

    /*Initialize CANFD Driver Stand-By pin */
    result = cyhal_gpio_init( CANFD_STB, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    if (result != CY_RSLT_SUCCESS)
    {handle_error();}

    /* Initialize the QSPI block and enter XIP mode*/
    result = cy_serial_flash_qspi_init(smifMemConfigs[1],
    		QSPI_IO0, QSPI_IO1,
    		QSPI_IO2, QSPI_IO3, NC, NC, NC, NC, QSPI_CLK,
			FLASH_SSEL, QSPI_BUS_FREQUENCY_HZ);
    if (result != CY_RSLT_SUCCESS)
    {handle_error();}
    result = cy_serial_flash_qspi_enable_xip(true);
    if (result != CY_RSLT_SUCCESS)
    {handle_error();}

    /*Initialize CAN FD and OBD*/
    if(CY_CANFD_SUCCESS != obd_can_init())
    {handle_error();}

    /*Initialize TFT Display*/
    result = EVE_init_spi();
    if (result != CY_RSLT_SUCCESS)
    {handle_error();}
    TFT_init();

    /*Enable interrupts*/
    __enable_irq();

    for (;;)
    {
    	CyDelay(5);
    	TFT_touch();

    	display_delay++;
		if(display_delay > 3)
		{
			display_delay = 0;

	        if(obd_is_online())
	        {
	        	cyhal_gpio_write(LED2, CYBSP_LED_STATE_OFF);
	        	cyhal_gpio_toggle(LED1);
	        	(void)obd_engine_speed(&rpm);
	        	(void)obd_ctrl_mod_voltage(&vlt);
	        	(void)obd_in_air_temp(&air_tmp);
	        	(void)obd_in_air_press(&map);
	        	(void)obd_air_mass(&maf);
	        	(void)turbo_boost(&boost);
	        }
	        else
	        {
	        	cyhal_gpio_write(LED1, CYBSP_LED_STATE_OFF);
	        	cyhal_gpio_toggle(LED2);
	        	boost = -101325;
	        	rpm = 0;
	        	vlt = 0;
	        	air_tmp = 0;
	        	map = 0;
	        	maf = 0;
	        }

			TFT_display();
		}
    }
}

void handle_error(void)
{
     /* Disable all interrupts. */
    __disable_irq();
    cyhal_gpio_write(LED1, CYBSP_LED_STATE_OFF);
    cyhal_gpio_write(LED2, CYBSP_LED_STATE_ON);
    CY_ASSERT(0);
}
/* [] END OF FILE */
