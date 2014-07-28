#include "dbg.h"
#include "libhs.h"

int main(int argc, char *argv[])
{
    const char *cur_appname = wns_current_appname();
    check(cur_appname != NULL, "get current appname failed.");
    check(0 == strcmp(cur_appname, "test_libhs_appname"),
            "get current appname not correct.");

    return 0;
error:
    return -1;
}
