VIVADO := $(XILINX_VIVADO)/bin/vivado
$(XCLBIN)/$(KERNEL).$(TARGET).$(DSA).xo: src/kernel.xml scripts/package_kernel.tcl scripts/gen_xo.tcl src/*.v src/*.sv src/*.xdc ../common/*.sv
	mkdir -p $(XCLBIN)
	$(VIVADO) -mode batch -source scripts/gen_xo.tcl -tclargs $(XCLBIN)/$(KERNEL).$(TARGET).$(DSA).xo $(KERNEL) $(TARGET) $(DSA)
