#ifndef __LIBWNS_ATOM_H__
#define __LIBWNS_ATOM_H__

#include "libbase_config.h"

#ifdef SF_USE_PORTABLE_ATOMIC
/* 下面是跨平台版本 */
#include "libbase_atomic_portable.h"
#else

#ifdef SF_ARCH_I686
#include "libbase_atomic_i686.h"
#endif

#ifdef SF_ARCH_X86_64
#include "libbase_atomic_x86_64.h"
#endif

#ifdef SF_ARCH_MIPS_32
#include "libbase_atomic_mips32.h"
#endif 

#endif /* SF_USE_PORTABLE_ATOMIC */

#endif /* __LIBWNS_ATOM_H__ */

