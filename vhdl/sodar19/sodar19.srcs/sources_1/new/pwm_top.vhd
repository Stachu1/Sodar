library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity pwm_top is
    Port (
        clk        : in std_logic;
        duty_cycle : in std_logic_vector(31 downto 0);
        period     : in std_logic_vector(31 downto 0);
        phase      : in std_logic_vector(31 downto 0); -- relative phase increment
        pwm_out    : out std_logic_vector(18 downto 0)  -- 19 PWM outputs
    );
end pwm_top;

architecture Behavioral of pwm_top is

    component pwm_generator is
        Port (
            clk         : in std_logic;
            duty_cycle  : in std_logic_vector(31 downto 0);
            period      : in std_logic_vector(31 downto 0);
            phase_shift : in std_logic_vector(31 downto 0);
            pwm_out     : out std_logic
        );
    end component;

    -- We'll use a generate loop to instantiate 4 PWM generators.
    -- For each instance, multiply the base phase by the channel index.
    -- You may want to adjust the multiplication size presuming the phase register holds
    -- a small value (for example, 3 or 5). For safety, conversion is done.

    function compute_phase(channel : integer; base_phase : std_logic_vector(31 downto 0)) return std_logic_vector is
        variable phase_val : integer := to_integer(unsigned(base_phase));
        variable effective : integer := channel * phase_val;
    begin
        return std_logic_vector(to_unsigned(effective, 32));
    end function;

begin

    gen_pwm: for i in 0 to 18 generate
        signal effective_phase : std_logic_vector(31 downto 0);
    begin
        effective_phase <= compute_phase(i, phase);

        pwm_inst: pwm_generator
            port map (
                clk         => clk,
                duty_cycle  => duty_cycle,
                period      => period,
                phase_shift => effective_phase,
                pwm_out     => pwm_out(i)
            );
    end generate gen_pwm;

end Behavioral;
