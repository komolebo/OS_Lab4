#include "Interface.h"
#include <map>
#include <list>
#include <vector>
#include <mutex>		// For synchronysing user input and queue workers
#include <Windows.h>

using namespace std;

void defragment();

Settings settings;
mutex m;

map<size_t, size_t> Table;
list<size_t> Queue;
list<pair<size_t, size_t>> CurProcesses;
vector<Segm> Memory;

void add_process()
{
	size_t size = rand() % settings.max_segm_size;
	Queue.push_back(size);
}

void del_process()
{
	if (!CurProcesses.size()) return;

	while (m.try_lock() == false) Sleep(settings.delay);

	// Select random process
	int i = rand() % CurProcesses.size();
	auto p = CurProcesses.begin();
	advance(p, i);
	printf("Deleted addr=%d\n", Table[p->first]);

	// If two or more processes use same segment, then only one process must be deleted
	int count = 0;
	for (auto & proc : CurProcesses)
		if (proc.first == p->first)
			count++;
	if (count > 1)
	{
		CurProcesses.erase(p);
		m.unlock();
		return;
	}

	// Only one process uses this segment. Finding this segment
	for (auto it = Memory.begin(); it != Memory.end(); it++)
		if ((*it).addr == Table[p->first])
		{
			Memory.erase(it);
			Table.erase(p->first);
			CurProcesses.erase(p);
			break;
		}
	m.unlock();
}

void Run()
{
	while (true)
	{
		if (!Queue.size()) continue;

		size_t size = Queue.front();

		int seg_number = 0;
		size_t shift = 0;

		if (rand() % 5 == 0 && settings.i-- > 0) // Segment exists in RAM
		{
			auto n = Table.begin();
			advance(n, rand() % Table.size());
			seg_number = n->first;
			shift = n->second;
		}
		else						  // Create new segment for process
		{
			// Find segment number
			while (Table.find(seg_number) != Table.end()) seg_number++;

			// Find free address
			int i = -1;
			size_t addr0 = 0;

			while (++i < (int)Memory.size())
			{
				if (Memory[i].addr >= addr0 + size)
					break;
				else
					addr0 = Memory[i].addr + Memory[i].size;
			}

			// Check if space is found
			if (addr0 + size > settings.mem_size || !addr0 && Memory.size() && !Memory[0].addr)
			{
				if (m.try_lock())
				{
					defragment();
					m.unlock();
				}
				Sleep(settings.delay);
				continue;
			}

			Table[seg_number] = addr0;

			// Put in right place of memory
			Segm s;
			s.size = size;
			s.addr = addr0;

			auto it = Memory.begin();
			advance(it, i);
			Memory.insert(it, s);
		}

		CurProcesses.push_back(pair<size_t, size_t>(seg_number, rand() % 10));
		Queue.pop_front();
		output();
		Sleep(settings.delay);
	}
}

void defr(unsigned int i)
{
	if (i >= Memory.size()) return;

	Segm * segm = &Memory[i];

	if (i == 0)
	{
		if (segm->addr != 0)
		{
			// Find segment id
			int seg_number;
			for (auto & rec : Table)
				if (rec.second == segm->addr)
					seg_number = rec.first;
			
			Table[seg_number] = segm->addr = 0;
		}
	}
	else
	{
		size_t new_addr = Memory[i - 1].addr + Memory[i - 1].size;

		if (segm->addr != new_addr)
		{
			// Find segment id
			size_t seg_number;
			for (auto & rec : Table)
				if (rec.second == segm->addr)
				{
					seg_number = rec.first;
					break;
				}
			Table[seg_number] = segm->addr = new_addr;
		}
	}

	return defr(i + 1);
}

void defragment()
{
	defr(0);
}

/*void output()
{
	for (auto & s : Memory)
		printf("%8d  ..%8d \n", s.addr, s.addr + s.size - 1);
}*/

void show()
{
	system("cls");
	printf("%5s %5s", "Seg N", "Time left");

	for (auto & p : CurProcesses)
		printf("%5d %5d\n", p.first, p.second);

	printf("\n Queue: %d,   Segm: %d, Processes: %d \n", Queue.size(), Memory.size(), CurProcesses.size());
}