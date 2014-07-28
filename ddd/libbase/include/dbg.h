#ifndef __dbg_h__
#define __dbg_h__

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_err(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_warn(M, ...) fprintf(stderr, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define check(A, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define sentinel(M, ...)  { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define check_mem(A) check((A), "Out of memory.")

#define check_debug(A, M, ...) if(!(A)) { debug(M, ##__VA_ARGS__); errno=0; goto error; }

// added by adam 2012-10-18
// dummy test macro for xtest
#define adam_check(A) if(!(A)) { log_err(""); errno=0; return; }
#define EXPECT_EQ(x,y) adam_check((x) == (y))
#define EXPECT_NE(x,y) adam_check((x) != (y))
#define EXPECT_TRUE(x) adam_check(0 != (x))
#define EXPECT_FALSE(x) adam_check(0 == (x))
#define EXPECT_STREQ(x,y) adam_check(0 == strcmp((x),(y)))
#define TEST(x,y) void test_##x_##y()
#define RUN_TEST(x,y) test_##x_##y()

#endif
