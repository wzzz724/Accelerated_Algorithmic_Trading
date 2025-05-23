open_project rxAppIf_proj

set_top rx_app_if

add_files rx_app_if.cpp
add_files rx_app_if.hpp

open_solution "solution1"
set_part $::env(XPART)
create_clock -period $::env(KERNEL_PERIOD) -name default

#csim_design -setup -clean
csynth_design
export_design -evaluate verilog
exit
