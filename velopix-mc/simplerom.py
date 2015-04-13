#!/usr/bin/env python
import sys

if len(sys.argv) < 2 :
    print "Error: give an input file (probably in the bychip directory)"
    print "    Example usage: "+sys.argv[0]+" bychip/desync444.txt"
    sys.exit(1)

binfile = file(sys.argv[1])
vhdfile = file("simplerom.vhd", "w")
numlines=0 

for l in binfile :
    numlines += 1
##`wc -l $1 | awk '{ print $1 }'`

vhdfile.write("""
---------------------------------------------------------------------------
-- Author(s)    : Karol Hennessy <karol.hennessy@cern.ch>
-- Affliliation : University of Liverpool
-- 
-- Creation Date : 21/03/2014
-- File          : simplerom.vhd
--
-- Abstract : A simple ROM implementation of VeloPix MC input data.
--            generated using simplerom.py using 
--            source data file = """+sys.argv[1]+"""
--
---------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;


entity simplerom is
        generic ( ADDRMAX : integer := """+str(numlines-1)+""" );
        port(
                clk    : in std_logic;
                addr   : in integer range 0 to ADDRMAX; 
                gwtframe : out std_logic_vector(127 downto 0)
        );
end simplerom;


architecture Structural of simplerom is

        type mem is array ( 0 to ADDRMAX ) of std_logic_vector(127 downto 0);
        constant rom : mem := (
""")
i=0
binfile.seek(0)
for l in binfile:
    if i == numlines-1 :
        vhdfile.write(str(i)+' => "'+l[0:128]+'"\n')
    else :
        vhdfile.write(str(i)+' => "'+l[0:128]+'",\n')
    i+=1

vhdfile.write( """
        );

begin
        process (addr)
        begin
                if (addr < ADDRMAX+1) then
                        gwtframe <= rom(addr);
                else 
                        gwtframe <= (others => '0');
                end if;
        end process;

end Structural;

"""
)

print "Output file: simplerom.vhd to replace the one in firmware source"
