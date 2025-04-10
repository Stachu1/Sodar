-- Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2019.1 (win64) Build 2552052 Fri May 24 14:49:42 MDT 2019
-- Date        : Thu Apr 10 15:46:37 2025
-- Host        : Bruno-Komp-MSI running 64-bit major release  (build 9200)
-- Command     : write_vhdl -force -mode synth_stub {C:/Users/plywa/Digital designe
--               vivado/sodar19/sodar19.srcs/sources_1/bd/sodar19/ip/sodar19_pwm_top_0_1/sodar19_pwm_top_0_1_stub.vhdl}
-- Design      : sodar19_pwm_top_0_1
-- Purpose     : Stub declaration of top-level module interface
-- Device      : xc7z020clg484-1
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity sodar19_pwm_top_0_1 is
  Port ( 
    clk : in STD_LOGIC;
    duty_cycle : in STD_LOGIC_VECTOR ( 31 downto 0 );
    period : in STD_LOGIC_VECTOR ( 31 downto 0 );
    phase : in STD_LOGIC_VECTOR ( 31 downto 0 );
    pwm_out : out STD_LOGIC_VECTOR ( 18 downto 0 )
  );

end sodar19_pwm_top_0_1;

architecture stub of sodar19_pwm_top_0_1 is
attribute syn_black_box : boolean;
attribute black_box_pad_pin : string;
attribute syn_black_box of stub : architecture is true;
attribute black_box_pad_pin of stub : architecture is "clk,duty_cycle[31:0],period[31:0],phase[31:0],pwm_out[18:0]";
attribute x_core_info : string;
attribute x_core_info of stub : architecture is "pwm_top,Vivado 2019.1";
begin
end;
