
# Need to choose which xrt library to link depending on emulation option
ifeq ($(XCL_EMULATION_MODE), sw_emu)
	XRT_DEFINES := -DXCL_EMULATION_MODE=sw_emu
	XRT_LIB := -lxrt_swemu
else
ifeq ($(XCL_EMULATION_MODE), hw_emu)
	XRT_DEFINES := -DXCL_EMULATION_MODE=hw_emu
	XRT_LIB := -lxrt_hwemu
else
	XRT_LIB := -lxrt_core
endif
endif

# Always link xrt_coreutil
XRT_LIB += -lxrt_coreutil





# Set project directory one level above of Makefile directory. $(CURDIR) is a GNU make variable containing the path to the current working directory
PROJDIR := $(realpath $(CURDIR)/../../..)
SOURCEDIR := $(PROJDIR)
BUILDDIR := $(PROJDIR)/build
OUTPUTDIR := $(PROJDIR)/../build

# Name of the final executable
TARGET = aat_shell_exe

# Decide whether the commands will be shown or not
VERBOSE = TRUE

# Create the list of directories
DIRS = \
	drivers/common/device_interface \
	drivers/common/ethernet \
	drivers/common/tcp_udp_ip \
	drivers/aat/clock_tick_generator \
	drivers/aat/feed_handler \
	drivers/aat/order_book \
	drivers/aat/order_book_data_mover \
	drivers/aat/order_entry \
	drivers/aat/pricing_engine \
	drivers/aat/line_handler \
	drivers/netcap/network_capture \
	drivers/netcap/network_tap \
	framework/shell \
	framework/shell_ext/shell_common_objects \
	framework/shell_ext/shell_aat_objects \
	framework/shell_ext/shell_netcap_objects \
	applications/aat/aat_objects \
	applications/aat/aat_shell_exe
	


SOURCEDIRS = $(foreach dir, $(DIRS), $(addprefix $(SOURCEDIR)/, $(dir)))
TARGETDIRS = $(foreach dir, $(DIRS), $(addprefix $(BUILDDIR)/, $(dir)))

# Generate the GCC includes parameters by adding -I before each source folder
INCLUDES = $(foreach dir, $(SOURCEDIRS), $(addprefix -I, $(dir)))

INCLUDES += -I$(XILINX_XRT)/include

# Add this list to VPATH, the place make will look for the source files
VPATH = $(SOURCEDIRS)

# Create a list of *.c sources in DIRS
SOURCES = $(foreach dir,$(SOURCEDIRS),$(wildcard $(dir)/*.cpp))

# Define objects for all sources
OBJS := $(subst $(SOURCEDIR),$(BUILDDIR),$(SOURCES:.cpp=.o))

# Define dependencies files for all objects
DEPS = $(OBJS:.o=.d)

# Name the compiler
CXX = g++
DEFINES	 := -D_UNICODE
CXXFLAGS := -g -std=gnu++11 -fPIC -pthread -D_REENTRANT $(DEFINES) $(XRT_DEFINES)  -pedantic-errors -Wall -Wextra 
LDFLAGS  := -pthread -L$(XILINX_XRT)/lib  -lstdc++ -lm -luuid $(XRT_LIB)

# OS specific part
ifeq ($(OS),Windows_NT)
    RM = del /F /Q 
    RMDIR = -RMDIR /S /Q
    MKDIR = -mkdir
    ERRIGNORE = 2>NUL || true
    SEP=\\
else
    RM = rm -rf 
    RMDIR = rm -rf 
    MKDIR = mkdir -p
    ERRIGNORE = 2>/dev/null
    SEP=/
endif

# Remove space after separator
PSEP = $(strip $(SEP))

# Hide or not the calls depending of VERBOSE
ifeq ($(VERBOSE),TRUE)
    HIDE =  
else
    HIDE = @
endif

# Define the function that will generate each rule
define generateRules
$(1)/%.o: %.cpp
	@echo Building $$@
	$(HIDE)$(CXX) $(CXXFLAGS) -c $$(INCLUDES) -o $$(subst /,$$(PSEP),$$@) $$(subst /,$$(PSEP),$$<) -MMD
endef

.PHONY: all clean directories 

all: directories $(OUTPUTDIR)/$(TARGET)

$(OUTPUTDIR)/$(TARGET): $(OBJS)
	$(HIDE)echo Linking $@
	$(HIDE)$(CXX) $(CXXFLAGS) $(INCLUDE) $(OBJS) -o $(OUTPUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS)

# Include dependencies
-include $(DEPS)

# Generate rules
$(foreach targetdir, $(TARGETDIRS), $(eval $(call generateRules, $(targetdir))))

directories: 
	$(HIDE)$(MKDIR) $(subst /,$(PSEP),$(TARGETDIRS)) $(ERRIGNORE)

# Remove all objects, dependencies and executable files generated during the build
clean:
	$(HIDE)$(RMDIR) $(subst /,$(PSEP),$(TARGETDIRS)) $(ERRIGNORE)
	$(HIDE)$(RM) $(OUTPUTDIR)/$(TARGET) $(ERRIGNORE)
	@echo Cleaning done ! 
