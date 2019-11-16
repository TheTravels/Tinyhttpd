#all: httpd client
LIBS = -lpthread #-lsocket

CC       := gcc
CFLAGS   := -g -rdynamic -lmysqlclient -DNDEBUG -W -Wall -lpthread -DGCC_BUILD=1 
CFLAGS   += -DBUILD_THREAD_VIN=1 
#CFLAGS   += -DBUILD_SERVER_YN=1 

ROOT	 := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
INC      := -I $(ROOT)/
BUILD_DIR:= $(ROOT)/build
#SRCS     := main.c daemon_init.c accept_request.c lock.c thread_pool.c #trunking.c
#SRCS     := main.c daemon_init.c lock.c thread_pool.c #trunking.c
#OBJS     := $(addsuffix .o,$(addprefix $(BUILD_DIR)/,$(basename $(SRCS))))

SRCS_LIST     := list.c daemon_init.c lock.c thread_pool.c sql.c
OBJS_LIST     := $(addsuffix .o,$(addprefix $(BUILD_DIR)/,$(basename $(SRCS_LIST))))
SRCS_EN  := encode.c #json_list.c
OBJS_EN  := $(addsuffix .o,$(addprefix $(BUILD_DIR)/,$(basename $(SRCS_EN))))

CCU_DIR  := CCU/protocol
CCU_SRC   = $(notdir $(wildcard $(ROOT)/$(CCU_DIR)/*.c))
CCU_OBJS  = $(addsuffix .o,$(addprefix $(BUILD_DIR)/$(CCU_DIR)/,$(basename $(CCU_SRC))))


Enc_DIR  := OBD_Report/Encrypt
Enc_SRC   = desc.c ied.c md5c.c nn.c prime.c r_keygen.c r_random.c r_stdlib.c rsa.c
Enc_OBJS  = $(addsuffix .o,$(addprefix $(BUILD_DIR)/$(Enc_DIR)/,$(basename $(Enc_SRC))))

OBJS  =
TARGET_OBJS =
#include target.mk
include target_modules.mk
OBJS     += $(TARGET_OBJS)
OBJS_LIST     += $(TARGET_OBJS)
OBJS_EN  += $(OBD_OBJS) $(UTC_OBJS) $(cJSON_OBJS) $(RSA_OBJS) $(OBJS_CFG)


CFLAGS   += $(INC)
HOME     = /home/$(shell whoami)
#IPATH    = "./bin"
IPATH    = $(HOME)/tools/Tinyhttpd
#IPATH    = "~/tools/Tinyhttpd"
#SRC      += $(CCU_SRC)

#all: modules $(BUILD_DIR) $(OBJS) httpd client list #encode
all: $(BUILD_DIR) $(OBJS) httpd #client list #encode
#	echo $(CCU_DIR)
#	echo $(CCU_SRC)
#	echo $(CCU_OBJS)
#	echo $(OBJS)
	@mkdir -p log
	@mkdir -p daemon
	@mkdir -p ./upload/cfg
	@mkdir -p ./upload/bin
	@cp modules/config/ServerConfig.cfg ./
modules:
	@echo modules obj add
#	$(call add_src_build,$(MODULES_OBJS),OBD_Report/lib,da)
	echo $(call add_src_build,MODULES_OBJS,OBD_Report/lib,da)
.PHONY : modules
# 添加编译
define add_src_build
	echo "file my name is $(0)"
	@echo "file  => $(1)"
	@echo "dir   => $(2)"
	@echo "dir   => $(3)"
#	MODULES_OBJS     += $(2) 
	1     += $(2) 

endef
define add_dir_build
    @echo "file my name is $(0)"
    @echo "file  => $(1)"
    @echo "dir   => $(2)"
    @echo "dir   => $(3)"

endef

install: all
	@echo install path: $(IPATH)
#	@echo $(IPATH)
	@mkdir -p $(IPATH)
	@mkdir -p $(IPATH)/log
	@mkdir -p $(IPATH)/daemon
	@mkdir -p $(IPATH)/upload/cfg
	@mkdir -p $(IPATH)/upload/bin
	@cp ./build/httpd $(IPATH)/httpd
#	@cd $(IPATH) && echo $(shell pwd)
#	@cd $(IPATH) && ./httpd -c 
#	@cd $(IPATH) && ./httpd -L 
	@cp ./upload/OBD.cfg $(IPATH)/upload/OBD.cfg
	cp -a ./upload $(IPATH)
#	@cp ./upload/Device.list $(IPATH)/upload/Device.list

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
#	@echo $(CCU_SRC)
#	@echo $(CCU_OBJS)
#	@echo $(OBJS)
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
#	@$(CC) -o $(BUILD_DIR)/$@ $(CFLAGS) $^
	@$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)

list: $(OBJS_LIST)
	@echo LD $(BUILD_DIR)/$@
	@$(CC) -o $(BUILD_DIR)/$@ $^ $(CFLAGS)

client: simpleclient.c
	gcc -W -Wall -o $@ $<
clean:
	rm -rf $(BUILD_DIR) httpd client log *.cfg
