#include <semphr.h>
int increment_thread_safe(SemaphoreHandle_t lock, int *count, char *thread_name);
