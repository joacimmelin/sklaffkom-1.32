#include "sklaff.h"

#ifdef LINUX
/* Simple strlcpy() */
size_t
strlcpy(char *dst, const char *src, size_t len)
{
    size_t n = len;
    char *d = dst;
    const char *sptr = src;

    if (n != 0) {
        while (--n != 0) {
            if ((*d++ = *sptr++) == '\0')
                break;
        }
    }
    if (n == 0) {
        if (len != 0) {
            *d = '\0';
            while (*sptr++) ;
        }
    }

    return sptr - src - 1;
}
#endif
