#all: httpd client
LIBS = -lpthread #-lsocket

CC       := gcc
CFLAGS   := -g -W -Wall -lpthread 

ROOT	 := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
INC      := -I $(ROOT)
BUILD_DIR:= $(ROOT)/build
SRCS     := httpd.c
OBJS     := $(addsuffix .o,$(addprefix $(BUILD_DIR)/,$(basename $(SRCS))))
CCU_DIR  := CCU/protocol
CCU_SRC   = $(notdir $(wildcard $(ROOT)/$(CCU_DIR)/*.c))
CCU_OBJS  = $(addsuffix .o,$(addprefix $(BUILD_DIR)/$(CCU_DIR)/,$(basename $(CCU_SRC))))
OBD_DIR  := OBD_Report/agreement
OBD_SRC   = $(notdir $(wildcard $(ROOT)/$(OBD_DIR)/*.c))
OBD_OBJS  = $(addsuffix .o,$(addprefix $(BUILD_DIR)/$(OBD_DIR)/,$(basename $(OBD_SRC))))
UTC_SRC   = OBD_Report/UTC/GpsUtcAndLocalTime/GpsUtcAndLocalTime/DateTime.c
UTC_OBJS  = $(addsuffix .o,$(addprefix $(BUILD_DIR)/,$(basename $(UTC_SRC))))

#OBJS     += $(CCU_OBJS) $(OBD_OBJS) $(UTC_OBJS)
OBJS     += $(OBD_OBJS) $(UTC_OBJS)

INC      += -I $(ROOT)/OBD_Report
INC      += -I $(ROOT)/OBD_Report/UTC/GpsUtcAndLocalTime/GpsUtcAndLocalTime
CFLAGS   += $(INC)

#SRC      += $(CCU_SRC)

all: $(BUILD_DIR) $(OBJS) httpd client
#	echo $(CCU_DIR)
#	echo $(CCU_SRC)
#	echo $(CCU_OBJS)
#	echo $(OBJS)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
	@echo $(CCU_SRC)
	@echo $(CCU_OBJS)
	@echo $(OBJS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c -o $@ $(CFLAGS) $<

httpd: $(OBJS)
#	gcc -g -W -Wall $(LIBS) -o $@ $<
#	$(CC) -o $(BUILD_DIR)/$@ $(CFLAGS) $<
	$(CC) -o $(BUILD_DIR)/$@ $(CFLAGS) $^

client: simpleclient.c
	gcc -W -Wall -o $@ $<
clean:
	rm -rf $(BUILD_DIR) httpd client
