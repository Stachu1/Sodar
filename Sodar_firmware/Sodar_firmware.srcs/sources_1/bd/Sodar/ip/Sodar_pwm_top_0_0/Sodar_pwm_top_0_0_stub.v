// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2019.1 (win64) Build 2552052 Fri May 24 14:49:42 MDT 2019
// Date        : Thu May  1 11:19:39 2025
// Host        : SBA-UDV running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub
//               C:/Output/Sodar/Sodar_firmware/Sodar_firmware.srcs/sources_1/bd/Sodar/ip/Sodar_pwm_top_0_0/Sodar_pwm_top_0_0_stub.v
// Design      : Sodar_pwm_top_0_0
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7z020clg484-1
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* x_core_info = "pwm_top,Vivado 2019.1" *)
module Sodar_pwm_top_0_0(clk, duty_cycle, period, phase, pwm_out)
/* synthesis syn_black_box black_box_pad_pin="clk,duty_cycle[31:0],period[31:0],phase[31:0],pwm_out[18:0]" */;
  input clk;
  input [31:0]duty_cycle;
  input [31:0]period;
  input [31:0]phase;
  output [18:0]pwm_out;
endmodule
