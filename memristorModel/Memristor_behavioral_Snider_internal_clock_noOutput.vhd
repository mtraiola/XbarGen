----------------------------------------------------------------------------------
--    Copyright (C) 2016 Marcello Traiola
--
--    This program is free software: you can redistribute it and/or modify
--    it under the terms of the GNU Affero General Public License as
--    published by the Free Software Foundation, either version 3 of the
--    License, or (at your option) any later version.
--
--    This program is distributed in the hope that it will be useful,
--    but WITHOUT ANY WARRANTY; without even the implied warranty of
--    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--    GNU Affero General Public License for more details.
--
--    You should have received a copy of the GNU Affero General Public License
--    along with this program.  If not, see <http://www.gnu.org/licenses/>.
----------------------------------------------------------------------------------

----------------------------------------------------------------------------------
-- Engineer: 		Marcello Traiola
-- 
-- Create Date:    14:33:03 03/09/2016 
-- Design Name: 
-- Module Name:    Memristor_behavioral_Snider - Behavioral 

-- Revision 2.0
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

library memristor_lib;
use memristor_lib.types.all;
use memristor_lib.variables.all;

entity Memristor_behavioral_Snider_internal_clock_noOutput is
    Port ( Vpos : inout  voltage;
           Vneg : inout  voltage);
end Memristor_behavioral_Snider_internal_clock_noOutput ;

architecture Behavioral of Memristor_behavioral_Snider_internal_clock_noOutput is

constant VthPos : voltage := Vth;
constant VthNeg : voltage := Vth_neg;

signal internal_state : STD_LOGIC :='1';
signal V: voltage := (others=>'0');
signal Vneg_temp_in, Vpos_temp_in: voltage := zero;
signal Vneg_temp_out, Vpos_temp_out: voltage := (others=>'Z');
signal out_en : std_logic := '0';

type fsm_state is (zero, one);
signal state : fsm_state := one;
signal next_state : fsm_state := one;

signal clk : std_logic := '0';

constant clk_period : time := 1 ns;


begin

-- Clock process definitions
clk_process : process (clk)
begin
	clk <= not(clk) after clk_period/2; --only behavioral simulation
end process;

Vneg <= Vneg_temp_out;
Vpos <= Vpos_temp_out;

V_out: process(out_en,Vneg_temp_out,Vpos_temp_out,Vpos,Vneg)
begin
	if out_en = '1' then
		
		if Vneg = "ZZZZ" then
			Vneg_temp_out <= Vpos_temp_in;
		end if;
		
		if Vpos = "ZZZZ" then
			Vpos_temp_out <= Vneg_temp_in;
		end if;
	
	else 
		Vpos_temp_out <= (others=>'Z') after 100 ps;
		Vneg_temp_out <= (others=>'Z') after 100 ps;
	end if;
end process;

V_in: process(Vneg,Vpos)
begin
	if verify_voltage(Vneg) then
		Vneg_temp_in <= Vneg;
	end if;
	
	if verify_voltage(Vpos) then
		Vpos_temp_in <= Vpos;
	end if;
end process;

change_state: process (clk)
begin
	if(clk'event and clk='1') then
		if(state /= next_state) then
			if(next_state = zero) then
				switchDownCnt:=switchDownCnt+1;
			else
				switchUpCnt:=switchUpCnt+1;
			end if;
		end if;
		state <= next_state;   --state change.
	end if;
end process;

V <= std_logic_vector(to_signed((to_integer(signed(Vpos_temp_in)) - to_integer(signed(Vneg_temp_in))),V'length));

execute: process (state,V,Vpos_temp_in,Vpos_temp_out)

begin

	case state is
  
	when one =>
		
		internal_state <= '1';
		out_en <= '0';
		
		if( to_integer(signed(V)) > to_integer(signed(Vth)))then
			next_state <= zero;
		else
			next_state <= one;
		end if;
		
   when zero =>
		
		internal_state <= '0';

		if( to_integer(signed(V)) < to_integer(signed(Vth_neg)) ) then
			next_state <= one;
		elsif(V = "0000") then
			next_state <= zero;
			out_en <= '0';
		else
			next_state <= zero;
			out_en <= '1';
		end if;

	end case;
	
end process;

end Behavioral;