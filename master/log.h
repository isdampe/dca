#ifndef LOG_H
#define LOG_H
#include <string.h>

#define DCA_LOG_MAX_STR_LEN 80
#define DCA_LOG_MAX_LINES 25

void log_append(char dest[][DCA_LOG_MAX_STR_LEN], const char *src);

#endif