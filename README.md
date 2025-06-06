Accelerated Algorithmic Trading
===============================

This is a package release for the Accelerated Algorithmic Trading (AAT) design.

Please consult the user guide for build instructions and a quick start overview:
https://www.xilinx.com/member/fintech.html#documents

A version of the user guide is included in the doc folder of this package for
convenience. To ensure you are viewing the latest version however, it is
recommended to download from the URL provided.

1. Set env variables first:
```bash
# U55C for example
DEVICE=/opt/xilinx/platforms/xilinx_u55c_gen3x16_xdma_3_202210_1/xilinx_u55c_gen3x16_xdma_3_202210_1.xpfm
XILINX_PLATFORM=xilinx_u55c_gen3x16_xdma_3_202210_1
XPART=xcu55c-fsvh2892-2L-e
PLATFORM_REPO_PATHS='/opt/xilinx/platforms'
```

2. Set platform information for IP kernel
```tcl
# Accelerated_Algorithmic_Trading\hw\ethernet_krnl\ethernet_krnl_axis_x4\scripts\package_kernel.tcl
...
# line 58
if {[string compare -nocase $board "u200"] == 0} {
set projPart "xcu200-fsgd2104-2-e"
set boardPart "xilinx.com:au200:part0:1.2"
# U250_START
} elseif {[string compare -nocase $board "u250"] == 0} {
set projPart "xcu250-figd2104-2L-e"
set boardPart "xilinx.com:au250:part0:1.3"
# U50_START
} elseif {[string compare -nocase $board "u50"] == 0} {
set projPart "xcu50-fsvh2104-2L-e"
set boardPart "xilinx.com:au50:part0:1.1"
# U50_END
} elseif {[string compare -nocase $board "u280"] == 0} {
set projPart "xcu280-fsvh2892-2L-e"
set boardPart "xilinx.com:au280:part0:1.2"
# U55C_START
} elseif {[string compare -nocase $board "u55c"] == 0} {
set projPart "xcu55c-fsvh2892-2L-e"
set boardPart "xilinx.com:au55c:1.0"
# U55C_END
# TODO: Set platform info according to your card
} else {
	puts "Unknown Board"
	exit
}
...
#line 95
if {[string compare -nocase $board "u200"] == 0} {
  set_property -dict [list CONFIG.GT_REF_CLK_FREQ {161.1328125} CONFIG.GT_GROUP_SELECT {Quad_X1Y11} CONFIG.LANE1_GT_LOC {X1Y44} CONFIG.LANE2_GT_LOC {X1Y45} CONFIG.LANE3_GT_LOC {X1Y46} CONFIG.LANE4_GT_LOC {X1Y47}] [get_ips xxv_ethernet_x4_0]
} elseif {[string compare -nocase $board "u250"] == 0} {
  set_property -dict [list CONFIG.GT_REF_CLK_FREQ {161.1328125} CONFIG.GT_GROUP_SELECT {Quad_X1Y10} CONFIG.LANE1_GT_LOC {X1Y40} CONFIG.LANE2_GT_LOC {X1Y41} CONFIG.LANE3_GT_LOC {X1Y42} CONFIG.LANE4_GT_LOC {X1Y43}] [get_ips xxv_ethernet_x4_0]
} elseif {[string compare -nocase $board "u280"] == 0} {
  set_property -dict [list CONFIG.GT_REF_CLK_FREQ {156.25} CONFIG.GT_GROUP_SELECT {Quad_X0Y10} CONFIG.LANE1_GT_LOC {X0Y40} CONFIG.LANE2_GT_LOC {X0Y41} CONFIG.GT_DRP_CLK {50}] [get_ips xxv_ethernet_x4_0]
# U50_START
} elseif {[string compare -nocase $board "u50"] == 0} {
  set_property -dict [list CONFIG.GT_REF_CLK_FREQ {161.1328125}  CONFIG.GT_GROUP_SELECT {Quad_X0Y7} CONFIG.LANE1_GT_LOC {X0Y28} CONFIG.LANE2_GT_LOC {X0Y29} CONFIG.LANE3_GT_LOC {X0Y30} CONFIG.LANE4_GT_LOC {X0Y31}] [get_ips xxv_ethernet_x4_0]
# U50_END
# U55C_START
} elseif {[string compare -nocase $board "u55c"] == 0} {
  set_property -dict [list CONFIG.GT_REF_CLK_FREQ {161.1328125}  CONFIG.GT_GROUP_SELECT {Quad_X0Y7} CONFIG.LANE1_GT_LOC {X0Y28} CONFIG.LANE2_GT_LOC {X0Y29} CONFIG.LANE3_GT_LOC {X0Y30} CONFIG.LANE4_GT_LOC {X0Y31}] [get_ips xxv_ethernet_x4_0]
# U55C_END
# TODO: Set platform info according to your card
} else {
    puts "Unknown board $board"
    exit
}
...
```
3. Customize signal connection. if your card is not U50, U250 or U55C, creating a new .ini file
```makefile
# Accelerated_Algorithmic_Trading\build\Makefile
# line 95
# shell specific connectivity
ifeq ($(TARGET),$(filter $(TARGET),hw_emu))
ifeq ($(DM_MODE), SB)
VPPLINKFLAGS := --config aat.hw_emu.sb.ini
else
VPPLINKFLAGS := --config aat.hw_emu.ini
endif
else
ifneq (,$(shell echo $(XILINX_PLATFORM) | awk '/_u50_gen.*_xdma_/'))
ifeq ($(DM_MODE), DMA)
VPPLINKFLAGS := --config aat.u50.ini
else
$(error Unsupported DM_MODE=$(DM_MODE) for XILINX_PLATFORM=$(XILINX_PLATFORM))
endif
else ifneq (,$(shell echo $(XILINX_PLATFORM) | awk '/_u50_gen.*_nodma_/'))
ifeq ($(DM_MODE), SB)
VPPLINKFLAGS := --config aat.u50.sb.ini
else
$(error Unsupported DM_MODE=$(DM_MODE) for XILINX_PLATFORM=$(XILINX_PLATFORM))
endif
else ifneq (,$(shell echo $(XILINX_PLATFORM) | awk '/_u250_gen.*_xdma_/'))
ifeq ($(DM_MODE), DMA)
VPPLINKFLAGS := --config aat.u250.ini
else ifeq ($(DM_MODE), SB)
VPPLINKFLAGS := --config aat.u250.sb.ini
else
$(error Unsupported DM_MODE=$(DM_MODE) for XILINX_PLATFORM=$(XILINX_PLATFORM))
endif
else ifneq (,$(shell echo $(XILINX_PLATFORM) | awk '/_u55c_gen.*_xdma_/'))
ifeq ($(DM_MODE), DMA)
VPPLINKFLAGS := --config aat.u55c.ini
else
$(error Unsupported DM_MODE=$(DM_MODE) for XILINX_PLATFORM=$(XILINX_PLATFORM))
endif
# TODO: add your .ini file here according to your card
else
$(error Unsupported XILINX_PLATFORM=$(XILINX_PLATFORM))
endif
endif
```

常见问题：

1. ping板卡没有回复
  
    解决方法：在AAT shell里运行`udpip0 seticmp true`开启ICMP

2. 各模块正常运行，orderbook中已经有了数据，但监听不到输出

    解决方法：应该先在网卡的host端开启监听`nc -l 192.168.20.100 12345 -v`，再在板卡上运行AAT，否则无法建立监听连接

3. 因为证书问题无法完成Bitstream Generation

    解决方法：

    1. http://www.xilinx.com/getlicense 获取10G与10G/25G Eternet的Evaluation版证书
    2. https://github.com/fpgasystems/Vitis_with_100Gbps_TCP-IP 下载这个项目
    3. 在Vitis_with_100Gbps_TCP-IP/fpga-network-stack/scripts/network_stack.tcl文件开头加一行代码
    ```
    set_param xilinx.com:47_license_vitis_kernel_license "yourpath/Xilinx.lic"
    ```
    4. 编译上面的项目
    5. 重新下载没有build过的AAT项目，进行编译，此时应该不会出现证书问题