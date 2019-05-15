#all: httpd client
LIBS = -lpthread #-lsocket

CC       := gcc
CFLAGS   := -g -W -Wall -lpthread 

ROOT	 := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
BUILD_DIR:= $(ROOT)/build
SRCS     := httpd.c
OBJS     := $(addsuffix .o,$(addprefix $(BUILD_DIR)/,$(basename $(SRCS))))
CCU_DIR  := CCU/protocol
CCU_SRC   = $(notdir $(wildcard $(ROOT)/$(CCU_DIR)/*.c))
CCU_OBJS  = $(addsuffix .o,$(addprefix $(BUILD_DIR)/$(CCU_DIR)/,$(basename $(CCU_SRC))))

OBJS     += $(CCU_OBJS)

#SRC      += $(CCU_SRC)

all: $(BUILD_DIR) $(OBJS) httpd client
#	echo $(CCU_DIR)
	echo $(CCU_SRC)
	echo $(CCU_OBJS)
	echo $(OBJS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	echo $(CCU_SRC)
	echo $(CCU_OBJS)
	echo $(OBJS)

$(BUILD_DIR)/%.o: %.c
	$(V1) mkdir -p $(dir $@)
	$(CC) -c -o $@ $(CFLAGS) $<

httpd: $(OBJS)
#	gcc -g -W -Wall $(LIBS) -o $@ $<
	$(CC) -o $@ $(CFLAGS) $<

client: simpleclient.c
	gcc -W -Wall -o $@ $<
clean:
	rm -rf $(BUILD_DIR) httpd client
