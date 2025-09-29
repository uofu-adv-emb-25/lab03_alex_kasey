#include <stdio.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <unity.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include "unity_config.h"
#include "../include/increment.h"

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

int main (void)
{
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);
    sleep_ms(5000); // Give time for TTY to attach.
    while(1) {
        printf("Start tests\n");
        UNITY_BEGIN();
        RUN_TEST(test_increment);
        RUN_TEST(test_timeout);
        sleep_ms(5000);
        UNITY_END();
    }
}
