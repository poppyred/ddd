#ifndef __LIBBASE_DIRUTILS_H__
#define __LIBBASE_DIRUTILS_H__

/**
 * 目录项上的回调
 * \param isdir 是否为目录
 * \param entry_name 目录项全路径
 * \param userdata listdir时的用户数据
 * \return 返回0正常，其它值将导致listdir函数立即结束遍历
 */
typedef int (*dir_entry_func_t)(int isdir, const char *entry_name, void *userdata);

/**
 * 列目录
 * \param dir 目录
 * \param userdata 用户数据
 * \param entry_callback 在每个目录项的回调
 * \return 正常返回0，否则返回entry_callback的值，无法打开目录返回-1，非正常结束可以在errno中取得错误
 */
int h_dirutils_listdir(const char *dir, void *userdata,
        dir_entry_func_t entry_callback);

/**
 * 列目录,以字母排序
 * \param dir 目录
 * \param userdata 用户数据
 * \param entry_callback 在每个目录项的回调
 * \return 正常返回0，否则返回entry_callback的值，无法打开目录返回-1，非正常结束可以在errno中取得错误
 */
int h_dirutils_scandir(const char *dir, void *userdata,
        dir_entry_func_t entry_callback);

/**
 * 递归删除一个目录
 * \param dir 目录
 * \return 正常返回0，可以在errno中取得错误
 */
int h_dirutils_rmdir(const char *dir);

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * 创建目录（抄自busybox)
 * \param path 路径
 * \param mode 模式
 * \param recur 1递归创建目录
 * \return a与b的差值
 */
static inline int h_mkdir (char *path, long mode, int recur)
{
	mode_t mask;
	const char *fail_msg;
	char *s = path;
	char c;
	struct stat st;

	mask = umask(0);
	if (mode == -1) {
		umask(mask);
		mode = (S_IXUSR | S_IXGRP | S_IXOTH |
				S_IWUSR | S_IWGRP | S_IWOTH |
				S_IRUSR | S_IRGRP | S_IROTH) & ~mask;
	} else {
		umask(mask & ~0300);
	}

	do {
		c = 0;

		if (recur) {	/* Get the parent. */
			/* Bypass leading non-'/'s and then subsequent '/'s. */
			while (*s) {
				if (*s == '/') {
					do {
						++s;
					} while (*s == '/');
					c = *s;		/* Save the current char */
					*s = 0;		/* and replace it with nul. */
					break;
				}
				++s;
			}
		}

		if (mkdir(path, 0777) < 0) {
			/* If we failed for any other reason than the directory
			 * already exists, output a diagnostic and return -1.*/
			if (errno != EEXIST
				|| !recur
				|| (stat(path, &st) < 0 || !S_ISDIR(st.st_mode))) {
				fail_msg = "create";
				umask(mask);
				break;
			}
			/* Since the directory exists, don't attempt to change
			 * permissions if it was the full target.  Note that
			 * this is not an error conditon. */
			if (!c) {
				umask(mask);
				return 0;
			}
		}

		if (!c) {
			/* Done.  If necessary, updated perms on the newly
			 * created directory.  Failure to update here _is_
			 * an error.*/
			umask(mask);
			if ((mode != -1) && (chmod(path, mode) < 0)){
				fail_msg = "set permissions of";
				break;
			}
			return 0;
		}

		/* Remove any inserted nul from the path (recursive mode). */
		*s = c;

	} while (1);

	fprintf(stderr,"Cannot %s directory `%s'", fail_msg, path);
	return -1;
}

#endif /*__LIBBASE_DIRUTILS_H__ */
