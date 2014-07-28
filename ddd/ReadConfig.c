#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Main.h"

/* Takes a filename and
 * reads pairs of 'key value' or 'key = value'.
 */

extern char *dns_ip; //主DNS地址
extern char *bind_ip; //绑定公网地址
extern char *response_ip;

int read_config(const char *fname)
{
	FILE *f;
	char buf[1024];
	char *s, *key, *arg;

	if ((f = fopen(fname, "r")) == NULL)
		return 0;

	while (fgets(buf, sizeof(buf), f) != NULL) {

		if ((s = strchr(buf, '#')) != NULL)
			*s = '\0';
		char *myStrBuf=NULL;
		key = strtok_r(buf, " \t=",&myStrBuf);
		arg = strtok_r(NULL, " \t\n",&myStrBuf);
		if (!key || !arg)
			continue;

		//printf("Found key [%s] arg [%s]\n", key, arg);

		if (!strcmp(key, "dns_ip")) {
			dns_ip = strdup(arg);
			continue;
		}
		if (!strcmp(key, "bind_ip")) {
			bind_ip = strdup(arg);
			continue;
		}
		if (!strcmp(key, "response_ip")) {
			response_ip = strdup(arg);
			continue;
		}
		/*if (!strcmp(key, "mysql_host")) {
			mysql_host = strdup(arg);
			continue;
		}
		if (!strcmp(key, "mysql_usr")) {
			mysql_usr = strdup(arg);
			continue;
		}
		if (!strcmp(key, "mysql_pwd")) {
			mysql_pwd = strdup(arg);
			continue;
		}*/

		//printf("Unable to parse '%s'\n", buf);
	}

	fclose(f);

	return 1;
}
