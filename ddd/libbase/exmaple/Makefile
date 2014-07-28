# readme: OPTFLAGS为外部环境变量
MYINC = -I../include -I../../include
LINKERS = -L../../log_init -llog_init -L.. -lwns
CFLAGS = -g -Wall $(OPTFLAGS)

SRCS = $(wildcard *.c)
BUILD = $(patsubst %.c, %, $(SRCS))

.c:
	$(CC) -o $@ $< $(CFLAGS) $(MYINC) $(LINKERS)

.PHONY:clean
	
all:$(BUILD)

clean:
	rm -f $(BUILD)
