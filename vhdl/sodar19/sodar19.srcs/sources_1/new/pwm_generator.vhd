library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity pwm_generator is
  Port (
    clk         : in  std_logic;
    duty_cycle  : in  std_logic_vector(31 downto 0); -- PWM high time
    period      : in  std_logic_vector(31 downto 0); -- Total period
    phase_shift : in  std_logic_vector(31 downto 0); -- Phase offset in counter units
    pwm_out     : out std_logic
  );
end pwm_generator;

architecture Behavioral of pwm_generator is
  signal counter         : unsigned(31 downto 0) := (others => '0');
  signal effective_value : integer := 0;
begin

  -- Counter process: increments at each clock edge and resets when reaching the period.
  process(clk)
  begin
    if rising_edge(clk) then
      if counter >= (unsigned(period) - 1) then
        counter <= (others => '0');
      else
        counter <= counter + 1;
      end if;
    end if;
  end process;
  
  -- Compute effective counter value = (counter + phase_shift) with wrap-around,
  -- i.e. subtract period if the sum exceeds (or equals) period.
  process(counter, phase_shift, period)
    variable sum_val    : integer;
    variable period_int : integer;
  begin
    period_int := to_integer(unsigned(period));
    sum_val := to_integer(counter) + to_integer(unsigned(phase_shift));
    if sum_val >= period_int then
      effective_value <= sum_val - period_int;
    else
      effective_value <= sum_val;
    end if;
  end process;

  -- Generate PWM: if the effective value is less than the duty threshold, output high.
  pwm_out <= '1' when (effective_value < to_integer(unsigned(duty_cycle))) else '0';

end Behavioral;
