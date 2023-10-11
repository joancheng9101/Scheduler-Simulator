#ifndef TASK_H
#define TASK_H

#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

struct task
{  
   int tid;
   int state; //0:waiting,1:ready,2:running,3:terminated
   long running_time;
   long waiting_time;
   long start_time;
   long turnaround_time;
   int *resources;
   int priority;
   int sleep_time;
   bool ok;
   int count;
   ucontext_t ucp;
};

void task_sleep(int);
void task_exit();
void get_algorithm(int number);
void start_sim(void);
void set_timer(int single);
void find_to_run(void);
void create_context(ucontext_t* context, ucontext_t* uc_link, void *function_name);
void create_task(char *task_name, char *function_name, int priority);
void delete_task(char *task_name);
void ps_show(void);
void to_ready(int ready_task);
void out_queue(int a, int b);
void get_resources(int, int *);
void release_resources(int, int *);
bool check_resource(int num);
void stop(void);
#endif
