#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>
#include "threads.h"

void side_thread(void *params)
{
	while (1) {
        vTaskDelay(100);
        int current_count;
        xSemaphoreTake(semaphore, 0xffff);
        {
            current_count = counter;
            counter = counter + 1;
            printf("hello world from %s! Count %d\n", "thread", current_count);
        }
        xSemaphoreGive(semaphore);
	}
}

void main_thread(void *params)
{
	while (1) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
        vTaskDelay(100);
        int current_count;
        xSemaphoreTake(semaphore, 0xffff);
        {
            current_count = counter++;
            printf("hello world from %s! Count %d\n", "main", current_count);
        }
        xSemaphoreGive(semaphore);
        on = !on;
	}
}

int main(void)
{
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);
    on = false;
    counter = 0;
    TaskHandle_t main, side;
    semaphore = xSemaphoreCreateCounting(1, 1);
    xTaskCreate(main_thread, "MainThread",
                MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, &main);
    xTaskCreate(side_thread, "SideThread",
                SIDE_TASK_STACK_SIZE, NULL, SIDE_TASK_PRIORITY, &side);
    vTaskStartScheduler();
	return 0;
}
