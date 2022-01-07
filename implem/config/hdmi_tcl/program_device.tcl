open_hw
connect_hw_server
open_hw_target
#current_hw_device [get_hw_devices xc7z010_1]
current_hw_device [get_hw_devices xc7z020_1]
#refresh_hw_device -update_hw_probes false [lindex [get_hw_devices xc7z010_1] 0]
refresh_hw_device -update_hw_probes false [lindex [get_hw_devices xc7z020_1] 0]
#set_property PROBES.FILE {} [get_hw_devices xc7z010_1]
set_property PROBES.FILE {} [get_hw_devices xc7z020_1]
#set_property FULL_PROBES.FILE {} [get_hw_devices xc7z010_1]
set_property FULL_PROBES.FILE {} [get_hw_devices xc7z020_1]
#set_property PROGRAM.FILE {./output/output.bit} [get_hw_devices xc7z010_1]
set_property PROGRAM.FILE {./output/output.bit} [get_hw_devices xc7z020_1]

#program_hw_devices [lindex [get_hw_devices xc7z010_1] 0]
program_hw_devices [lindex [get_hw_devices xc7z020_1] 0]
#refresh_hw_device [lindex [get_hw_devices xc7z010_1] 0]
refresh_hw_device [lindex [get_hw_devices xc7z020_1] 0]
