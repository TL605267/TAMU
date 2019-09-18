/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "ir_demod.h"
void print(char *str);

int main()
{
	u32 reg0;
	u32 reg0_prev;
	u32 reg1;
	u32 reg1_prev;
	u32 reg3;
	u32 reg3_prev;

    init_platform();
    while (1){
    	//IR_DEMOD_mWriteReg(XPAR_IR_DEMOD_0_S00_AXI_BASEADDR, IR_DEMOD_S00_AXI_SLV_REG0_OFFSET, 16);
    	reg0 = IR_DEMOD_mReadReg(XPAR_IR_DEMOD_0_S00_AXI_BASEADDR, IR_DEMOD_S00_AXI_SLV_REG0_OFFSET);
    	reg1 = IR_DEMOD_mReadReg(XPAR_IR_DEMOD_0_S00_AXI_BASEADDR, IR_DEMOD_S00_AXI_SLV_REG1_OFFSET);
    	reg3 = IR_DEMOD_mReadReg(XPAR_IR_DEMOD_0_S00_AXI_BASEADDR, IR_DEMOD_S00_AXI_SLV_REG3_OFFSET);
		if (reg0 != reg0_prev) {
			printf("Message = 0x%03x; ", reg0);
			printf("Counter = %d\n", reg1);
			reg0_prev = reg0;
		}
    }
    cleanup_platform();
    return 0;
}