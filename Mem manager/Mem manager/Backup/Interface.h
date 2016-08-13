struct Segm
{
	size_t addr, size;
};

struct Settings
{
	const unsigned int POSSIBL = 10;
	size_t max_segm_size = 100;
	size_t mem_size = 1024;
	const unsigned int delay = 1;
	int i = 5;		// every i-th process uses existed segment
};

void add_process();
void del_process();

void previous_load();
void Run();
void show();