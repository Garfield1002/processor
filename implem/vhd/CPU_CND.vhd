library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.PKG.all;

entity CPU_CND is
    generic (
        mutant      : integer := 0
    );
    port (
        ina         : in w32;
        inb         : in w32;
        f           : in  std_logic;
        r           : in  std_logic;
        e           : in  std_logic;
        d           : in  std_logic;
        s           : out std_logic;
        j           : out std_logic
    );
end entity;

architecture RTL of CPU_CND is

begin
end architecture;
