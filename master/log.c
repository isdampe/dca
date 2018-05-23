#include <string.h>
#include "log.h"

void log_append(char dest[][DCA_LOG_MAX_STR_LEN], const char *src)
{
	if (strlen(src) > (DCA_LOG_MAX_STR_LEN -1))
		return;

	for (int i=0; i<DCA_LOG_MAX_LINES -1; ++i)
		strcpy(dest[i], dest[i +1]);
	
	strcpy(dest[DCA_LOG_MAX_LINES -1], src);
}