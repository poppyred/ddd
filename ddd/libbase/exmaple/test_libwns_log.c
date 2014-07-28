#include <unistd.h>
#include "libhs.h"

int main(int argc, char *argv[])
{
    // 需要定义日志模块的插件配置文件 "/wns/etc/sysconfig/log/test_libhs_log.ini"
    // (其中test_libhs_log为程序运行时的名称)
    // 如果找不到, 则使用默认配置文件 "/wns/etc/sysconfig/log/xtest_run.ini"
    //
    // 配置文件参考 libhs/log_plugins/xtest_run.ini

    int i;
    for (i = 0; ; i++) {
        sleep(1);

        // 输出调试日志
        WNS_LOG_DEBUG(ET_DEBUG_LEVEL2, "test_<debug2-all>-%d", i);

        // 输出警告日志
        WNS_LOG_WARN("test_<warn-all>-%d", i);

        // 输出错误日志
        WNS_LOG_FATAL("test_<fatal-all>-%d", i);

        // 输出普通日志
        WNS_LOG_INFO("test_<info-all>-%d", i);

        // 输出安全日志
        WNS_LOG_SAFE("test_<safe-all>-%d", i);

        // 输出错误日志到telnet
        WNS_WRITE_LOG(TO_TELNET, ET_FATAL, 0, "test_<fatal-telnet>-%d", i);

        // 输出调试日志到终端
        WNS_WRITE_LOG(TO_TERM, ET_DEBUG, ET_DEBUG_LEVEL6, "test_<debug6-term>-%d", i);

        // 输出调试日志到logs
        WNS_WRITE_LOG(TO_LOGS, ET_DEBUG, ET_DEBUG_LEVEL4, "test_<debug4-logs>-%d", i);

        // 输出调试日志到文件
        WNS_WRITE_LOG(TO_FILE, ET_DEBUG, ET_DEBUG_LEVEL8, "test_<debug8-file>-%d", i);
    }

    return 0;
}

