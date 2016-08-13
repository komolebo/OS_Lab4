#include "Interface.h"
#include <vector>
#include <map>
#include <Windows.h>		// Delay

using namespace std;

void defragment();
void outmem();
void output();

vector<Proc> Queue;
vector<Proc> CurProcesses;
vector<Proc> PerformedProcesses;
vector<Segm> Memory;
map<size_t, size_t> Table;

Settings settings;

size_t get_free_space()
{
	size_t free_space = settings.MEM_SIZE;

	for (auto & s : Memory)
		free_space-= s.size;

	return free_space;
}

vector<Segm>::iterator get_segm_by_addr(size_t address)
{
	for (vector<Segm>::iterator it = Memory.begin(); it != Memory.end(); ++it)
	{
		if (it->addr == address)
			return it;
	}

	return Memory.end();
}

size_t add_segment(size_t size)
{
	Segm s;
	s.size = size;

	// Find free address of new segment
	if (Memory.size() == 0)
	{
		s.addr = 0;
		Memory.push_back(s);

		return 0;
	}

	int i = -1;
	while (++i < (int)Memory.size() - 1)
	{
		if (i < (signed)Memory.size() && Memory[i].addr + Memory[i].size + size <= Memory[i + 1].addr)
			s.addr = Memory[i].addr + Memory[i].size;
	}

	if (Memory.size() == 1)
		s.addr = Memory[0].addr + Memory[0].size;
	else
		s.addr = Memory[i].addr + Memory[i].size;
	
	// Insert segment in memory
	vector<Segm>::iterator it = Memory.begin();
	advance(it, i + 1);
	Memory.insert(it, s);

	return s.addr;
}

void add_process(unsigned int tact, unsigned int & id)
{
	Proc p;
	p.ID = id++;
	p.time_join = tact;
	p.time_left = p.time_todo = rand() % settings.MAX_COMPLEXITY;
	p.segm_size = rand() % settings.MAX_SEGM_SIZE + 1;
	
	Queue.push_back(p);
}

void load_process(unsigned int & segNum, unsigned int tact)
{
	vector<Proc>::iterator it = Queue.begin();

	// Process can point onto existing segment
	if ((unsigned) rand() % 100 <= settings.EXISTING_SEGMENT_POSSIBL && !CurProcesses.empty())
		it->seg_number = CurProcesses[rand() % CurProcesses.size()].seg_number;
	else
	{
		it->seg_number = segNum++;
		Table[it->seg_number] = add_segment(it->segm_size);
	}

	it->time_load = tact;

	// Add in list
	CurProcesses.push_back(*it);
	Queue.erase(it);
}

void finish_process(unsigned int id, unsigned int & tact)
{
	for (vector<Proc>::iterator it = CurProcesses.begin(); it != CurProcesses.end(); ++it)
	{
		if (it->ID == id)
		{
			it->time_done = tact;

			// Free segment. Firstly if there's no other process using to same segment
			bool is_used_by_other = false;
			for (auto & p : CurProcesses)
			{
				if (p.ID != it->ID && p.seg_number == it->seg_number)
				{
					is_used_by_other = true;
					break;
				}
			}

			if (!is_used_by_other)
			{
				auto s = get_segm_by_addr(Table[it->seg_number]);

				Table.erase(it->seg_number);
				Memory.erase(s);
			}

			PerformedProcesses.push_back(*it);
			CurProcesses.erase(it);

			return;
		}
	}
}

void run()
{
	unsigned int tact = 0, ID = 0, segN = 0;
	// While there is task in ram or queue or is being planning to add
	while (settings.processes_to_run || !Queue.empty() || !CurProcesses.empty())
	{
		// Add task to queue
		if (settings.processes_to_run)
		{
			if ((unsigned)rand() % 100 <= settings.TASKS_POSSIBILITY)
			{
				add_process(tact, ID);
				settings.processes_to_run--;
			}
		}

		output();

		// Move first task from queue to RAM. Check if there is space for this task
		if (Queue.size() && Queue[0].segm_size <= get_free_space())
		{
			// Check if defragmentation is needed
			if (Memory.size() && Queue[0].segm_size > settings.MEM_SIZE - (Memory[Memory.size() - 1].addr + Memory[Memory.size() - 1].size))
				defragment();

			load_process(segN, tact);
		}

		// All current processes worked in RAM 1 tact
		for (unsigned int i = 0; i < CurProcesses.size(); i++)
		{
			if (CurProcesses[i].time_left <= 1)
			{
				finish_process(CurProcesses[i].ID, tact);
				i--;
			}
			else
				CurProcesses[i].time_left--;
		}

		tact++;
		Sleep(settings.DELAY);
	}
	output();
}

void defr(unsigned int i)
{
	if (i >= Memory.size()) return;

	if (i == 0)
	{
		for (auto & v : Table)
			if (v.second == Memory[0].addr)
			{
				Table[v.first] = Memory[0].addr = 0;
				break;
			}
	}
	else
		for (auto & v : Table)
			if (v.second == Memory[i].addr)
			{
				Table[v.first] = Memory[i].addr = Memory[i - 1].addr + Memory[i - 1].size;
				break;
			}

	return defr(i + 1);
}

void defragment()
{
	printf("\nDEFRAGMENTATION ..............................................\n");

	while (getchar() != 'q')
		continue;
	
	defr(0);

	output();
	printf("\nDEFRAGMENTATION COMPLETED ....................................\n");
	while (getchar() != 'q')
		continue;
}

void outmem()
{
	printf("Memory map. Address space = [%d..%d]: \n", 0, settings.MEM_SIZE - 1);
	for (Segm & s : Memory)
		printf("%4d..%4d \n", s.addr, s.addr + s.size - 1);
}

void output()
{
	system("cls");
	outmem();

	printf("-----------------------------------------------\n");
	printf("%5s %5s %5s %5s %5s %5s\n", "id", "join", "load", "all", "left", "size");
	for (Proc & p : CurProcesses)
		printf("%5d %5d %5d %5d %5d %5d\n", p.ID, p.time_join, p.time_load, p.time_todo, p.time_left, p.segm_size);

	printf("-----------------------------------------------\n");
	printf("Queue:\n");
	for (Proc & p: Queue)
		printf("%5d %5d %5d %5d %5d %5d\n", p.ID, p.time_join, p.time_load, p.time_todo, p.time_left, p.segm_size);

	printf("\n%22s %5d\n", "Performed processes: ", PerformedProcesses.size());
	printf("%22s %5d\n", "Free space: ", get_free_space());
	printf("%22s %5d\n", "Tasks will come: ", settings.processes_to_run);
}