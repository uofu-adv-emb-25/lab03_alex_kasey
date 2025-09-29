#include <semphr.h>

struct DeadlockArgs
{
    SemaphoreHandle_t first_lock;
    SemaphoreHandle_t second_lock;
    int count;
    char *thread_name;
};

void deadlock_thread(void *);

int increment_thread_safe(SemaphoreHandle_t lock, int *count, char *thread_name);
