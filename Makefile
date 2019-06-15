#all: httpd client
LIBS = -lpthread #-lsocket

CC       := gcc
CFLAGS   := -g -W -Wall -lpthread 

ROOT	 := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
INC      := -I $(ROOT)/
BUILD_DIR:= $(ROOT)/build
SRCS     := httpd.c #json_list.c
OBJS     := $(addsuffix .o,$(addprefix $(BUILD_DIR)/,$(basename $(SRCS))))
SRCS_EN  := encode.c #json_list.c
OBJS_EN  := $(addsuffix .o,$(addprefix $(BUILD_DIR)/,$(basename $(SRCS_EN))))
CFG_DIR  := OBD_Report
SRCS_CFG := json_list.c
OBJS_CFG := $(addsuffix .o,$(addprefix $(BUILD_DIR)/$(CFG_DIR)/,$(basename $(SRCS_CFG))))
CCU_DIR  := CCU/protocol
CCU_SRC   = $(notdir $(wildcard $(ROOT)/$(CCU_DIR)/*.c))
CCU_OBJS  = $(addsuffix .o,$(addprefix $(BUILD_DIR)/$(CCU_DIR)/,$(basename $(CCU_SRC))))
OBD_DIR  := OBD_Report/agreement
OBD_SRC   = $(notdir $(wildcard $(ROOT)/$(OBD_DIR)/*.c))
OBD_OBJS  = $(addsuffix .o,$(addprefix $(BUILD_DIR)/$(OBD_DIR)/,$(basename $(OBD_SRC))))
Enc_DIR  := OBD_Report/Encrypt
Enc_SRC   = desc.c ied.c md5c.c nn.c prime.c r_keygen.c r_random.c r_stdlib.c rsa.c
Enc_OBJS  = $(addsuffix .o,$(addprefix $(BUILD_DIR)/$(Enc_DIR)/,$(basename $(Enc_SRC))))
RSA_DIR  := RSA
RSA_SRC   = bignum.c prime.c rsa.c
RSA_OBJS  = $(addsuffix .o,$(addprefix $(BUILD_DIR)/$(RSA_DIR)/,$(basename $(RSA_SRC))))
UTC_SRC   = UTC/GpsUtcAndLocalTime/GpsUtcAndLocalTime/DateTime.c
UTC_OBJS  = $(addsuffix .o,$(addprefix $(BUILD_DIR)/,$(basename $(UTC_SRC))))
cJSON_DIR  := cJSON
cJSON_SRC   = cJSON.c mem_malloc.c
cJSON_OBJS  = $(addsuffix .o,$(addprefix $(BUILD_DIR)/$(cJSON_DIR)/,$(basename $(cJSON_SRC))))

#OBJS     += $(CCU_OBJS) $(OBD_OBJS) $(UTC_OBJS)
OBJS     += $(OBD_OBJS) $(UTC_OBJS) $(cJSON_OBJS) $(RSA_OBJS) $(OBJS_CFG)
OBJS_EN  += $(OBD_OBJS) $(UTC_OBJS) $(cJSON_OBJS) $(RSA_OBJS) $(OBJS_CFG)

INC      += -I $(ROOT)/cJSON
INC      += -I $(ROOT)/OBD_Report
#INC      += -I $(ROOT)/OBD_Report/Encrypt
INC      += -I $(ROOT)/RSA
INC      += -I $(ROOT)/UTC/GpsUtcAndLocalTime/GpsUtcAndLocalTime
CFLAGS   += $(INC)

#SRC      += $(CCU_SRC)

all: $(BUILD_DIR) $(OBJS) httpd client encode
#	echo $(CCU_DIR)
#	echo $(CCU_SRC)
#	echo $(CCU_OBJS)
#	echo $(OBJS)
	@mkdir -p log

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
#	@echo $(CCU_SRC)
#	@echo $(CCU_OBJS)
	@echo $(OBJS)
#	echo $(OBJS_CFG)
#	echo $(SRCS_CFG)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo CC $@
	@$(CC) -c -o $@ $(CFLAGS) $<

#cfg:$(OBJS_CFG) 
#	@echo LD $(BUILD_DIR)/$@
#	@$(CC) -o $(BUILD_DIR)/$@ $(CFLAGS) $^
encode:$(OBJS_EN) 
	@echo LD $(BUILD_DIR)/$@
	@$(CC) -o $(BUILD_DIR)/$@ $(CFLAGS) $^
httpd: $(OBJS)
#	gcc -g -W -Wall $(LIBS) -o $@ $<
#	$(CC) -o $(BUILD_DIR)/$@ $(CFLAGS) $<
	@echo LD $(BUILD_DIR)/$@
	@$(CC) -o $(BUILD_DIR)/$@ $(CFLAGS) $^

client: simpleclient.c
	gcc -W -Wall -o $@ $<
clean:
	rm -rf $(BUILD_DIR) httpd client log *.cfg
