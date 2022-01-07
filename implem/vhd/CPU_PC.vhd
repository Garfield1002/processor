library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.PKG.all;


entity CPU_PC is
    generic(
        mutant: integer := 0
    );
    Port (
        -- Clock/Reset
        clk    : in  std_logic ;
        rst    : in  std_logic ;

        -- Interface PC to PO
        cmd    : out PO_cmd ;
        status : in  PO_status
    );
end entity;

architecture RTL of CPU_PC is
    type State_type is (
        S_Error,
        S_Init,
        S_Pre_Fetch,
        S_Fetch,
        S_Decode,
        S_LUI
    );

    signal state_d, state_q : State_type;
    signal cmd_cs : PO_cs_cmd;


    function arith_sel (IR : unsigned( 31 downto 0 ))
        return ALU_op_type is
        variable res : ALU_op_type;
    begin
        if IR(30) = '0' or IR(5) = '0' then
            res := ALU_plus;
        else
            res := ALU_minus;
        end if;
        return res;
    end arith_sel;

    function logical_sel (IR : unsigned( 31 downto 0 ))
        return LOGICAL_op_type is
        variable res : LOGICAL_op_type;
    begin
        if IR(12) = '1' then
            res := LOGICAL_and;
        else
            if IR(13) = '1' then
                res := LOGICAL_or;
            else
                res := LOGICAL_xor;
            end if;
        end if;
        return res;
    end logical_sel;

    function shifter_sel (IR : unsigned( 31 downto 0 ))
        return SHIFTER_op_type is
        variable res : SHIFTER_op_type;
    begin
        res := SHIFT_ll;
        if IR(14) = '1' then
            if IR(30) = '1' then
                res := SHIFT_ra;
            else
                res := SHIFT_rl;
            end if;
        end if;
        return res;
    end shifter_sel;

begin

    cmd.cs <= cmd_cs;

    FSM_synchrone : process(clk)
    begin
        if clk'event and clk='1' then
            if rst='1' then
                state_q <= S_Init;
            else
                state_q <= state_d;
            end if;
        end if;
    end process FSM_synchrone;

    FSM_comb : process (state_q, status)
    begin

        -- Valeurs par défaut de cmd à définir selon les préférences de chacun
        cmd.rst               <= '1';   -- Reset ?
        cmd.ALU_op            <= ALU_plus;      -- Sélection de l'opération arithmétique effectuée par l'ALU
        cmd.LOGICAL_op        <= LOGICAL_and;   -- Sélection de l'opération logique effectuée par l'ALU
        cmd.ALU_Y_sel         <= ALU_Y_immI;    -- Sélection de l'opérande Y sur l'ALU

        cmd.SHIFTER_op        <= SHIFT_rl;          -- Sélection de l'opération shift effectuée par l'ALU
        cmd.SHIFTER_Y_sel     <= SHIFTER_Y_ir_sh;   -- Sélection de l'opérande Y sur l'ALU

        cmd.RF_we             <= '1';           -- Valide l'écriture dans RF
        cmd.RF_SIZE_sel       <= RF_SIZE_word;  -- Self descrptive
        cmd.RF_SIGN_enable    <= '0';
        cmd.DATA_sel          <= DATA_from_alu; -- Sélection de la provenance de la donnée à écrire dans le banc de registres

        cmd.PC_we             <= '1';
        cmd.PC_sel            <= PC_rstvec;

        cmd.PC_X_sel          <= PC_X_cst_x00;  -- Sélection de l'opérande X sur l'additionneur vers le banc de registre
        cmd.PC_Y_sel          <= PC_Y_cst_x04;  -- Sélection de l'opérande Y sur l'additionneur vers le banc de registre

        cmd.TO_PC_Y_sel       <= TO_PC_Y_cst_x04;   -- Sélection de l'opérande Y sur l'additionneur du PC

        cmd.AD_we             <= '1';           -- Valide l'écriture dans AD ?
        cmd.AD_Y_sel          <= AD_Y_immI;    -- Sélection de l'opérande Y sur l'additionneur d'AD

        cmd.IR_we             <= '1';   -- Valide l'écriture dans IR

        cmd.ADDR_sel          <= ADDR_from_pc;  -- Sélection de l'adresse vers la mémoire
        cmd.mem_we            <= '1';           -- Valide une écriture dans la mémoire
        cmd.mem_ce            <= '1';           -- Valide une transaction dans la mémoire (r/w)

        -- IGNORE
            cmd_cs.CSR_we         <= UNDEFINED;      -- Valide l'écriture sur l'un des registres de contrôle/statut

            cmd_cs.TO_CSR_sel     <= UNDEFINED;   -- Sélection de la provenance de la donnée à écrire dans l'un des registres de contrôle/statut
            cmd_cs.CSR_sel        <= UNDEFINED;      -- Sélection du registre de contrôle/statut à envoyer au banc de registre
            cmd_cs.MEPC_sel       <= UNDEFINED;      -- Sélection de la provenance de la donnée à écrire dans le registre mepc

            cmd_cs.MSTATUS_mie_set   <= 'U';    -- WWTF
            cmd_cs.MSTATUS_mie_reset <= 'U';    -- WWWTF

            cmd_cs.CSR_WRITE_mode    <= UNDEFINED; -- AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
        -- Stop Ignoring

        state_d <= state_q;

        case state_q is
            when S_Error =>
                state_d <= S_Error;

            when S_Init =>
                -- PC <- RESET_VECTOR
                cmd.PC_we <= '1';
                cmd.PC_sel <= PC_rstvec;
                state_d <= S_Pre_Fetch;

            when S_Pre_Fetch =>
                -- mem[PC]
                cmd.mem_ce <= '1';
                state_d <= S_Fetch;

            when S_Fetch =>
                -- IR <- mem_datain
                cmd.IR_we <= '1';
                state_d <= S_Decode;

            -- when S_Decode =>
            --     -- PC <- PC + 4
            --     cmd.TO_PC_Y_sel <= TO_PC_Y_cst_x04;
            --     cmd.PC_sel <= PC_from_pc;
            --     cmd.PC_we <= '1';

            --     state_d <= S_Init;

            when S_Decode =>
                -- On peut aussi utiliser un case, ...
                -- et ne pas le faire juste pour les branchements et auipc
                if status.IR(6 downto 0) = "0110111" then
                    cmd.TO_PC_Y_sel <= TO_PC_Y_cst_x04;
                    cmd.PC_sel <= PC_from_pc;
                    cmd.IR_we <= '0';
                    cmd.PC_we <= '1 ';
                    state_d <= S_LUI;
                    else
                    state_d <= S_Error; -- Pour détecter les ratés du décodage
                end if;


                -- Décodage effectif des instructions,
                -- à compléter par vos soins

---------- Instructions avec immediat de type U ----------
            when S_LUI =>
                -- rd <- ImmU + 0
                cmd.PC_X_sel <= PC_X_cst_x00;
                cmd.PC_Y_sel <= PC_Y_immU;
                cmd.RF_we <= '1';
                cmd.DATA_sel <= DATA_from_pc;
                -- lecture mem[PC]
                cmd.ADDR_sel <= ADDR_from_pc;
                cmd.mem_ce <= '1';
                cmd.mem_we <= '0';
                -- next state
                state_d <= S_Fetch;

---------- Instructions arithmétiques et logiques ----------

---------- Instructions de saut ----------

---------- Instructions de chargement à partir de la mémoire ----------

---------- Instructions de sauvegarde en mémoire ----------

---------- Instructions d'accès aux CSR ----------

            when others => null;
        end case;

    end process FSM_comb;

end architecture;
