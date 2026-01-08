#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <memory>
#include <limits>
#include <cmath>

#include "allocator.h"
#include "cache.h"
#include "virtualmemory.h"

void printHeader(const std::string &title)
{
    std::cout << "\n--- " << title << " ---" << std::endl;
}

bool isPowerOfTwo(size_t n)
{
    return (n > 0) && ((n & (n - 1)) == 0);
}


void runAllocatorCLI()
{
    printHeader("Allocator Mode");
    std::cout << "[Cmds: init <size>, mode <first|best|worst>, malloc <size>, free <id>, stats, dump, back]" << std::endl;

    std::unique_ptr<Memory> mem = nullptr;
    std::string strategy = "first";
    std::string line;

    while (true)
    {
        std::cout << "allocator> ";
        if (!std::getline(std::cin, line))
            break;
        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "back")
            break;
        if (cmd == "exit")
            exit(0);

        if (cmd == "init")
        {
            size_t size;
            if (!(ss >> size))
            {
                std::cout << "Error: Usage 'init <size>'\n";
                continue;
            }
            mem = std::make_unique<Memory>(size);
            std::cout << "Memory initialized: " << size << " bytes.\n";
        }
        else if (cmd == "mode")
        {
            ss >> strategy;
            std::cout << "Strategy set to: " << strategy << "\n";
        }
        else if (cmd == "malloc")
        {
            if (!mem)
            {
                std::cout << "Error: Run 'init' first.\n";
                continue;
            }
            size_t size;
            ss >> size;
            int id = (strategy == "best") ? mem->allocate_bestfit(size) : (strategy == "worst") ? mem->allocate_worstfit(size)
                                                                                                : mem->allocate_firstfit(size);
            if (id != -1)
                std::cout << "Allocated block ID: " << id << "\n";
            else
                std::cout << "Allocation failed (Fragmentation/OOM).\n";
        }
        else if (cmd == "free")
        {
            if (!mem)
            {
                std::cout << "Error: Run 'init' first.\n";
                continue;
            }
            int id;
            ss >> id;
            mem->free(id);
            std::cout << "Freed ID " << id << ".\n";
        }
        else if (cmd == "stats")
        {
            if (!mem)
                continue;
            std::cout << "Total: " << mem->total_memory()
                      << " | Used: " << mem->used_memory()
                      << " | Success: " << mem->alloc_success_rate() << "%"
                      << " | Ext Frag: " << mem->ext_frag() * 100 << "%\n";
        }
        else if (cmd == "dump")
        {
            if (mem)
                mem->dump();
        }
        else
        {
            std::cout << "Unknown command.\n";
        }
    }
}

void runCacheCLI()
{
    printHeader("Cache Simulator");
    std::cout << "[Cmds: init <L1> <L2> <blk> <assoc> <pol>, access <addr> <0|1>, stats, dump, back]" << std::endl;

    std::unique_ptr<Cache> l1 = nullptr, l2 = nullptr;
    std::unique_ptr<cacheHierarchy> hierarchy = nullptr;
    std::string line;

    while (true)
    {
        std::cout << "cache> ";
        if (!std::getline(std::cin, line))
            break;
        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "back")
            break;
        if (cmd == "exit")
            exit(0);

        if (cmd == "init")
        {
            size_t s1, s2, bs;
            int as, pol;
            if (!(ss >> s1 >> s2 >> bs >> as >> pol))
            {
                std::cout << "Error: Invalid parameters.\n";
                continue;
            }
            bool valid = true;
            if (!isPowerOfTwo(s1) || !isPowerOfTwo(s2))
            {
                std::cout << "Error: Cache sizes must be powers of 2.\n";
                valid = false;
            }
            if (!isPowerOfTwo(bs))
            {
                std::cout << "Error: Block size must be a power of 2.\n";
                valid = false;
            }
            if (!isPowerOfTwo(as))
            {
                std::cout << "Error: Associativity must be a power of 2.\n";
                valid = false;
            }
            // Check: Associativity <= (Cache Size / Block Size)
            if (valid && (as > (s1 / bs) || as > (s2 / bs)))
            {
                std::cout << "Error: Associativity cannot exceed (CacheSize / BlockSize).\n";
                valid = false;
            }

            if (!valid)
                continue;

            l1 = std::make_unique<Cache>(s1, bs, as, (bool)pol);
            l2 = std::make_unique<Cache>(s2, bs, as, (bool)pol);
            hierarchy = std::make_unique<cacheHierarchy>(l1.get(), l2.get());
            std::cout << "Hierarchy Ready (" << (pol ? "LRU" : "FIFO") << ").\n";
        }
        else if (cmd == "access")
        {
            if (!hierarchy)
            {
                std::cout << "Error: Run 'init' first.\n";
                continue;
            }
            uint64_t addr;
            int type;
            if (!(ss >> addr >> type))
                continue;
            hierarchy->access(addr, (bool)type);
            std::cout << (type ? "WRITE" : "READ") << " @ Address " << addr << "\n";
        }
        else if (cmd == "dump")
        {
            if (l1)
            {
                std::cout << "--- L1 Cache ---";
                l1->dump();
            }
            if (l2)
            {
                std::cout << "--- L2 Cache ---";
                l2->dump();
            }
        }
        else if (cmd == "stats")
        {
            if (hierarchy)
                hierarchy->stats();
        }
        else
        {
            std::cout << "Unknown command.\n";
        }
    }
}

