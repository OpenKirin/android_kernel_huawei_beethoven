#ifndef __CHR_USER_H__
#define __CHR_USER_H__

#ifdef __cpluscplus
	#if __cplusplus
	extern "C" {
	#endif
#endif

/*
 * 1 Other Include Head File
 */
#include "chr_errno.h"

typedef enum chr_LogPriority{
	CHR_LOG_DEBUG = 0,
	CHR_LOG_INFO,
	CHR_LOG_WARN,
	CHR_LOG_ERROR,
}CHR_LOGPRIORITY;

typedef enum chr_LogTag{
	CHR_LOG_TAG_PLAT = 0,
	CHR_LOG_TAG_WIFI,
	CHR_LOG_TAG_GNSS,
	CHR_LOG_TAG_BT,
	CHR_LOG_TAG_FM,
	CHR_LOG_TAG_NFC,
}CHR_LOG_TAG;

extern int __chr_printLog(CHR_LOGPRIORITY prio, CHR_LOG_TAG tag, const char *fmt,...);
extern int __chr_exception(unsigned int errno);
extern void chr_dev_exception_callback(void *buff, unsigned short len);

#define CHR_LOG(prio, tag, fmt...)                  __chr_printLog(prio, tag, ##fmt)
#define CHR_EXCEPTION(errno)                        __chr_exception(errno)

#ifdef __cpluscplus
	#if __cplusplus
		}
	#endif
#endif

#endif
