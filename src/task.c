#include "../include/task.h"
#include "../include/function.h"
#include "../include/shell.h"
#include "../include/command.h"
#include "../include/builtin.h"

//global variable
int algorithm; //1:FCFS, 2:RR, 3:PP
ucontext_t to_run;
int task_num = 0;
long timer = 0;
struct task all_task[200];
int ready_queue[200];
int waiting_queue[200];
int wait_resource_queue[200];
int sleep_queue[200];
char task_name_array[200][100];
bool resource_array[20];
int ready_num = 0;
int waiting_num = 0;
int sleep_num = 0;
int wait_resource_num = 0;
int running = 0;
int now=0;
bool start_ = true;
bool idle_ = false;
bool pause_ = false;
bool resume = true;

void task_sleep(int ms)  //check
{   
    // while(!lock);
    // lock = false;
    int temp;
    temp = running;
    all_task[temp].sleep_time = ms;
    sleep_queue[sleep_num++] = running;
    waiting_queue[waiting_num++] = running;
    printf("Task %s goes to sleep.\n",task_name_array[temp]);
    getcontext(&(all_task[temp].ucp));
    all_task[running].state = 0;
    running = 0;
    //lock = true;
    swapcontext(&(all_task[temp].ucp), &to_run);
}

void task_exit()
{   
    // while(!lock);
    // lock = false;
    //printf("in");
    int temp;
    temp = running;
    all_task[running].turnaround_time = timer - all_task[running].start_time;
    printf("Task %s has terminated.\n",task_name_array[temp]);
    getcontext(&(all_task[temp].ucp));
    all_task[temp].state = 3;
    running = 0;
    //lock = true;
    swapcontext(&(all_task[temp].ucp), &to_run);
}

void get_resources(int count, int *resources)
{   
    //while(!lock);
    //lock = false;
    int temp;
    all_task[running].ok = true;
    all_task[running].resources = resources;
    all_task[running].count = count;
    for(int i=0; i<count; i++){
        if(resource_array[resources[i]] != true){
            all_task[running].ok = false;
            temp = running;
            wait_resource_queue[wait_resource_num++] = running;
            waiting_queue[waiting_num++] = running;
            printf("Task %s is waiting resource.\n",task_name_array[running]);
            getcontext(&(all_task[temp].ucp));
            all_task[running].state = 0;
            running = 0;
            break;
        }
    }
    if(!all_task[running].ok){
        //lock = true;
        swapcontext(&(all_task[temp].ucp), &to_run);
        get_resources(all_task[temp].count,all_task[temp].resources);
    }else{
        for(int i=0; i<count; i++){
            resource_array[resources[i]] = false;
            printf("Task %s gets resource %d.\n",task_name_array[running],resources[i]);
            //lock = true;
        }
    }
}

bool check_resource(int num){
    int count = all_task[wait_resource_queue[num]].count;
    int *res = all_task[wait_resource_queue[num]].resources;
    bool r;
    for(int i=0; i<count; i++){
        if(resource_array[res[i]] == true){
            r = true;
        }else{
            r = false;
            break;
        }
    }
    return r;
}

void release_resources(int count, int *resources)
{   
    // while(!lock);
    //lock = false;
    for(int i=0; i<count; i++){
        resource_array[resources[i]] = true;
        printf("Task %s releases resource %d.\n",task_name_array[running],resources[i]);
    }
    all_task[running].count = 0;
    all_task[running].ok = false;
    all_task[running].resources = NULL;
    //lock = true;
}

void get_algorithm(int number){
    algorithm = number;
}

void start_sim(void){
    ucontext_t main;
    if(start_){
        struct itimerval signal_timer,ovalue;
        for(int i=0; i<20; i++){
            resource_array[i] = true;
        }
        //initial timer
        signal(SIGVTALRM,set_timer); 
        signal_timer.it_value.tv_sec = 0;
        signal_timer.it_value.tv_usec = 10000;
        signal_timer.it_interval.tv_sec = 0;
        signal_timer.it_interval.tv_usec = 10000;
        signal(SIGTSTP,stop);
        setitimer(ITIMER_VIRTUAL,&signal_timer,&ovalue);
        printf("Start simulation.\n");
        create_context(&to_run,&main,&find_to_run);
        start_ = false;
        swapcontext(&main,&to_run);
    }else{
        printf("Start simulation.\n");
        pause_ = true;
    }
}

