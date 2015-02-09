#	$Id: 2013-05-23 likunxiang$
#

FIONAME =exe/run_fio

CLEANFILES = $(FIONAME) exe/*.o 
INCLUPATH = -I ../sys -I inc
CFLAGS = -pipe 
#CFLAGS += -O2
CFLAGS += -g
#CFLAGS += -Werror 
CFLAGS += -Wall
#CFLAGS += -Wextra 
ifeq (${MYRELVAL},)
CFLAGS   += -D__FIO_RELEASE__ -DUSE_BF_OCCUPY
else ifeq (${MYRELVAL}, 1)
CFLAGS   += -D__FIO_RELEASE__ -DUSE_BF_OCCUPY
endif

LIBPATH = -L ./

LDFLAGS += -lfio_as
LDFLAGS += -lrt -lpthread

SUBDIR = src

FIOOBJS = exe/fio.o 

%.o:	%.c
	$(CC) $(CFLAGS) $(INCLUPATH) -c -o $@ $<

all: $(FIONAME)

$(FIONAME): $(FIOOBJS)
	$(CC) $(CFLAGS) -o $@ $^  $(LIBPATH) $(LDFLAGS)


clean:
	-@rm -rf $(CLEANFILES)
