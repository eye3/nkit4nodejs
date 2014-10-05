#ifndef NKIT_NETBSD_STRPTIME_H
#define NKIT_NETBSD_STRPTIME_H 1

#include <ctime>

namespace nkit
{
#if defined(NKIT_WINNT)
	char * strptime (const char *buf, const char *fmt, struct ::tm *timeptr);
#endif
} // namespace nkit

#endif // NKIT_NETBSD_STRPTIME_H
