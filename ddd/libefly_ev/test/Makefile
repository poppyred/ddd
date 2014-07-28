TARGET    = test
INC_DIR   = /usr/local/include ../ 
SRC_DIR   =  . 
CFLAGS    = -DSF_ARCH_X86_64  -std=gnu99 -fno-strict-aliasing -ggdb -O2
CXXFLAGS  = -fno-strict-aliasing -ggdb
#LIB_DEPENDS = -L/usr/local/lib  -levent
LIB_DEPENDS = -L../ -lefly_ev
#LIB_DEPENDS = -L/home/swen/libhs/ -lbase
#LIB_DEPENDS = -L/home/compile_env/wns1.0/commonlibs/lib/ -L../lib/ -lpthread  -lwns -lauth_attr
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