void set_timer(int single){
    int temp;
    int release[20];
    int flag=0;
    timer ++;
    //lock = false;
    //ready_queue(waiting_time)
    for(int i=0; i < ready_num ; i++){
        all_task[ready_queue[i]].waiting_time ++;
    }
    
    //waiting_queue(resources)
    for(int i=0; i<wait_resource_num; i++){
        if(check_resource(i)){
            release[flag++] = wait_resource_queue[i];
        }
    }
    for(int i=0; i<flag; i++){
        for(int j=0; j<waiting_num; j++){
            if(waiting_queue[j]==release[i]){
                out_queue(0,j);
                break;
            }
        }
        for(int j=0; j<wait_resource_num; j++){
            if(wait_resource_queue[j]==release[i]){
                out_queue(3,j);
                break;
            }
        }
        all_task[release[i]].state = 1;
        to_ready(release[i]);
    }
    flag = 0;
    //waiting_queue(sleep)
    for(int i=0; i < sleep_num ; i++){
        all_task[sleep_queue[i]].sleep_time --;
        if(all_task[sleep_queue[i]].sleep_time == 0){
            release[flag++] = sleep_queue[i];
        }
    }
    for(int i=0; i<flag; i++){
        for(int j=0; j<waiting_num; j++){
            if(waiting_queue[j]==release[i]){
                out_queue(0,j);
                break;
            }
        }
        for(int j=0; j<sleep_num; j++){
            if(sleep_queue[j]==release[i]){
                out_queue(2,j);
                break;
            }
        }
        all_task[release[i]].state = 1;
        to_ready(release[i]);
    }
    //running_time
    if(running!=0){
        all_task[running].running_time ++;
    }
    //lock = true;
    if(algorithm == 2){  
        if(running!=0){
            temp = running;
            if(all_task[running].running_time % 3 == 0 && all_task[running].running_time!=0){
                all_task[running].state = 1;
                to_ready(running);
                running = 0;
                //lock = true;
                swapcontext(&(all_task[temp].ucp), &to_run);
            }
        }
    }else if(algorithm == 3){
        if(running!=0 && ready_num > 0){
            temp = running;
            if(all_task[running].priority > all_task[ready_queue[0]].priority){
                all_task[running].state = 1;
                to_ready(running);
                running = 0;
                //lock = true;
                swapcontext(&(all_task[temp].ucp), &to_run);
            }
        }
    }
    signal(SIGVTALRM,set_timer);
}

void find_to_run(void){ 
    getcontext(&to_run);
    //while(!lock);
    //lock = false;
   if(ready_num == 0 && waiting_num != 0){
        if(!idle_){
            printf("CPU idle.\n");
            idle_ = true;
        }
        //lock = true;
        setcontext(&to_run);
    }else if(ready_num != 0){
        idle_ = false;
        running = ready_queue[0];
        now = running;
        out_queue(1,0);
        printf("Task %s is running.\n",task_name_array[running]);
        all_task[running].state = 2;
        setcontext(&all_task[running].ucp);
    }else if(ready_num == 0 && waiting_num == 0){
        printf("Simulation over.\n");
        start_ = true;
        //lock = true;
        if(pause_){
            pause_ = false;
            shell();
        }
        pause_ = false;
    }
}

void create_context(ucontext_t* context, ucontext_t* uc_link, void *function_name){
    getcontext(context);
    context->uc_stack.ss_sp = malloc(1024*128);
    context->uc_stack.ss_size = 1024*128;
    context->uc_stack.ss_flags = 0;
    context->uc_link = uc_link;
    makecontext(context,(void (*)(void))function_name,0);
}

void create_task(char *task_name, char *function_name, int priority){
    task_num ++;
    strcpy(task_name_array[task_num],task_name);
    all_task[task_num].tid = task_num;
    all_task[task_num].state = 1;
    all_task[task_num].running_time = 0;
    all_task[task_num].waiting_time = 0;
    all_task[task_num].turnaround_time = 0;
    all_task[task_num].resources = NULL;
    all_task[task_num].priority = priority;
    all_task[task_num].sleep_time = 0;
    all_task[task_num].ok = false;

    if(strcmp(function_name, "task1") == 0){
        create_context(&(all_task[task_num].ucp),&to_run,&task1);
    }else if(strcmp(function_name, "task2") == 0){
        create_context(&(all_task[task_num].ucp),&to_run,&task2);
    }else if(strcmp(function_name, "task3") == 0){
        create_context(&(all_task[task_num].ucp),&to_run,&task3);
    }else if(strcmp(function_name, "task4") == 0){
        create_context(&(all_task[task_num].ucp),&to_run,&task4);
    }else if(strcmp(function_name, "task5") == 0){
        create_context(&(all_task[task_num].ucp),&to_run,&task5);
    }else if(strcmp(function_name, "task6") == 0){
        create_context(&(all_task[task_num].ucp),&to_run,&task6);
    }else if(strcmp(function_name, "task7") == 0){
        create_context(&(all_task[task_num].ucp),&to_run,&task7);
    }else if(strcmp(function_name, "task8") == 0){
        create_context(&(all_task[task_num].ucp),&to_run,&task8);
    }else if(strcmp(function_name, "task9") == 0){
        create_context(&(all_task[task_num].ucp),&to_run,&task9);
    }else if(strcmp(function_name, "test_exit") == 0){
        create_context(&(all_task[task_num].ucp),&to_run,&test_exit);
    }else if(strcmp(function_name, "test_sleep") == 0){
        create_context(&(all_task[task_num].ucp),&to_run,&test_sleep);
    }else if(strcmp(function_name, "test_resource1") == 0){
        create_context(&(all_task[task_num].ucp),&to_run,&test_resource1);
    }else if(strcmp(function_name, "test_resource2") == 0){
        create_context(&(all_task[task_num].ucp),&to_run,&test_resource2);
    }
    
    all_task[task_num].start_time = timer;
    printf("Task %s is ready.\n",task_name_array[task_num]);
    to_ready(task_num);
}

