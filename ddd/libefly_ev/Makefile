TARGET    = libefly_ev.so
STATIC    = libefly_ev.a
INC_DIR   =  /usr/local/include ../libbase/include 
SRC_DIR   =  . 
CFLAGS    =  -Wall   -fPIC  -std=gnu99 -fno-strict-aliasing -ggdb -O2
CXXFLAGS  =  -Wall  -fno-strict-aliasing -ggdb
LIB_DEPENDS =  -shared -ldl  -rdynamic -lrt -L../ -lbase -L/usr/local/lib/ -levent
#LIB_DEPENDS = --shared -ldl  -rdynamic -lrt #-L/home/swen/libhs/ -lbase  -L/usr/local/lib/ -levent
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
AR	 = ar rcs

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
	rm -rf $(ALL_OBJ)  $(TARGET) *.o *.a
	
$(TARGET): $(ALL_OBJ)
	$(CC)  $(ALL_OBJ)  -o $@ $(C_LIB)
	$(AR)  $(STATIC)  $(ALL_OBJ) 

	cp $(TARGET) ../lib/
	cp $(STATIC) ../lib/
	cp $(STATIC) /lib64/
	cp $(TARGET) /lib64/
#cp libefly_ev.so ../
################################################################################
