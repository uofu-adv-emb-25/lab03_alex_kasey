#include <stdio.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>
#include "../include/functions.h"

int increment_thread_safe(SemaphoreHandle_t lock, int *count, char *thread_name)
{
    if(xSemaphoreTake(lock, 0xfff) == pdTRUE)
    {
        *count = *count + 1;
        printf("hello world from %s! Count %d\n", thread_name, *count);
        xSemaphoreGive(lock);
        return pdTRUE;
    } else {
        return pdFALSE;
    }
}

void deadlock_thread(void *args)
{
    // Extract the arguments from the calling task
    struct DeadlockArgs *arguments = (struct DeadlockArgs *)args;
    printf("Starting deadlock from thread: %s\r\n", arguments->thread_name);
    // Acquire first lock
    xSemaphoreTake(arguments->first_lock, portMAX_DELAY);
    {
        arguments->count++;
        printf("Inside first lock from thread: %s\r\n", arguments->thread_name);
        vTaskDelay(1000);
        // Acquire second lock
        xSemaphoreTake(arguments->second_lock, portMAX_DELAY);
        {
            arguments->count++;
            printf("Inside second lock from thread: %s\r\n", arguments->thread_name);
        }
        // Give back second lock
        xSemaphoreGive(arguments->second_lock);
    }
    // Give back first lock
    xSemaphoreGive(arguments->first_lock);
    // Suspend self to avoid segfault
    vTaskSuspend(NULL);
}

void orphaned_lock(void *args)
{
    // Extract arguments
    struct OrphanedArgs *arguments = (struct OrphanedArgs *)args;
    printf("Starting Oprhaned Thread with count: %d\r\n", arguments->count);
    while (1) {
        xSemaphoreTake(arguments->lock, portMAX_DELAY);
        arguments->count++;
        if (arguments->count % 2) {
            continue;
        }
        printf("Count %d\n", arguments->count);
        xSemaphoreGive(arguments->lock);
    }
}

void orphaned_lock_fixed(void *args)
{
    // Extract arguments
    struct OrphanedArgs *arguments = (struct OrphanedArgs *)args;
    printf("Starting Oprhaned Thread with count: %d\r\n", arguments->count);
    while (1) {
        xSemaphoreTake(arguments->lock, portMAX_DELAY);
        arguments->count++;
        if (arguments->count % 2) {
            // Be sure to give lock back before continuing
            xSemaphoreGive(arguments->lock);
            continue;
        }
        printf("Count %d\n", arguments->count);
        xSemaphoreGive(arguments->lock);
    }
}