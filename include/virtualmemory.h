#pragma once
#include <vector>
#include <list>
#include <unordered_map>
#include <cstdint>
#include <iostream>
#include <cmath>
class VirtualMemory;

class PhysicalMemory
{
public:
    PhysicalMemory(size_t ramSize, size_t pageSize, bool useLRU);
    void registerProcess(int pid, VirtualMemory *vm);

    void access(int frameIndex);
    int allocate(int ownerPID, uint64_t ownerVPN);
    void printStatus();
    size_t pagesize;
    uint64_t pageFaults = 0;
    uint64_t pageHits = 0;

private:
    struct FrameInfo
    {
        bool isUsed = false;
        uint64_t ownerVPN = 0;
        int ownerPID = -1;
    };

    size_t numFrames;
    bool isLRU;

    std::vector<FrameInfo> frames;
    std::unordered_map<int, VirtualMemory *> processRegistry;

    size_t nextFreeFrameIndex = 0;
    bool isFull = false;
    int fifoPointer = 0;
    std::list<int> lruList;
    std::unordered_map<int, std::list<int>::iterator> lruMap;
};

class VirtualMemory
{
public:
    VirtualMemory(PhysicalMemory *pm, int pid);

    VirtualMemory(PhysicalMemory *pm);

    uint64_t translate(uint64_t virtualAddr);

    void invalidatePage(uint64_t vpn);

private:
    struct PageTableEntry
    {
        bool valid = false;
        int frameNumber = -1;
    };

    PhysicalMemory *physMem;
    int pid;
    size_t pageSize;
    int offsetBits;

    std::unordered_map<uint64_t, PageTableEntry> pageTable;
};