#include <stdio.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>
#include <unity.h>
#include "unity_config.h"
#include "../include/functions.h"

#define HIGHEST_TASK_PRIORITY      ( tskIDLE_PRIORITY + 5UL )
#define HIGHEST_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

void setUp(void) {}

void tearDown(void) {}

void test_timeout(void)
{
    int count = 0;
    SemaphoreHandle_t semaphore = xSemaphoreCreateCounting(1, 1);
    xSemaphoreTake(semaphore, 0xffff);
    TEST_ASSERT_EQUAL_MESSAGE(pdFALSE, increment_thread_safe(semaphore, &count, "test_timeout"), "Semaphore should lock and return false.");
}

void test_increment(void)
{
    int count = 0;
    int past_count = 0;
    SemaphoreHandle_t semaphore = xSemaphoreCreateCounting(1, 1);
    for(int i = 0; i < 10; i++) {
        increment_thread_safe(semaphore, &count, "test_increment");
        TEST_ASSERT_EQUAL_MESSAGE(past_count + 1, count, "Count value should increment by 1 each time.");
        past_count = count;
    }
}

void test_deadlock(void)
{
    // Create thread pointers
    TaskHandle_t thread_a, thread_b;
    // Create locks
    SemaphoreHandle_t lock_a = xSemaphoreCreateCounting(1, 1);
    SemaphoreHandle_t lock_b = xSemaphoreCreateCounting(1, 1);

    // Create thread arguments
    struct DeadlockArgs thread_a_args = {lock_a, lock_b, 0, "Thread A"};
    struct DeadlockArgs thread_b_args = {lock_b, lock_a, 50, "Thread B"};
    
    // Create threads
    xTaskCreate(deadlock_thread, "Thread A", configMINIMAL_STACK_SIZE, (void *)&thread_a_args, tskIDLE_PRIORITY + 1UL, &thread_a);
    xTaskCreate(deadlock_thread, "Thread B", configMINIMAL_STACK_SIZE, (void *)&thread_b_args, tskIDLE_PRIORITY + 1UL, &thread_b);

    printf("Created threads, starting deadlock.\r\n");
    vTaskDelay(1000);
    // Check status of locks, both should be 0
    TEST_ASSERT_EQUAL_MESSAGE(0, uxSemaphoreGetCount(lock_a), "lock_a should be acquired by thread_a");
    TEST_ASSERT_EQUAL_MESSAGE(0, uxSemaphoreGetCount(lock_b), "lock_b should be acquired by thread_b");
    // Check status of count in each thread, both should be incremented by only 1
    TEST_ASSERT_EQUAL_MESSAGE(1, thread_a_args.count, "thread_a count should be incremented by 1.");
    TEST_ASSERT_EQUAL_MESSAGE(51, thread_b_args.count, "thread_b count should be incremented by 1.");
    // Delete tasks
    printf("Done with deadlocking threads, killing them.\r\n");
    vTaskDelete(thread_a);
    vTaskDelete(thread_b);
    printf("Deadlocking threads deleted.\r\n");
}

void test_orphaned(void)
{
    TaskHandle_t orphaned_thread;
    SemaphoreHandle_t lock = xSemaphoreCreateCounting(1, 1);
    struct OrphanedArgs thread_args = {lock, 0};

    xTaskCreate(orphaned_lock, "Orphaned Thread", configMINIMAL_STACK_SIZE, (void *)&thread_args, tskIDLE_PRIORITY + 1UL, &orphaned_thread);
    printf("Created/Started orphaned thread.\r\n");
    vTaskDelay(1000);
    // Check that orphaned thread is locked
    TEST_ASSERT_EQUAL_MESSAGE(0, uxSemaphoreGetCount(lock), "lock should be acquired by orphaned lock");

    // Check orphaned count value
    TEST_ASSERT_EQUAL_MESSAGE(1, thread_args.count, "lock should be orphaned after first execution.");

    // Delete task
    printf("Deleting orphaned lock.\r\n");
    vTaskDelete(orphaned_thread);
    printf("Orphaned thread deleted.\r\n");
}

void test_orphaned_fixed(void)
{
    TaskHandle_t not_orphaned_thread;
    SemaphoreHandle_t lock = xSemaphoreCreateCounting(1, 1);
    struct OrphanedArgs thread_args = {lock, 0};

    xTaskCreate(orphaned_lock_fixed, "Not Orphaned Thread", configMINIMAL_STACK_SIZE, (void *)&thread_args, tskIDLE_PRIORITY + 1UL, &not_orphaned_thread);
    printf("Created/Started not orphaned thread.\r\n");
    vTaskDelay(10);
    // Check that orphaned thread is not locked
    TEST_ASSERT_EQUAL_MESSAGE(eReady, eTaskGetState(not_orphaned_thread), "Not Orphaned Thread should be ready.");

    // Be sure to lock so not orphaned thread suspends
    xSemaphoreTake(lock, 0xffff);
    // Check orphaned count value
    TEST_ASSERT_TRUE_MESSAGE(thread_args.count > 1, "Not Orphaned Thread should increment count past 1.");

    // Delete task
    printf("Deleting not orphaned lock.\r\n");
    vTaskDelete(not_orphaned_thread);
    printf("Not Orphaned thread deleted.\r\n");
}

void runner(__unused void *args)
{
    sleep_ms(10000);
    while(1) {
        printf("Start tests\n");
        UNITY_BEGIN();
        RUN_TEST(test_increment);
        RUN_TEST(test_timeout);
        RUN_TEST(test_deadlock);
        RUN_TEST(test_orphaned);
        RUN_TEST(test_orphaned_fixed);

        UNITY_END();
        sleep_ms(5000);
    }
}

int main (void)
{
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);
    sleep_ms(1000); // Give time for TTY to attach.
    xTaskCreate(runner, "TestRunner",
                configMINIMAL_STACK_SIZE, NULL, HIGHEST_TASK_PRIORITY, NULL);
    vTaskStartScheduler();
    return 0;
}
