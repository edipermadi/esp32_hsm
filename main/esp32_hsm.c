#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <task_parser.h>

void app_main(void)
{
    task_parser_start();
}
