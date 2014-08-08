TARGET    = libbase.so
STATIC    = libbase.a
INC_DIR   =  ./include  
SRC_DIR   =  . 
CFLAGS    =  -Wall  -DSF_ARCH_X86_64 -fPIC -std=gnu99 -fno-strict-aliasing -ggdb -O2
CXXFLAGS  =  -Wall  -fno-strict-aliasing -ggdb
LIB_DEPENDS = --shared -ldl  -rdynamic
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
	rm -rf $(ALL_OBJ)  $(TARGET) *.o $(STATIC)
	
$(TARGET): $(ALL_OBJ)
	$(CC) $(C_LIB) $(ALL_OBJ)  -o $@
	$(AR) $(STATIC)  $(ALL_OBJ)  	

	rm -rf ../lib/$(TARGET)
	rm -rf ../lib/$(STATIC)
	rm -rf /lib64/$(TARGET)
	rm -rf /lib64/$(STATIC)
	cp $(TARGET) ../lib/
	cp $(STATIC) ../lib/
	cp $(TARGET) /lib64/
	cp $(STATIC) /lib64/
################################################################################