void runVirtualMemoryCLI()
{
    printHeader("Virtual Memory Simulator");
    std::cout << "[Cmds: init <ram> <pg> <pol>, access <pid> <addr>, status, back]" << std::endl;

    std::unique_ptr<PhysicalMemory> pm = nullptr;
    std::unordered_map<int, std::unique_ptr<VirtualMemory>> processes;
    std::string line;

    while (true)
    {
        std::cout << "vm> ";
        if (!std::getline(std::cin, line))
            break;
        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "back")
            break;
        if (cmd == "exit")
            exit(0);

        if (cmd == "init")
        {
            size_t rs, ps;
            int pol;
            if (!(ss >> rs >> ps >> pol))
            {
                std::cout << "Usage: init <ramSize> <pageSize> <0|1>\n";
                continue;
            }

            if (!isPowerOfTwo(rs))
            {
                std::cout << "Error: RAM size (" << rs << ") must be a power of 2.\n";
                continue;
            }
            if (!isPowerOfTwo(ps))
            {
                std::cout << "Error: Page size (" << ps << ") must be a power of 2.\n";
                continue;
            }
            // Check: RAM Size > Page Size
            if (rs <= ps)
            {
                std::cout << "Error: RAM size must be greater than Page size.\n";
                continue;
            }

            pm = std::make_unique<PhysicalMemory>(rs, ps, (bool)pol);
            processes.clear();
            std::cout << "Physical RAM ready (" << (pol ? "LRU" : "FIFO") << ").\n";
        }
        else if (cmd == "access")
        {
            if (!pm)
                continue;
            int pid;
            uint64_t vAddr;
            if (!(ss >> pid >> vAddr))
                continue;
            if (processes.find(pid) == processes.end())
                processes[pid] = std::make_unique<VirtualMemory>(pm.get(), pid);
            uint64_t pAddr = processes[pid]->translate(vAddr);
            std::cout << "PID " << pid << " | VA " << vAddr << " -> PA " << pAddr << "\n";
        }
        else if (cmd == "status")
        {
            if (pm)
                pm->printStatus();
        }
        else
        {
            std::cout << "Unknown command.\n";
        }
    }
}

