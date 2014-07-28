/**
 * \file i18n.h
 * 国际化例程
 * \author hyb
 * \defgroup base 基础函数库
 * \{
 */
#ifndef __LIBWNS_I18N_H__
#define __LIBWNS_I18N_H__

/**
 * 国际化字串转换
 */
#define _T(x) wns_i18n_gettext(x)

/**
 * 国际化字串转换
 * \param ori_str 配置文件中的id
 * \return 配置文件中指定的翻译
 */
const char *wns_i18n_gettext(const char *ori_str);

int wns_i18n_load_resource(const char *lang_id, const char *file);

/** \} */

/**
 * \defgroup i18n 国际化
 * <h1>国际化howto</h1>
 * 插件国际化的细节已被框架所处理，在加载插件时，自动加载相应的资源文件。下面举个例子并说明如何使插件支持国际化。
 * \par dissector_datatype中的例子
 * <p>dissector_datatype为插件的唯一名字，那么它对应会加载%config_root%/i18n/dissector_datatype.ini作为资源文件，
 * 放心，该文件并不是必须的，如果插件不会有与语言环境相关的字串，根本用不着该文件。</p>
 * <p>一旦提供该文件，那么必须保证该文件使用以下的格式书写：
 * \code
 [1]
 Other = 其它

 [3]
 Other = Other
 * \endcode
 * [1]与[3]为字串翻译段落，中文将使用1段，英文使用3段（在webuisetting中配置），"Other"为索引字串，"其它"为翻译字串，该文件将代码中所有
 * "Other"翻译为"其它"</p>
 *
 * <p>这很像原来的IDS方案，只不过现在的IDS直接使用了一个ini合法字串，而不是define在头文件中。</p>
 * \par 关于翻译
 * 调用i18n_gettext函数将进行翻译，(使用下划线`_'宏进行缩写)
 * 所有的翻译都储存在同一个地方，所以加载多个插件翻译文件将会产生冲突，
 * 目前不准备刻意去规避这种冲突，所以最好写代码时最好保证key是唯一的。
 * \see i18n_gettext
 * \note 一个好的习惯是编码时不要假定当前的系统语言。使用system_get_language取得当前语言（当前语言对你的插件有什么用？）
 */

#endif /* __LIBWNS_I18N_H__ */