void delete_task(char *task_name){  //running
    for(int i=1; i<=task_num; i++){
        if(strcmp(task_name_array[i],task_name)==0){
            all_task[i].turnaround_time = timer - all_task[i].start_time;
            printf("Task %s is killed.\n",task_name_array[i]);
            if(now == i && now!=0){
                if(all_task[now].ok){
                    for(int i=0; i<all_task[now].count; i++){
                        resource_array[all_task[now].resources[i]] = true;
                    }
                }
                now =0;
                running = 0;
                resume = false;
            }
            for(int j=0; j<waiting_num; j++){
                if(waiting_queue[j]==i){
                    out_queue(0,j);
                    break;
                }
            }
            for(int j=0; j<sleep_num; j++){
                if(sleep_queue[j]==i){
                    out_queue(2,j);
                    break;
                }
            }
            for(int j=0; j<wait_resource_num; j++){
                if(wait_resource_queue[j]==i){
                    out_queue(3,j);
                    break;
                }
            }
            for(int j=0; j<ready_num; j++){
                if(ready_queue[j]==i){
                    out_queue(1,j);
                    break;
                }
            }
            all_task[i].state = 3;
            break;
        }
    }
}

void ps_show(void){  //resource
    char s[15];
    char t[10];
    char r[30];
    char p[10];
    char r_w[5];
    printf("TID|   name|       state| running| waiting| turnaround| resources| priority\n");
    printf("---------------------------------------------------------------------------\n");
    for(int i=1; i<=task_num; i++){
        switch (all_task[i].state)
        {
        case 0:
            strcpy(s,"WAITING");
            strcpy(t,"none");
            break;
        case 1:
            strcpy(s,"READY");
            strcpy(t,"none");
            break;
        case 2:
            strcpy(s,"RUNNING");
            strcpy(t,"none");
            break;
        case 3:
            strcpy(s,"TERMINATED");
            sprintf(t,"%ld",all_task[i].turnaround_time);
            break;
        default:
            break;
        }
        if(algorithm == 3){
            sprintf(p,"%d",all_task[i].priority);
        }else if(algorithm == 1 || algorithm == 2){
            strcpy(p,"*");
        }
        if(all_task[i].ok){
            for(int j=0; j<all_task[i].count; j++){
                sprintf(r_w,"%d",all_task[i].resources[j]);
                strcat(r,r_w);
                strcat(r,",");
            }
        }else{
            strcpy(r,"none");
        }
        printf("%3d|%7s|%12s|%8ld|%8ld|%11s|%10s|%9s\n",i,task_name_array[i],s,all_task[i].running_time,all_task[i].waiting_time,t,r,p);
        strcpy(r,"\0");
    }
}

void to_ready(int ready_task){
    if(algorithm == 3){
        int index;
        if(ready_num == 0){
            ready_queue[0] = ready_task;
        }else{
            for(index=0; index<ready_num; index++){
                if(all_task[ready_task].priority < all_task[ready_queue[index]].priority){
                    break;
                }
            }
            for(int i=ready_num; i > index; i--){
                ready_queue[i] = ready_queue[i-1];
            }
            ready_queue[index] = ready_task;
        }
        ready_num ++;
    }else if (algorithm == 1 || algorithm == 2){
        ready_queue[ready_num] = ready_task;
        ready_num ++;
    }
}

void out_queue(int a, int b){
    if (a==0){
        waiting_num -- ;
        for(int i=b; i < waiting_num; i++){
            waiting_queue[i] = waiting_queue[i+1];
        }
    }else if(a==1){
        ready_num -- ;
        for(int i=b; i < ready_num; i++){
            ready_queue[i] = ready_queue[i+1];
        }
    }else if(a==2){
        sleep_num -- ;
        for(int i=b; i < sleep_num; i++){
            sleep_queue[i] = sleep_queue[i+1];
        }
    }else if(a==3){
        wait_resource_num -- ;
        for(int i=b; i < wait_resource_num; i++){
            wait_resource_queue[i] = wait_resource_queue[i+1];
        }
    }
}

void stop(void){
    pause_ = false;
    shell();
}

bool get_can(void){
    return pause_;
}