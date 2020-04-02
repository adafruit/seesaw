BOARD=debug
-include Makefile.user
include boards/$(BOARD)/board.mk
CC=arm-none-eabi-gcc
CXX=arm-none-eabi-g++

ifeq ($(DEBUG), 1)
ENABLE_LOGGING = -DENABLE_LOGGING
else
ENABLE_LOGGING =
endif

COMMON_FLAGS = -mthumb -mcpu=cortex-m0plus -Os -g3 -D$(CHIP_FAMILY) -D__$(CHIP_VARIANT)__ -DBOARD_$(BOARD_NAME) $(ENABLE_LOGGING)

WFLAGS = \
-Wall -Werror

SFLAGS = $(COMMON_FLAGS) \
-x assembler-with-cpp -c \
$(WFLAGS)
CFLAGS = $(COMMON_FLAGS) \
-x c -std=gnu99 -c \
-MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" \
$(WFLAGS)
CXXFLAGS = $(COMMON_FLAGS) \
-ffunction-sections -fdata-sections \
-fno-rtti -fno-exceptions -c -std=c++11 \
-MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" \
$(WFLAGS)

LDFLAGS= $(COMMON_FLAGS) \
-Wall -Werror -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -Wl,--unresolved-symbols=report-all -Wl,--warn-common \
-Wl,--warn-section-align -Wl,--warn-unresolved-symbols \
-save-temps \
--specs=nano.specs --specs=nosys.specs
BUILD_PATH=build/$(BOARD)

QPPORT = lib/qp/ports/arm-cm/qxk/gnu

INCLUDES = -I. -I./include -I./include/USB -I./bsp -I./lib/qp/extras -I./lib/qp/include -I./lib/qp/source -I$(QPPORT)
INCLUDES += -I./boards/$(BOARD) -Ilib/cmsis/CMSIS/Core/Include
INCLUDES += -I$(BUILD_PATH)

ifeq ($(CHIP_FAMILY), SAMD21)
INCLUDES += -Ilib/samd21/samd21a/include/
endif

ifeq ($(CHIP_FAMILY), SAMD09)
INCLUDES += -Ilib/samd09/include/
endif

ifeq ($(CHIP_FAMILY), SAMD10)
INCLUDES += -Ilib/samd10/include/
endif

SSOURCES = \
	$(QPPORT)/qxk_port.S \

ifeq ($(CHIP_FAMILY), SAMD21)
CSOURCES = Device_Startup/startup_samd21.c \
	Device_Startup/system_samd21.c
endif

ifeq ($(CHIP_FAMILY), SAMD09)
CSOURCES = Device_Startup/startup_samd09.c \
	Device_Startup/system_samd09.c
endif

ifeq ($(CHIP_FAMILY), SAMD10)
CSOURCES = Device_Startup/startup_samd10.c \
	Device_Startup/system_samd10.c
endif

COMMON_SRC = \
	lib/qp/source/qep_hsm.cpp \
	lib/qp/source/qep_msm.cpp \
	lib/qp/source/qf_act.cpp \
	lib/qp/source/qf_actq.cpp \
	lib/qp/source/qf_defer.cpp \
	lib/qp/source/qf_dyn.cpp \
	lib/qp/source/qf_mem.cpp \
	lib/qp/source/qf_ps.cpp \
	lib/qp/source/qf_qact.cpp \
	lib/qp/source/qf_qeq.cpp \
	lib/qp/source/qf_qmact.cpp \
	lib/qp/source/qf_time.cpp \
	lib/qp/source/qxk.cpp \
	lib/qp/source/qxk_mutex.cpp \
	lib/qp/source/qxk_sema.cpp \
	lib/qp/source/qxk_xthr.cpp \
	lib/qp/include/qstamp.cpp \
	lib/qp/extras/fw_evt.cpp \
	lib/qp/extras/fw_log.cpp \

