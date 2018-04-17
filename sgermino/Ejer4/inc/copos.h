#pragma once
#include <stdint.h>


/* The maximum number of tasks required at any one time during the execution
   of the program. MUST BE ADJUSTED FOR EACH NEW PROJECT */
#ifndef SCHEDULER_MAX_TASKS
   #define SCHEDULER_MAX_TASKS   (15)
#endif


typedef struct
{
    void (* pTask)(void *ctx, uint32_t ticks);
    void *context;
    // Delay (ticks) until the function will (next) be run
    // - see schedulerAddTask() for further details
    int32_t delay;
    // Interval (ticks) between subsequent runs.
    // - see schedulerAddTask() for further details
    int32_t period;
    // Incremented (by scheduler) when task is due to execute
    int32_t runMe;
}
sTask_t;


void        schedulerInit           (void);
void        schedulerStart          (uint32_t);
void        schedulerUpdate         (uint32_t ticks);
void        schedulerDispatchTasks  (uint32_t ticks);
uint32_t    schedulerAddTask        (void (* pFunction)(void *ctx,
                                                        uint32_t ticks),
                                     void *context, uint32_t delay,
                                     uint32_t period);
int8_t      schedulerModifyTaskPeriod (uint32_t taskIndex, uint32_t newPeriod);
int8_t      schedulerDeleteTask     (uint32_t taskIndex);
void        schedulerReportStatus   (void);
