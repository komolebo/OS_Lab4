struct Segm
{
	size_t addr, size;
};

struct Proc
{
	unsigned int ID;
	int time_load = -1;
	int time_done = -1;
	int seg_number = -1;
	size_t time_join;
	size_t time_todo;
	size_t time_left;

	size_t segm_size;
};

struct Settings
{
	const unsigned int TASKS_POSSIBILITY = 100; //100
	const unsigned int EXISTING_SEGMENT_POSSIBL = 20;		// every i-th process uses existed segment
	const unsigned int DELAY = 1000; //1000
	const size_t MAX_SEGM_SIZE = 20;
	const size_t MAX_COMPLEXITY = 20;						// time for process to work on
	const size_t MEM_SIZE = 128;

	size_t processes_to_run = 500;
};

void run();