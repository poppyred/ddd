# readme: OPTFLAGS为外部环境变量
CFLAGS= -std=c99 -Wall -O2 $(OPTFLAGS)

#DEFINED= -D WORDS_BIGENDIAN

.PHONY: all clean
all: crc32_tablegen crc64_tablegen

crc32_tablegen: crc32_tablegen.c crc32_init.c
	${CC} ${CFLAGS} ${DEFINED} -o $@ $^

crc64_tablegen: crc64_tablegen.c crc64_init.c
	${CC} ${CFLAGS} ${DEFINED} -o $@ $^


clean:
	rm -f crc32_tablegen crc64_tablegen
