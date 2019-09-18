#include <xparameters.h>
#include <xgpio.h>
#include <xstatus.h>
#include <xil_printf.h>

/* Definitions */
#define GPIO_DEVICE_ID_LED XPAR_LED_DEVICE_ID   /* GPIO device that LEDs are connected to */
#define GPIO_DEVICE_ID_SW_BTN XPAR_AXI_GPIO_1_DEVICE_ID   /* GPIO device that LEDs are connected to */

#define WAIT_VAL 0x10000000

int delay(void);

int main() {

    int count;
    int count_masked;
    XGpio leds;
    XGpio sws_btns; // switches and buttons
    int status;
    short sw_status, btn_status; // Varibles that store the status of switch buttons (sw_status) and push buttons (btn_status
    short btn3_flg, btn4_flg; // Flag varibles to store the status of button3 and button4. This flag is used to prevent the countinuous print out of current action on TCL console when button3 or button4 is holding down
	// Initalize LEDs
    status = XGpio_Initialize(&leds, GPIO_DEVICE_ID_LED);
    XGpio_SetDataDirection(&leds, 1, 0x00);
    if(status != XST_SUCCESS) { // Error handler
        xil_printf("Initialization failed");
    }
	// Initalize switches and push buttons on the dual-channel 8-bit GPIO
    status = XGpio_Initialize(&sws_btns, GPIO_DEVICE_ID_SW_BTN);
    XGpio_SetDataDirection(&sws_btns, 1, 0x01); // Switches: GPIO channel 1, set as input
    XGpio_SetDataDirection(&sws_btns, 2, 0x01); // Push buttons: GPIO channel 1, set as input
    if(status != XST_SUCCESS) { // Error handler
        xil_printf("Initialization failed");    
    }
	// Initalize local varibles
    count = 0;
    btn3_flg = 0;
    btn4_flg = 0;

    while(1) {
    	sw_status = XGpio_DiscreteRead(&sws_btns, 1); // read switches status (GPIO channel 1)
    	btn_status = XGpio_DiscreteRead(&sws_btns, 2); // read buttons status (GPIO channel 2)
    	switch(btn_status) { // Using switch cases for different button status
    	case 0b0001: // Button 1 is pressed
            count++; // COUNT increment
            count_masked = count & 0xF; // Since we only have 4 LEDS, we just read the lower 4 bits of the counter here
            xil_printf("COUNT = 0x%x (Increment);                  LEDs = 0x0\n\r", count_masked); // Print out LED value as well as the current action on TCL console
            delay();
            if (btn3_flg == 1) btn3_flg = 0; // Since button1 was pressed in this loop cycle, set the button3 flag to 0
            if (btn4_flg == 1) btn4_flg = 0; // Since button1 was pressed in this loop cycle, set the button4 flag to 0
            break;
    	case 0b0010: // Button 2 is pressed
            count--; // COUNT decrement
            count_masked = count & 0xF; // Since we only have 4 LEDS, we just read the lower 4 bits of the counter here
            xil_printf("COUNT = 0x%x (Decrement);                  LEDs = 0x0\n\r", count_masked); // Print out LED value as well as the current action on TCL console
            delay();
            if (btn3_flg == 1) btn3_flg = 0; // Since button2 was pressed in this loop cycle, set the button3 flag to 0
            if (btn4_flg == 1) btn4_flg = 0; // Since button2 was pressed in this loop cycle, set the button4 flag to 0
            break;
    	case 0b0100: // Button 3 is pressed 
    		if (btn3_flg == 0) { // If button3 wasn't pressed in the previous loop cycle
				xil_printf("switches = 0x%x (Display switches status); LEDs = 0x%x\n\r", sw_status, sw_status); // Display status of switches
				XGpio_DiscreteWrite(&leds, 1, sw_status); // Write the switch status to LEDs
				btn3_flg = 1; // Since button3 was pressed in this loop cycle, set the button3 flag to 1
    		}
    		else { // If button3 was pressed in the previous loop cycle
				XGpio_DiscreteWrite(&leds, 1, sw_status); // Then only display switches statys on LEDs, don't print the same message on TCL console
    		}
            if (btn4_flg == 1) btn4_flg = 0; // Since button3 was pressed in this loop cycle, set the button4 flag to 0
            break;
    	case 0b1000: // Button 4 is pressed 
    		if (btn4_flg == 0) { // If button4 was not pressed in the previous loop cycle
				count_masked = count & 0xF; // Get the lower 4 bit of the count varible
				xil_printf("COUNT = 0x%x (Display COUNT on LEDs);      LEDs = 0x%x\n\r", count_masked, count_masked); // Print out current actuon and LED values on TCL console
				XGpio_DiscreteWrite(&leds, 1, count_masked); // Display COUNT on LEDs
				btn4_flg = 1; // Since button3 was pressed in this loop cycle, set the button4 flag to 1
    		}
    		else { // If button4 was pressed in the previous loop cycle
				XGpio_DiscreteWrite(&leds, 1, count_masked);// Then only display COUNT on LEDs, don't print the same message on TCL console
    		}
            if (btn3_flg == 1) btn3_flg = 0; // Since button4 was pressed in this loop cycle, set the button3 flag to 0
            break;
    	default: // In all other cases
            XGpio_DiscreteWrite(&leds, 1, 0); // Turn off LEDs
            if (btn3_flg == 1) btn3_flg = 0; // Set button3 flag to 0
            if (btn4_flg == 1) btn4_flg = 0; // Set button4 flag to 0
    		break;
    	}
    }
    return (0);
}

int delay(void) {
    volatile int delay_count = 0;
    while(delay_count < WAIT_VAL)
        delay_count++;
    return(0);
}

