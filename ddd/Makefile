TARGET    = eflydns
INC_DIR   = ./libbase/include ./include ./libefly_ev ./FastIO/framework/inc ./FastIO/sys/ /opt/librdkafka/ 
SRC_DIR   =  . 
CFLAGS    =  -Wall -g #-O2 -fno-strict-aliasing #-Werror#-DSF_ARCH_X86_64 -std=gnu99 -fno-strict-aliasing -O2
CXXFLAGS  =  -ggdb #-Wall  -fno-strict-aliasing -ggdb
LIB_DEPENDS = ./lib/libbase.a  -lpthread -L./lib/ -lefly_ev -lfio_as  -lm -lz -lrt -ldl ./lib/libssl.a ./lib/libcrypto.a
#LIB_DEPENDS = 
#-lxtest
#################################################################################
# makefile template
#################################################################################
CC       = gcc
CXX      = g++
CPP      = cpp
CFLAGS   += $(C_INC)
CXXFLAGS += $(C_INC)
CFLAGS	 += $(EXTRA_CFLAGS)
CXXFLAGS += $(EXTRA_CXXFLAGS)
#AR	 = ar

C_INC    = $(INC_DIR:%=-I%)
C_LIB    = $(LIB_DIR:%=-L%) $(LIB_DEPENDS:lib%=-l%) $(SYS_LIB:lib%=-l%)


C_SRC   = $(wildcard $(SRC_DIR:%=%/*.c))
CXX_SRC = $(wildcard $(SRC_DIR:%=%/*.cpp))
C_OBJ   = $(C_SRC:%.c=%.o)
CXX_OBJ = $(CXX_SRC:%.cpp=%.o)
ALL_OBJ = $(C_OBJ) $(CXX_OBJ)


.PHONY: all clean

all: $(TARGET)

clean:
	rm -rf $(ALL_OBJ)  $(TARGET) *.o
	
$(TARGET): $(ALL_OBJ)
	$(CC) -o $@ $(ALL_OBJ) $(C_LIB)
#$(STATIC): $(ALL_OBJ)
#	$(AR) $(c_lib) $(ALL_OBJ)  -o $@	
################################################################################
