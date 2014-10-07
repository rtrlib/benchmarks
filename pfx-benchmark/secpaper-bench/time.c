#include <sys/timeb.h>

unsigned int get_msec_time (
        unsigned int result;
        struct timeb tp;
        ftime(&tp);
        result = tp
