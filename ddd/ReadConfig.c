#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dns_comdef.h>
#include "Main.h"

/* Takes a filename and
 * reads pairs of 'key value' or 'key = value'.
 */

extern char *g_mgr_ip; 
extern char *g_core_ip;

int dns_read_config(const char *fname)
{
	FILE *f;
	char buf[1024];
	char *s, *key, *arg;

	if ((f = fopen(fname, "r")) == NULL)
		return -1;

	while (fgets(buf, sizeof(buf), f) != NULL) {

		if ((s = strchr(buf, '#')) != NULL)
			*s = '\0';
		char *myStrBuf=NULL;
		key = strtok_r(buf, " \t=",&myStrBuf);
		arg = strtok_r(NULL, " \t\n",&myStrBuf);
		if (!key || !arg)
			continue;

		//printf("Found key [%s] arg [%s]\n", key, arg);

		if (!strcmp(key, "Mgr_addr")) {
			g_mgr_ip = strdup(arg);
            hyb_debug("Mgr_addr:%s\n",g_mgr_ip);
			continue;
		}
		if (!strcmp(key, "Core_addr")) {
			g_core_ip = strdup(arg);
            hyb_debug("Core_addr:%s\n",g_core_ip);
			continue;
		}

        /*
		if (!strcmp(key, "response_ip")) {
			response_ip = strdup(arg);
            hyb_debug("response_ip:%s\n",response_ip);
			continue;
		}
		if (!strcmp(key, "mysql_host")) {
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

	return 0;
}
