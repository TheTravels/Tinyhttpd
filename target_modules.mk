# target list

OBJS  += $(addsuffix .o,$(addprefix $(BUILD_DIR)/modules/main/,$(basename main.c daemon_init.c lock.c thread_pool.c)))
CFG_DIR  := OBD_Report
CFG_SRC  := json_list.c thread_list.c msg_relay.c trunking.c service.c thread_vin.c
#TARGET_OBJS  += $(addsuffix .o,$(addprefix $(BUILD_DIR)/OBD_Report/,$(basename $(CFG_SRC))))

#RSA_DIR  := RSA
#RSA_SRC   = bignum.c prime.c rsa.c
TARGET_OBJS  += $(addsuffix .o,$(addprefix $(BUILD_DIR)/RSA/,$(basename bignum.c prime.c rsa.c)))
# modules
TARGET_OBJS  += $(addsuffix .o,$(addprefix $(BUILD_DIR)/modules/UTC/,$(basename DateTime.c)))
TARGET_OBJS  += $(addsuffix .o,$(addprefix $(BUILD_DIR)/modules/cJSON/,$(basename cJSON.c mem_malloc.c)))
TARGET_OBJS  += $(addsuffix .o,$(addprefix $(BUILD_DIR)/modules/config/,$(basename config_load.c config_data.c)))
TARGET_OBJS  += $(addsuffix .o,$(addprefix $(BUILD_DIR)/modules/epoll/,$(basename epoll.c epoll_server.c)))
TARGET_OBJS  += $(addsuffix .o,$(addprefix $(BUILD_DIR)/modules/json/,$(basename configure.c  vin_list.c)))
TARGET_OBJS  += $(addsuffix .o,$(addprefix $(BUILD_DIR)/modules/obd/,$(basename $(CFG_SRC))))

#MYSQL
#TARGET_OBJS  += $(addsuffix .o,$(addprefix $(BUILD_DIR)/OBD_Report/lib/,$(basename data_base.c sql.c fw.c)))
TARGET_OBJS  += $(addsuffix .o,$(addprefix $(BUILD_DIR)/modules/lib/,$(basename data_base.c sql.c fw.c)))

OBD_DIR  := modules/agreement
OBD_SRC   = $(notdir $(wildcard $(ROOT)/$(OBD_DIR)/*.c))
TARGET_OBJS  += $(addsuffix .o,$(addprefix $(BUILD_DIR)/$(OBD_DIR)/,$(basename $(OBD_SRC))))


#INC      += -I $(ROOT)/OBD_Report
#INC      += -I $(ROOT)/OBD_Report/Encrypt
INC      += -I $(ROOT)/RSA
#INC      += -I $(ROOT)/UTC/GpsUtcAndLocalTime/GpsUtcAndLocalTime
#INC      += -I $(ROOT)/OBD_Report/mysql
#INC      += -I $(ROOT)/OBD_Report/mysql/include
INC      += -I $(ROOT)/modules/lib
INC      += -I $(ROOT)/modules/lib/include
INC      += -I $(ROOT)/modules
INC      += -I $(ROOT)/modules/UTC
INC      += -I $(ROOT)/modules/cJSON
INC      += -I $(ROOT)/modules/main

