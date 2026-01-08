#include "virtualmemory.h"
#include <iomanip>

PhysicalMemory::PhysicalMemory(size_t ramSize, size_t pageSize, bool useLRU_)
    :  pagesize(pageSize),isLRU(useLRU_)
{
    numFrames = ramSize / pagesize;
    frames.resize(numFrames);
}

void PhysicalMemory::registerProcess(int pid, VirtualMemory *vm)
{
    processRegistry[pid] = vm;
}

void PhysicalMemory::access(int frameIndex)
{
    if (!isLRU)
        return;
    auto it = lruMap.find(frameIndex);
    if (it != lruMap.end())
    {
        lruList.splice(lruList.begin(), lruList, it->second);
    }
}

int PhysicalMemory::allocate(int ownerPID, uint64_t ownerVPN)
{
    int targetFrame = -1;
    if (!isFull)
    {
        targetFrame = nextFreeFrameIndex;
        nextFreeFrameIndex++;
        if (nextFreeFrameIndex == numFrames)
            isFull = true;

        if (isLRU)
        {
            lruList.push_front(targetFrame);
            lruMap[targetFrame] = lruList.begin();
        }
    }
    else
    {
        if (!isLRU)
        {
            targetFrame = fifoPointer;
            fifoPointer = (fifoPointer + 1) % numFrames;
        }
        else
        {
            targetFrame = lruList.back();
            lruList.pop_back();
            lruMap.erase(targetFrame);
            lruList.push_front(targetFrame);
            lruMap[targetFrame] = lruList.begin();
        }
        if (frames[targetFrame].isUsed)
        {
            int victimPID = frames[targetFrame].ownerPID;
            uint64_t victimVPN = frames[targetFrame].ownerVPN;
            if (processRegistry.find(victimPID) != processRegistry.end())
            {
                processRegistry[victimPID]->invalidatePage(victimVPN);
            }
        }
    }

    frames[targetFrame].isUsed = true;
    frames[targetFrame].ownerVPN = ownerVPN;
    frames[targetFrame].ownerPID = ownerPID;

    return targetFrame;
}

void PhysicalMemory::printStatus()
{
    std::cout << "--- Physical RAM Status ---\n";
    std::cout << "Frames Used: " << (isFull ? numFrames : nextFreeFrameIndex) << "/" << numFrames << "\n";
    for (size_t i = 0; i < numFrames; ++i)
    {
        if (frames[i].isUsed)
            std::cout << " Frame " << i << ": PID " << frames[i].ownerPID << " (VPN " << frames[i].ownerVPN << ")\n";
    }
    std::cout << "Hits: " << pageHits << " | Faults: " << pageFaults << "\n";
}

VirtualMemory::VirtualMemory(PhysicalMemory *pm, int pid_)
    : physMem(pm), pid(pid_)
{
    offsetBits = std::log2(physMem->pagesize);
    physMem->registerProcess(pid, this);
}

// if considering one process for simplicity
VirtualMemory::VirtualMemory(PhysicalMemory *pm)
    : physMem(pm), pid(1)
{
    offsetBits = std::log2(physMem->pagesize);
    physMem->registerProcess(pid, this);
}

void VirtualMemory::invalidatePage(uint64_t vpn)
{
    if (pageTable.count(vpn))
    {
        pageTable[vpn].valid = false;
        pageTable[vpn].frameNumber = -1;
    }
}

uint64_t VirtualMemory::translate(uint64_t virtualAddr)
{
    uint64_t vpn = virtualAddr >> offsetBits;
    uint64_t offset = virtualAddr & ((1ULL << offsetBits) - 1);
    if (pageTable[vpn].valid)
    {
        physMem->pageHits++;
        int frame = pageTable[vpn].frameNumber;
        physMem->access(frame);
        return (frame << offsetBits) | offset;
    }
    physMem->pageFaults++;
    int newFrame = physMem->allocate(pid, vpn);

    pageTable[vpn].valid = true;
    pageTable[vpn].frameNumber = newFrame;

    return (newFrame << offsetBits) | offset;
}