SOURCES = $(COMMON_SRC) \
	source/bsp.cpp \
	source/event.cpp \
	source/AOADC.cpp \
	source/AODAC.cpp \
	source/AOTouch.cpp \
	source/AOInterrupt.cpp \
	source/AOSERCOM.cpp \
	source/AOTimer.cpp \
	source/dap.cpp \
	source/Delegate.cpp \
	source/I2CSlave.cpp \
	source/SPISlave.cpp \
	source/main.cpp \
	source/Neopixel.cpp \
	source/System.cpp \
	source/AOKeypad.cpp \
	source/AOEncoder.cpp \
	bsp/bsp_adc.cpp \
	bsp/bsp_gpio.cpp \
	bsp/bsp_sercom.cpp \
	bsp/bsp_timer.cpp \
	bsp/bsp_dma.cpp \
	bsp/pinmux.cpp \
	bsp/bsp_neopix.cpp \
	bsp/adafruit_ptc.cpp \

ifeq ($(CHIP_FAMILY), SAMD21)
FULL_SOURCES = $(SOURCES) \
	source/AOUSB.cpp \
	source/USB/CDC.cpp \
	source/USB/USBCore.cpp
endif

ifeq ($(CHIP_FAMILY), SAMD09)
FULL_SOURCES = $(SOURCES)
endif

ifeq ($(CHIP_FAMILY), SAMD10)
FULL_SOURCES = $(SOURCES)
endif

SOBJECTS = $(patsubst %.S,$(BUILD_PATH)/%.o,$(SSOURCES))
COBJECTS = $(patsubst %.c,$(BUILD_PATH)/%.o,$(CSOURCES))
OBJECTS = $(patsubst %.cpp,$(BUILD_PATH)/%.o,$(FULL_SOURCES))

NAME=seesaw-$(BOARD)
EXECUTABLE=$(BUILD_PATH)/$(NAME).bin
ARTIFACT=fw/$(NAME).bin

all: dirs $(EXECUTABLE)

dirs:
	@echo "Building $(BOARD)"
	@python scripts/datecode.py
	-@mkdir -p $(BUILD_PATH)
	-@mkdir -p $(BUILD_PATH)/lib/qp/source
	-@mkdir -p $(BUILD_PATH)/lib/qp/include
	-@mkdir -p $(BUILD_PATH)/lib/qp/extras
	-@mkdir -p $(BUILD_PATH)/lib/qp/ports/arm-cm/qxk/gnu/
	-@mkdir -p $(BUILD_PATH)/source/USB
	-@mkdir -p $(BUILD_PATH)/Device_Startup
	-@mkdir -p $(BUILD_PATH)/bsp
	-@mkdir -p fw

.PHONY: artifact
artifact: $(ARTIFACT)
$(ARTIFACT): $(EXECUTABLE)
	@cp $< $@


$(EXECUTABLE): $(SOBJECTS) $(COBJECTS) $(OBJECTS)
	$(CC) -L$(BUILD_PATH) $(LDFLAGS) \
		 -T$(LINKER_SCRIPT) \
		 -Wl,-Map,$(BUILD_PATH)/$(NAME).map -o $(BUILD_PATH)/$(NAME).elf $(SOBJECTS) $(COBJECTS) $(OBJECTS)
	arm-none-eabi-objcopy -O binary $(BUILD_PATH)/$(NAME).elf $@
	@echo
	-@arm-none-eabi-size $(BUILD_PATH)/$(NAME).elf
	@echo

$(BUILD_PATH)/%.o: %.S $(wildcard include/*.h boards/*/*.h) | dirs
	@echo "$<"
	@$(CC) $(SFLAGS) $(BLD_EXTA_FLAGS) $(INCLUDES) $< -o $@

$(BUILD_PATH)/%.o: %.c $(wildcard include/*.h boards/*/*.h) | dirs
	@echo "$<"
	@$(CC) $(CFLAGS) $(BLD_EXTA_FLAGS) $(INCLUDES) $< -o $@

$(BUILD_PATH)/%.o: %.cpp $(wildcard include/*.h boards/*/*.h) | dirs
	@echo "$<"
	@$(CXX) $(CXXFLAGS) $(BLD_EXTA_FLAGS) $(INCLUDES) $< -o $@

clean:
	rm -rf build

.PHONY: board-%
board-%:
	$(MAKE) BOARD=$* artifact

.PHONY: all-boards
all-boards: $(patsubst %, board-%, $(shell cd boards; ls))
