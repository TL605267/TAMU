`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 09/07/2018 10:47:37 AM
// Design Name: 
// Module Name: switch
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

// This is the module for module7: counter
module clock_divider(clk, out_clk);
    input clk; // Input clock, which is the fast clock signal
    output reg out_clk; // The output clock, the slower clock signal
    reg [23:0] clk_counter; // The counter is used to count the cycle of the fast clock
    
    initial begin
        out_clk = 0; // Initalize output clock to zero
    end
    
    always @(posedge clk) begin 
        if (clk_counter == 24'hFFFFFF) begin // If the number of fast clock cycles reach 2^23-1
            out_clk <= ~out_clk; // Toggle the clock output
            clk_counter <= 0; // Clear the counter
        end
        else begin
            clk_counter = clk_counter + 1; // If not, keep counting
        end
   end     
endmodule

module counter(BUTTONS, LEDS, CLOCK); // This is the counter module
    input [2:0] BUTTONS; // First define three button unputs: up, down, and reset
    input CLOCK; // Input clock signal
    output [3:0] LEDS; // The output of the counter are four LEDs
    reg [3:0] counter; // The LEDs are controlled using a 4-bit register
    wire my_clock; // Output of the clock divider, which is the slower clock
    clock_divider div(CLOCK, my_clock); // Instancation
    always @ (posedge my_clock) begin
        if (BUTTONS[0]) // If count up button is pressed
            counter <= counter + 1; // Register count up 1
        if (BUTTONS[1]) // If count down button is pressed
            counter <= counter - 1; // Register count down1
        if (BUTTONS[2]) //If reset button is pressed
            counter <= 0; // Set counter register to zero
    end

    assign LEDS[3:0] = counter[3:0]; // Connect LEDS with counter register
endmodule
