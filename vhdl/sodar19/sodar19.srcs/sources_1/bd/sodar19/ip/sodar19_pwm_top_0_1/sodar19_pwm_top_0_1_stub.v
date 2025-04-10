// Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2019.1 (win64) Build 2552052 Fri May 24 14:49:42 MDT 2019
// Date        : Thu Apr 10 15:46:37 2025
// Host        : Bruno-Komp-MSI running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub {C:/Users/plywa/Digital designe
//               vivado/sodar19/sodar19.srcs/sources_1/bd/sodar19/ip/sodar19_pwm_top_0_1/sodar19_pwm_top_0_1_stub.v}
// Design      : sodar19_pwm_top_0_1
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7z020clg484-1
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* x_core_info = "pwm_top,Vivado 2019.1" *)
module sodar19_pwm_top_0_1(clk, duty_cycle, period, phase, pwm_out)
/* synthesis syn_black_box black_box_pad_pin="clk,duty_cycle[31:0],period[31:0],phase[31:0],pwm_out[18:0]" */;
  input clk;
  input [31:0]duty_cycle;
  input [31:0]period;
  input [31:0]phase;
  output [18:0]pwm_out;
endmodule
