#include "include/shell.h"
#include "include/command.h"
#include "include/task.h"

int main(int argc, char *argv[])
{
	history_count = 0;
	for (int i = 0; i < MAX_RECORD_NUM; ++i)
    	history[i] = (char *)malloc(BUF_SIZE * sizeof(char));
		
	if(strcmp(argv[1], "FCFS") == 0){
		get_algorithm(1);
	}else if(strcmp(argv[1], "RR") == 0){
		get_algorithm(2);
	}else if(strcmp(argv[1], "PP") == 0){
		get_algorithm(3);
	}

	shell();

	for (int i = 0; i < MAX_RECORD_NUM; ++i)
    	free(history[i]);

	return 0;
}
