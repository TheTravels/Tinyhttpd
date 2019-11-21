# x86
CC = gcc
RSA_SRC := main.c bignum.c prime.c rsa.c
APP = build/test

all:${APP}

${APP}:
	@mkdir -p build
	${CC} -Wall ${RSA_SRC} -o $@

clean:
	@rm -f ${APP}

# 9x07
# COMPILER_ROOT := /opt/ql-oe/sysroots
# CROSS_COMPILER := $(COMPILER_ROOT)/x86_64-linux/usr/bin/arm-oe-linux-gnueabi/arm-oe-linux-gnueabi-
# GCC_VERSION := 4.9.2

# TARGET_EXE := build/rsa
# LOCAL_SRC_FILES := main.c bignum.c prime.c rsa.c

# LOCAL_C_FLAGS := -march=armv7-a -mfloat-abi=softfp -mfpu=neon --sysroot=$(COMPILER_ROOT)/mdm9607 \
#                  -g -Wall -Wno-unused-function -pthread -Wno-unused-but-set-variable \
#                  -Wno-missing-braces

# all:$(TARGET_EXE)

# $(TARGET_EXE):
# 	$(CROSS_COMPILER)gcc $(LOCAL_C_FLAGS) $(INCLUDES) $(LOCAL_SRC_FILES) -o $@

# .PHONY: clean
# clean:
# 	@find build -name "*.o" -exec rm {} \;
# 	@find build -name $(notdir $(TARGET_EXE)) -exec rm {} \;
