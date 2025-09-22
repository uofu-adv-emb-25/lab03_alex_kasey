#include <stdio.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>
#include "../include/increment.h"

int increment_thread_safe(SemaphoreHandle_t lock, int *count, char *thread_name)
{
    if(xSemaphoreTake(lock, 0xffff) == pdTRUE)
    {
        *count = *count + 1;
        printf("hello world from %s! Count %d\n", thread_name, *count);
        xSemaphoreGive(lock);
        return pdTRUE;
    } else {
        return pdFALSE;
    }
}