## clock_rtl
set_property PACKAGE_PIN L16 [get_ports clock_rtl]
set_property IOSTANDARD LVCMOS33 [get_ports clock_rtl]
create_clock -add -name sys_clk_pin -period 10.00 -waveform {0 5} [get_ports clock_rtl]

## led_tri_o
set_property PACKAGE_PIN M14 [get_ports {led_tri_o[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led_tri_o[0]}]

set_property PACKAGE_PIN M15 [get_ports {led_tri_o[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led_tri_o[1]}]

set_property PACKAGE_PIN G14 [get_ports {led_tri_o[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led_tri_o[2]}]

set_property PACKAGE_PIN D18 [get_ports {led_tri_o[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {led_tri_o[3]}]

## Switches: connect lower 4 bits of GPIO to switches
set_property PACKAGE_PIN G15 [get_ports {switches_tri_i[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {switches_tri_i[0]}]

set_property PACKAGE_PIN P15 [get_ports {switches_tri_i[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {switches_tri_i[1]}]

set_property PACKAGE_PIN W13 [get_ports {switches_tri_i[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {switches_tri_i[2]}]

set_property PACKAGE_PIN T16 [get_ports {switches_tri_i[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {switches_tri_i[3]}]

## Push Buttons: connect upper 4 bits of GPIO to push buttons
set_property PACKAGE_PIN R18 [get_ports push_buttons_tri_i[0]]
set_property IOSTANDARD LVCMOS33 [get_ports push_buttons_tri_i[0]]

set_property PACKAGE_PIN P16 [get_ports push_buttons_tri_i[1]]
set_property IOSTANDARD LVCMOS33 [get_ports push_buttons_tri_i[1]]

set_property PACKAGE_PIN V16 [get_ports push_buttons_tri_i[2]]
set_property IOSTANDARD LVCMOS33 [get_ports push_buttons_tri_i[2]]

set_property PACKAGE_PIN Y16 [get_ports push_buttons_tri_i[3]]
set_property IOSTANDARD LVCMOS33 [get_ports push_buttons_tri_i[3]]
