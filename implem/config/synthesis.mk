### Parameters
TOP    ?= PROC_complet
#DEVICE ?= xc7z010clg400-1
DEVICE ?= xc7z020clg400-1

TOP_OK := PROC_simple PROC_complet
ifeq ($(filter $(TOP),$(TOP_OK)),)
  $(error "Design toplevel inconnu: $(TOP)\n")
endif

# Xilinx tools
ifeq ($(origin XILINX_VIVADO), undefined)
  $(error "Outils Xilinx introuvables, il faut sourcer le fichier /<Chemin vers le répertoire d'installation des outils Xilinx Vivado>/Vivado/2019/settings64.sh")
endif
XILINX_PREFIX ?= $(XILINX_VIVADO)/bin/

# Log level
VERB ?= 0
LOG_FILE ?= $(abspath log)
quiet-command = $(if $(filter 1, $(VERB)),$(1), $(if $(2),@echo $(2) && ($(1)) >> $(LOG_FILE), @$(1)))

### Main rule
all: bitstream

### Files & Directories
include config/directory.mk
BIT_EMPTY_FILE := $(BIT_DIR)/$(TOP).bit
BIT_FILE       := $(BIT_DIR)/$(TOP).patched.bit
VHD_FILE       := $(wildcard $(VHD_DIR)/*.vhd)

SCRIPT_BIT := $(abspath config/synthesis.vivado.tcl)
SCRIPT_PRO := $(abspath config/programFPGA.vivado.tcl)

TOCLEAN:= $(filter xsim% %.jou %.log,$(wildcard *)) log
TOPOLISH:= $(SYN_DIR) $(BIT_DIR)

# Rules
.PHONY: bitstream patch run

bitstream: $(BIT_EMPTY_FILE)

patch: $(BIT_FILE)

run: patch
	$(call quiet-command, cd $(SYN_DIR) ; $(XILINX_PREFIX)vivado -mode batch -source $(SCRIPT_PRO) -nolog -nojournal -tclargs $(TOP) $(DEVICE), "Downloading bitstream..")

$(BIT_EMPTY_FILE): $(VHD_FILE) $(SCRIPT_BIT) |$(SYN_DIR) $(BIT_DIR)
	$(call quiet-command, cd $(SYN_DIR) ; $(XILINX_PREFIX)vivado -mode batch -source $(SCRIPT_BIT) -nolog -nojournal -tclargs $(DEVICE) $(TOP), "Building bitstream..")
	@ echo "Fréquence d'horloge maximale :"
	@ slack=$$(cat .CEPcache/synth/$(TOP)_timing.rpt | awk '/Slack/{print $$4}' | sed -e 's/ns//');r=$$(cat .CEPcache/synth/$(TOP)_timing.rpt | awk '/Requirement/{print $$2}' | sed -e 's/ns//') ; t=$$(echo "scale=1;1000/($$r - $$slack)" | bc) ; echo "$$t MHz"

$(BIT_FILE): $(BIT_EMPTY_FILE) $(MEM_DIR)/prog.mem $(SYN_DIR)/map.mmi |$(BIT_DIR)
	$(call quiet-command, $(XILINX_PREFIX)updatemem -debug -force --meminfo $(SYN_DIR)/map.mmi --data $(MEM_DIR)/prog.mem --proc "C_RAM_PROG/XPM_RAM/xpm_memory_base_inst" --bit $< --out $@, "Patching bitstream..")

clean:
	echo "Removing $(TOCLEAN)"
	rm -rf $(TOCLEAN)

realclean: clean
	echo "Removing $(TOPOLISH)"
	rm -rf $(TOPOLISH)
