library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity pwm_top is
    port(
        clk        : in  std_logic;
        duty_cycle : in  std_logic_vector(31 downto 0);
        period     : in  std_logic_vector(31 downto 0);
        phase      : in  std_logic_vector(31 downto 0);
        pwm_out    : out std_logic_vector(18 downto 0)
    );
end pwm_top;

architecture Behavioral of pwm_top is

    component pwm_generator is
        port(
            clk         : in std_logic;
            duty_cycle  : in std_logic_vector(31 downto 0);
            period      : in std_logic_vector(31 downto 0);
            phase_shift : in std_logic_vector(31 downto 0);
            pwm_out     : out std_logic
        );
    end component;

    
    function compute_phase(channel : integer; base_phase : std_logic_vector(31 downto 0)) 
      return std_logic_vector is
        variable phase_val : integer := to_integer(signed(base_phase));
        variable effective : integer := channel * phase_val;
    begin
        return std_logic_vector(to_signed(effective, 32));  --  signed value in two's complement
    end function;

begin

    gen_pwm: for i in 0 to 18 generate
        constant effective_phase : std_logic_vector(31 downto 0) :=
            compute_phase(i, phase);
    begin
        pwm_inst: pwm_generator
            port map(
                clk         => clk,
                duty_cycle  => duty_cycle,
                period      => period,
                phase_shift => effective_phase,
                pwm_out     => pwm_out(i)
            );
    end generate gen_pwm;

end Behavioral;