void runIntegratedCLI()
{
    printHeader("Integrated Mode (VM + Cache)");
    std::cout << "[Cmds: init <ram> <pg> <l1> <l2> <blk> <asc>, access <pid> <addr> <0|1>, stats, dump, back]" << std::endl;

    std::unique_ptr<PhysicalMemory> pm = nullptr;
    std::unique_ptr<Cache> l1 = nullptr, l2 = nullptr;
    std::unique_ptr<cacheHierarchy> hierarchy = nullptr;
    std::unordered_map<int, std::unique_ptr<VirtualMemory>> processes;
    std::string line;

    while (true)
    {
        std::cout << "integrated> ";
        if (!std::getline(std::cin, line))
            break;
        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "back")
            break;
        if (cmd == "exit")
            exit(0);

        if (cmd == "init")
        {
            size_t rs, ps, l1s, l2s, bs;
            int as;
            if (!(ss >> rs >> ps >> l1s >> l2s >> bs >> as))
            {
                std::cout << "Usage: init <ramS> <pageS> <L1S> <L2S> <blockS> <assoc>\n";
                continue;
            }
            bool valid = true;

            // Check Powers of Two
            if (!isPowerOfTwo(rs))
            {
                std::cout << "Error: RAM size must be power of 2.\n";
                valid = false;
            }
            if (!isPowerOfTwo(ps))
            {
                std::cout << "Error: Page size must be power of 2.\n";
                valid = false;
            }
            if (!isPowerOfTwo(l1s))
            {
                std::cout << "Error: L1 size must be power of 2.\n";
                valid = false;
            }
            if (!isPowerOfTwo(l2s))
            {
                std::cout << "Error: L2 size must be power of 2.\n";
                valid = false;
            }
            if (!isPowerOfTwo(bs))
            {
                std::cout << "Error: Block size must be power of 2.\n";
                valid = false;
            }
            if (!isPowerOfTwo(as))
            {
                std::cout << "Error: Associativity must be power of 2.\n";
                valid = false;
            }

            // Functional Checks
            if (valid && rs <= ps)
            {
                std::cout << "Error: RAM size must be greater than Page size.\n";
                valid = false;
            }
            if (valid && (as > (l1s / bs) || as > (l2s / bs)))
            {
                std::cout << "Error: Associativity cannot exceed (CacheSize / BlockSize).\n";
                valid = false;
            }

            if (!valid)
                continue;

            pm = std::make_unique<PhysicalMemory>(rs, ps, true);
            l1 = std::make_unique<Cache>(l1s, bs, as, true);
            l2 = std::make_unique<Cache>(l2s, bs, as, true);
            hierarchy = std::make_unique<cacheHierarchy>(l1.get(), l2.get());
            processes.clear();
            std::cout << "System ready (LRU Policy).\n";
        }
        else if (cmd == "access")
        {
            if (!pm || !hierarchy)
                continue;
            int pid;
            uint64_t vAddr;
            int write;
            if (!(ss >> pid >> vAddr >> write))
                continue;
            if (processes.find(pid) == processes.end())
                processes[pid] = std::make_unique<VirtualMemory>(pm.get(), pid);

            uint64_t pAddr = processes[pid]->translate(vAddr);
            hierarchy->access(pAddr, (bool)write);
            std::cout << "Processed VA " << vAddr << " (" << (write ? "W" : "R") << ")\n";
        }
        else if (cmd == "dump")
        {
            if (l1)
            {
                std::cout << "\n[L1 DUMP]\n";
                l1->dump();
            }
            if (l2)
            {
                std::cout << "\n[L2 DUMP]\n";
                l2->dump();
            }
            if (pm)
            {
                std::cout << "\n[PHYSICAL RAM DUMP]\n";
                pm->printStatus();
            }
        }
        else if (cmd == "stats")
        {
            if (hierarchy)
                hierarchy->stats();
        }
        else
        {
            std::cout << "Unknown command.\n";
        }
    }
}

int main(int argc, char *argv[])
{
    // 1. File Redirection
    if (argc > 1)
    {
        static std::ifstream file(argv[1]);
        if (!file)
        {
            std::cerr << "Could not open file: " << argv[1] << std::endl;
            return 1;
        }
        std::cin.rdbuf(file.rdbuf());
    }

    while (true)
    {
        printHeader("Memory Management Simulator");
        std::cout << "[1. Allocator | 2. Cache | 3. VM | 4. Integrated | 5. Exit]\nChoice: ";

        int choice;
        if (!(std::cin >> choice))
        {
            if (std::cin.eof())
                break;

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice)
        {
        case 1:
            runAllocatorCLI();
            break;
        case 2:
            runCacheCLI();
            break;
        case 3:
            runVirtualMemoryCLI();
            break;
        case 4:
            runIntegratedCLI();
            break;
        case 5:
            return 0;
        default:
            std::cout << "Invalid.\n";
        }

        if (std::cin.eof())
            break;
    }
    return 0;
}