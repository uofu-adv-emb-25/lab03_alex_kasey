#include <semphr.h>

struct DeadlockArgs
{
    SemaphoreHandle_t first_lock;
    SemaphoreHandle_t second_lock;
    int count;
    char *thread_name;
};

struct OrphanedArgs
{
    SemaphoreHandle_t lock;
    int count;
};

void deadlock_thread(void *);
void orphaned_lock(void *args);
void orphaned_lock_fixed(void *args);

int increment_thread_safe(SemaphoreHandle_t lock, int *count, char *thread_name);
