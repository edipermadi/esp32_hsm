//
// Created by edi on 9/19/24.
//

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "task_parser.h"

static TaskHandle_t task_parser_handle = NULL;
static void task_parser(void* pvParameters);

void task_parser_start()
{
    xTaskCreate(task_parser, "task_parser", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &task_parser_handle);
}

void task_parser_stop()
{
}

static void task_parser(void* pvParameters)
{
    for (;;)
    {
        printf("task parser here\n");
        vTaskDelay(100);
    }
}
