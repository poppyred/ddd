#ifndef __LIBWNS_ERROR_H__
#define __LIBWNS_ERROR_H__

#include <stdio.h>

#define wns_die(msg) do { \
		fprintf(stderr, msg); \
		exit(1); \
	} while (0)

#endif /* __LIBWNS_ERROR_H__ */
