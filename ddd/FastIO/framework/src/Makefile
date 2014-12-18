#	$Id: 2013-05-23 likunxiang$
#

ARCH = x64;
ifeq ($(shell getconf LONG_BIT), 32)
	ARCH = x86
else
	ARCH = x64
endif

INCLUDE   = -I../../sys -I../inc -I../libcommon/inc

OBJS      =  fio_mac_manager.o fio_conf.o fio_nm_util.o fio_map32.o \
			 proto_map.o fio_timer.o fio_route_manager.o dns_pack.o \
			 fio_ringbuf.o fio_statistics.o fio_context.o fio_nic.o \
			 fio_sysconfig.o fio_handler.o fio_dns_logic.o fio_log.o \
			 fio_pack_misc.o fio_icmp.o fio_syslog.o fio_sysstack.o \
			 fio_sockpair.o fio_multip.o fio_mapstr.o fio_event.o bsd_udpcli.o \
			 fio_test_send.o 

CFLAGS    =  -Wall ${INCLUDE}
#CFLAGS	 += -O2 
CFLAGS	 += -g
CFLAGS	 += -fno-strict-aliasing
ifeq (${MYRELVAL},)
CFLAGS	 += -D__FIO_RELEASE__ -DUSE_BF_OCCUPY
else ifeq (${MYRELVAL}, 1)
CFLAGS	 += -D__FIO_RELEASE__ -DUSE_BF_OCCUPY
endif

# How to make an object file
%.o: %.c %.h 
#	@echo "=*= making object $@ =*="
	${CC} ${CFLAGS} -c $< -o $@

#
# Main targets
#
TARGETS = libfio_as.a
LIBPATH = /lib
LIBPATH64 = /lib64

ifeq ($(TARGETS), $(wildcard $(TARGETS)))
fiorelease:
ifeq ($(ARCH), x86)
	@echo "=*= exist library $@ copy to x86 lib =*="
	@cp $(TARGETS) $(LIBPATH) -f
else
	@echo "=*= exist library $@ copy to x64 lib =*="
	@cp $(TARGETS) $(LIBPATH64) -f
endif

all:
ifeq ($(ARCH), x86)
	@echo "=*= exist library $@ copy to x86 lib =*="
	@cp $(TARGETS) $(LIBPATH) -f
else
	@echo "=*= exist library $@ copy to x64 lib =*="
	@cp $(TARGETS) $(LIBPATH64) -f
endif

else

fiorelease:	${TARGETS}

all: ${TARGETS}

${TARGETS}: ${OBJS}
	@echo "=*= making library $@ =*="
	ar rs $@ ${OBJS} 
	ranlib $@
ifeq ($(ARCH), x86)
	@echo "=*= copy to x86 lib =*="
	@cp $(TARGETS) $(LIBPATH) -f
else
	@echo "=*= copy to x64 lib =*="
	@cp $(TARGETS) $(LIBPATH64) -f
endif
endif

clean:
	@rm -f *.o *~
