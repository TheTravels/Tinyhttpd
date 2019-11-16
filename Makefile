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
SRCS     := main.c daemon_init.c lock.c thread_pool.c #trunking.c
OBJS     := $(addsuffix .o,$(addprefix $(BUILD_DIR)/,$(basename $(SRCS))))
SRCS_LIST     := list.c daemon_init.c lock.c thread_pool.c sql.c
OBJS_LIST     := $(addsuffix .o,$(addprefix $(BUILD_DIR)/,$(basename $(SRCS_LIST))))
SRCS_EN  := encode.c #json_list.c
OBJS_EN  := $(addsuffix .o,$(addprefix $(BUILD_DIR)/,$(basename $(SRCS_EN))))
CFG_DIR  := OBD_Report
SRCS_CFG := json_list.c thread_list.c msg_relay.c trunking.c service.c thread_vin.c
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
cJSON_DIR  := modules/cJSON
cJSON_SRC   = cJSON.c mem_malloc.c
cJSON_OBJS  = $(addsuffix .o,$(addprefix $(BUILD_DIR)/$(cJSON_DIR)/,$(basename $(cJSON_SRC))))
CONFIG_DIR  := modules/config
CONFIG_SRC   = config_load.c config_data.c
CONFIG_OBJS  = $(addsuffix .o,$(addprefix $(BUILD_DIR)/$(CONFIG_DIR)/,$(basename $(CONFIG_SRC))))
MODULE_DIR  := modules/lib
MODULE_SRC   = data_base.c
MODULE_OBJS  = $(addsuffix .o,$(addprefix $(BUILD_DIR)/$(MODULE_DIR)/,$(basename $(MODULE_SRC))))
EPOLL_DIR  := modules/epoll
EPOLL_SRC   = epoll.c epoll_server.c
EPOLL_OBJS  = $(addsuffix .o,$(addprefix $(BUILD_DIR)/$(EPOLL_DIR)/,$(basename $(EPOLL_SRC))))
JSON_DIR  := OBD_Report/json
JSON_SRC   = configure.c  vin_list.c
JSON_OBJS  = $(addsuffix .o,$(addprefix $(BUILD_DIR)/$(JSON_DIR)/,$(basename $(JSON_SRC))))
#MYSQL_DIR  := OBD_Report/mysql
#MYSQL_SRC   = MySql.c
MYSQL_DIR  := OBD_Report/lib
MYSQL_SRC   = data_base.c sql.c fw.c
MYSQL_OBJS  = $(addsuffix .o,$(addprefix $(BUILD_DIR)/$(MYSQL_DIR)/,$(basename $(MYSQL_SRC))))

#OBJS     += $(CCU_OBJS) $(OBD_OBJS) $(UTC_OBJS)
SUB_OBJS     := $(OBD_OBJS) $(UTC_OBJS) $(cJSON_OBJS) $(RSA_OBJS) $(OBJS_CFG) $(JSON_OBJS) $(MYSQL_OBJS) $(CONFIG_OBJS) $(EPOLL_OBJS)
OBJS     += $(SUB_OBJS)
OBJS_LIST     += $(SUB_OBJS) 
OBJS_EN  += $(OBD_OBJS) $(UTC_OBJS) $(cJSON_OBJS) $(RSA_OBJS) $(OBJS_CFG)

INC      += -I $(ROOT)/cJSON
INC      += -I $(ROOT)/OBD_Report
#INC      += -I $(ROOT)/OBD_Report/Encrypt
INC      += -I $(ROOT)/RSA
INC      += -I $(ROOT)/UTC/GpsUtcAndLocalTime/GpsUtcAndLocalTime
#INC      += -I $(ROOT)/OBD_Report/mysql
#INC      += -I $(ROOT)/OBD_Report/mysql/include
INC      += -I $(ROOT)/OBD_Report/lib
INC      += -I $(ROOT)/OBD_Report/lib/include
INC      += -I $(ROOT)/modules
CFLAGS   += $(INC)
HOME     = /home/$(shell whoami)
#IPATH    = "./bin"
IPATH    = $(HOME)/tools/Tinyhttpd
#IPATH    = "~/tools/Tinyhttpd"
#SRC      += $(CCU_SRC)

#all: modules $(BUILD_DIR) $(OBJS) httpd client list #encode
all: $(BUILD_DIR) $(OBJS) httpd client list #encode
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
