#include <unistd.h>
#include "libhs.h"

int main(int argc, char *argv[])
{
    int ret;
    char res_file[255];
    sprintf_n(res_file, 255, "./%s.ini", wns_current_appname());
    if (argc > 1) {
        ret = wns_i18n_load_resource("chinese", res_file);
    } else {
        ret = wns_i18n_load_resource("english", res_file);
    }
    if (ret < 0) {
        printf("load i18n failed.\n");
        return -1;
    }

    int i;
    for (i = 0; ; i++) {
        sleep(1);

        // 注意: 需要国际化的日志, 不能使用反斜杠
        WNS_LOG_WARN(_("(%s) test_<warn-all>-%d"),
                wns_current_appname(), i);
        WNS_LOG_WARN(_("(%s) test-%d"), wns_current_appname(), i);
        WNS_LOG_WARN(_("simple"));
        WNS_LOG_WARN(_("si mple"));
    }

    return 0;
}

