`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 09/08/2018 01:31:37 PM
// Design Name: 
// Module Name: jackpot
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////

module clock_divider(clk_in, clk_out); // same clock divider from step 6
    input clk_in; // The input clock signal, fast clock
    output reg clk_out; // The output clock signal, the slower clock
    reg [23:0] clk_counter; // The counter is used to count the clock cycles
    
    initial begin
        clk_out = 0; // Initalize output clock signal to zero
    end
    
    always @(posedge clk_in) begin // at positive edge of input clock
        if (clk_counter == 24'hFFFFFF) begin // Count clock cycle up to 2^23-1
            clk_out <= ~clk_out; // Toggle output clock signal
            clk_counter <= 0; // Clear the counter register
        end
        else begin
            clk_counter = clk_counter + 1; // If clock cycle haven't reach 2^23-1, keep counting
        end
   end     
endmodule

module jackpot(SWITCHES, LEDS, CLOCK, RESET); // jackpot implemented from state machine
    input [3:0] SWITCHES; // Initalize switches input
    input CLOCK; // Initalize clock signal
    input RESET; // Initalize reset button
    output reg [3:0] LEDS; // Initalize output LEDs
    wire my_clk; // This is the slow clock signal 
    reg [1:0] state; // 
    reg prev_state;
    reg [3:0] led_pattern;
    parameter IDLE = 2'b00, PUSHED = 2'b01, MISSED = 2'b10, WIN = 2'b11; 
    clock_divider clk_div(CLOCK, my_clk);
    
    always @ (posedge my_clk or posedge RESET) begin
        if (RESET) led_pattern <= 4'b0001;
        else led_pattern <= led_pattern[3] ? 4'b0001 : led_pattern << 1;
    end
    
    always @ (posedge CLOCK) begin // jackpot logic (FSM)
        case(state)
            IDLE: begin
                LEDS <= led_pattern;
                if (SWITCHES) state <= PUSHED;
            end
            PUSHED: begin
                if (SWITCHES != LEDS) 
                    state <= MISSED;
                else state <= WIN;
            end
            MISSED: begin
                if (!SWITCHES) state <= IDLE;
                LEDS <= led_pattern;
            end
            WIN: begin
                LEDS <= 4'b1111;
                if (RESET) state <= SWITCHES? MISSED: IDLE;
            end
        endcase
    end
endmodule
