--
--	Package File Template
--
--	Purpose: This package defines supplemental types, subtypes, 
--		 constants, and functions 
--
--   To use any of the example code shown below, uncomment the lines and modify as necessary
--

library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.ALL;

package types is

-- Declare types
subtype voltage is std_logic_vector(3 downto 0);
type voltage_vector is array (natural range <>) of voltage;

-- Declare constants

constant zero : STD_LOGIC_VECTOR(3 downto 0) := std_logic_vector(to_signed(0,4));
constant Vth : STD_LOGIC_VECTOR(3 downto 0) := std_logic_vector(to_signed(3,4));
constant Vth_neg : STD_LOGIC_VECTOR(3 downto 0) := std_logic_vector(to_signed(-3,4));
constant Vr: STD_LOGIC_VECTOR(3 downto 0) := std_logic_vector(to_signed(2,4));
constant Vr_neg : STD_LOGIC_VECTOR(3 downto 0) := std_logic_vector(to_signed(-2,4));
constant Vw : STD_LOGIC_VECTOR(3 downto 0) := std_logic_vector(to_signed(4,4));
constant Vw_neg : STD_LOGIC_VECTOR(3 downto 0) := std_logic_vector(to_signed(-4,4));
constant Vn : STD_LOGIC_VECTOR(3 downto 0) := std_logic_vector(to_signed(7,4));
constant Vn_neg : STD_LOGIC_VECTOR(3 downto 0) := std_logic_vector(to_signed(-7,4));

-- Declare functions

function verify_voltage (v: voltage) return boolean;

end types;

package body types is
  
function verify_voltage (v: voltage) return boolean is
begin
   case v is
     when "0000" => return true;
     when "0001" => return true;
     when "0010" => return true;
     when "0011" => return true;
     when "0100" => return true;
     when "0101" => return true;
     when "0110" => return true;
     when "0111" => return true;
	  when "1000" => return true;
     when "1001" => return true;
     when "1010" => return true;
     when "1011" => return true;
     when "1100" => return true;
     when "1101" => return true;
     when "1110" => return true;
     when "1111" => return true;
     when others => return false;
   end case;
end verify_voltage;
  
end types;


