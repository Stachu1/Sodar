library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity pwm_generator is
  Port (
    clk         : in  std_logic;
    duty_cycle  : in  std_logic_vector(31 downto 0); 
    period      : in  std_logic_vector(31 downto 0); 
    phase_shift : in  std_logic_vector(31 downto 0); 
    pwm_out     : out std_logic
  );
end pwm_generator;

architecture Behavioral of pwm_generator is
  signal counter         : unsigned(31 downto 0) := (others => '0');
  signal effective_value : integer := 0;
begin

-- counter
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

 --phase computation with wrappping
  process(counter, phase_shift, period)
    variable total       : integer;
    variable period_int  : integer;
    variable wrapped     : integer;
  begin
    period_int := to_integer(unsigned(period));
    total := to_integer(counter) + to_integer(signed(phase_shift));

    if period_int = 0 then
      wrapped := 0;
    elsif total < 0 then
      -- Negative phase, add multiples of period until the result is between 0 and period
      if total >= -period_int then
        wrapped := total + period_int;
      elsif total >= -2 * period_int then
        wrapped := total + 2 * period_int;
      elsif total >= -3 * period_int then
        wrapped := total + 3 * period_int;
      elsif total >= -4 * period_int then
        wrapped := total + 4 * period_int;
      elsif total >= -5 * period_int then
        wrapped := total + 5 * period_int;
      elsif total >= -6 * period_int then
        wrapped := total + 6 * period_int;
      else
        wrapped := total + 7 * period_int;
      end if;
    else
     -- positive phase, subtracts multiples of period until the result is between 0 and period
      if total < period_int then
        wrapped := total;
      elsif total < 2 * period_int then
        wrapped := total - period_int;
      elsif total < 3 * period_int then
        wrapped := total - 2 * period_int;
      elsif total < 4 * period_int then
        wrapped := total - 3 * period_int;
      elsif total < 5 * period_int then
        wrapped := total - 4 * period_int;
      elsif total < 6 * period_int then
        wrapped := total - 5 * period_int;
      elsif total < 7 * period_int then
        wrapped := total - 6 * period_int;
      else
        wrapped := total - 7 * period_int;  -- fallback if total is even higher
      end if;
    end if;

    effective_value <= wrapped;
  end process;

  -- Generate PWM 
  process(effective_value, duty_cycle)
  begin
    if effective_value < to_integer(unsigned(duty_cycle)) then
      pwm_out <= '1';
    else
      pwm_out <= '0';
    end if;
  end process;

end Behavioral;
