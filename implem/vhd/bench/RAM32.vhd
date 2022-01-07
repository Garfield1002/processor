library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_textio.all;

use std.textio.all;

use work.PKG.all;
use work.txt_util.all;


entity RAM32 is
    generic (
        -- Memory configuration
        MEMORY_SIZE : positive := 4096 ;

        -- Memory initialization
        FILE_NAME   : string   := "none"
    );
    port (
        -- Clock
        clk  : in  std_logic ;
        rst  : in  std_logic ;

        -- Memory slave interface
        addr : in  waddr ;
        size : in  RF_SIZE_select ;
        do   : out w32 ;
        di   : in  w32 ;
        ce   : in  std_logic ;
        we   : in  std_logic
    );
end entity;


architecture behavioral of RAM32 is
    function log2ceil (x:positive) return natural is
        variable i : natural := x-1 ;
        variable n : natural := 0 ;
    begin
        while i > 0 loop
            n := n + 1; i := i / 2;
        end loop;
        return n;
    end function;

    -- Constants
    constant ADDRESS_BASE : waddr   := waddr_zero ;
    constant N_BIT        : natural := log2ceil( MEMORY_SIZE ) ;
    constant ADDRESS_MASK : waddr   := to_unsigned( 2**N_BIT - 1, waddr'length ) ;

    -- Type
    type     memory_type is array (natural range 0 to MEMORY_SIZE-1) of w32;

    -- Procedure to initilize a memory container from a ".mem" file
    impure function memory_initialize_from_file (
                 file_name    : in string ;
        constant ADDRESS_BASE : in waddr ;
        constant MEMORY_SIZE  : in positive
    ) return memory_type is
        file     data_file   : text; --open read_mode is file_name ;
        variable line_input  : line;
        variable data_line   : std_logic_vector(31 downto 0);
        variable write_index : integer := 0;
        variable c           : character;
        variable memory      : memory_type := (others => w32_zero);
    begin
        if file_name="none" then
            return memory;
        end if;
        file_open(data_file, file_name, read_mode);
        while not endfile(data_file) loop
            readline(data_file, line_input);
            if line_input(1)='@' then -- read an address
                read(line_input, c);
                hread(line_input, data_line);
                data_line := data_line xor std_logic_vector(ADDRESS_BASE); -- Remove base address
                assert data_line(1 downto 0) = "00" report "Provide a 4 bytes aligned .mem file";
                write_index := to_integer( unsigned( data_line( 31 downto 2 ) ) );
                report "@ " & hstr(data_line) & " = " & integer'image(write_index); 
            elsif 0<=write_index and write_index < MEMORY_SIZE/4 then
                hread(line_input, data_line);
                memory(write_index) := w32(data_line);
                report "memory[" & integer'image(write_index) & "] = " & hstr(data_line);
                write_index := write_index + 1;
            end if;
        end loop;
        return memory;
    end function;

    -- Internal signals
    signal internal_memory  : memory_type := memory_initialize_from_file(FILE_NAME, ADDRESS_BASE, MEMORY_SIZE);
    signal internal_address : waddr;
    signal internal_index   : integer;
begin

    process (addr, internal_address)
    begin
        -- Remove base address and 
        internal_address <= addr and ADDRESS_MASK ;
        internal_index   <= to_integer(internal_address) / 4 ;
    end process;

    process(clk, internal_memory, internal_index, di)
    begin
        if rising_edge(clk) then
            -- Memory behavior (Read First)
            if ce = '1'  then
                do <= internal_memory(internal_index); -- Read
                if we='1' then
                    internal_memory(internal_index) <= di; -- Write
                end if;
            end if;
        end if;

    end process;

end architecture;
