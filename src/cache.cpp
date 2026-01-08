#include "cache.h"
#include <cmath>
#include <iostream>
#include <iomanip>

Cache::Cache(size_t cSize, size_t bSize, int assoc, bool useLRU_)
    : blockSize(bSize), associativity(assoc), isLRU(useLRU_)
{
    numSets = cSize / (blockSize * associativity);
    // calculate bits for decoding (assuming powers of 2)
    offsetBits = std::log2(blockSize);
    indexBits = std::log2(numSets);
    // initialize memory
    sets.resize(numSets);
    for (auto &set : sets)
    {
        set.lines.resize(associativity);
    }
    if (associativity > 1)
    {
        if (isLRU)
        {
            initLRU();
        }
        else
        {
            initFIFO();
        }
    }
}

Cache::Cache(size_t cSize, size_t bSize, int assoc)
    : blockSize(bSize), associativity(assoc), isLRU(false)
{
    numSets = cSize / (blockSize * associativity);
    offsetBits = std::log2(blockSize);
    indexBits = std::log2(numSets);
    // Initialize Memory
    sets.resize(numSets);
    for (auto &set : sets)
    {
        set.lines.resize(associativity);
    }
    if (associativity > 1)
    {
        initFIFO();
    }
}

void Cache::initFIFO()
{
    fifoNextVictim.resize(numSets, 0);
}

void Cache::initLRU()
{
    lruLists.resize(numSets);
    lruMaps.resize(numSets);
    // assuming in index fill in way 0,1,2... when cache is empty
    for (size_t i = 0; i < numSets; i++)
    {
        for (int lineIdx = 0; lineIdx < associativity; lineIdx++)
        {
            lruLists[i].push_back(lineIdx);
            lruMaps[i][lineIdx] = std::prev(lruLists[i].end());
        }
    }
}

Cache::DecodedAddress Cache::decodeAddress(uint64_t physicalAddress) const
{
    DecodedAddress addr;
    addr.offset = physicalAddress & ((1ULL << offsetBits) - 1);
    addr.setIndex = (physicalAddress >> offsetBits) & ((1ULL << indexBits) - 1);
    addr.tag = physicalAddress >> (offsetBits + indexBits);
    return addr;
}

bool Cache::access(uint64_t physicalAddress, bool write)
{
    DecodedAddress addr = decodeAddress(physicalAddress);
    CacheSet &set = sets[addr.setIndex];
    if (associativity == 1)
    {
        CacheLine &line = set.lines[0];
        if (write)
        {
            line.modified = true;
        }
        if (line.valid && line.tag == addr.tag)
        {
            hits++;
            return true;
        }
        misses++;
        line.valid = true;
        line.tag = addr.tag;
        line.phyAddr = physicalAddress;
        return false;
    }
    for (int i = 0; i < associativity; i++)
    {
        if (set.lines[i].valid && set.lines[i].tag == addr.tag)
        {
            hits++;
            if (write)
            {
                set.lines[i].modified = true;
            }
            updatePolicyOnHit(addr.setIndex, i);
            return true;
        }
    }
    misses++;
    int targetIndex = -1;
    for (int i = 0; i < associativity; i++)
    {
        if (!set.lines[i].valid)
        {
            targetIndex = i;
            break;
        }
    }
    if (targetIndex == -1)
    {
        targetIndex = getVictimIndex(addr.setIndex);
    }
    set.lines[targetIndex].valid = true;
    set.lines[targetIndex].tag = addr.tag;
    set.lines[targetIndex].phyAddr = physicalAddress;
    if (write)
        set.lines[targetIndex].modified = true;
    updatePolicyOnReplace(addr.setIndex, targetIndex);
    return false;
}
void Cache::updatePolicyOnHit(int setIndex, int lineIndex)
{
    if (isLRU)
    {
        auto &list = lruLists[setIndex];
        auto &map = lruMaps[setIndex];
        // move iterator to front of list (Most Recently Used)
        list.splice(list.begin(), list, map[lineIndex]);
    }
}

int Cache::getVictimIndex(int setIndex)
{
    if (isLRU)
    {
        return lruLists[setIndex].back(); // LRU is at back
    }
    else
    {
        return fifoNextVictim[setIndex];
    }
}

void Cache::updatePolicyOnReplace(int setIndex, int lineIndex)
{
    if (isLRU)
    {
        updatePolicyOnHit(setIndex, lineIndex);
    }
    else
    {
        fifoNextVictim[setIndex] = (fifoNextVictim[setIndex] + 1) % associativity;
    }
}

uint64_t Cache::getHits() const { return hits; }
uint64_t Cache::getMisses() const { return misses; }
double Cache::getHitRate() const
{
    return (hits + misses) == 0 ? 0.0 : (double)hits / (hits + misses);
}

void Cache::dump() const
{
    std::cout << "--- Cache Dump ---" << std::endl;
    for (size_t i = 0; i < numSets; i++)
    {
        for (int j = 0; j < associativity; j++)
        {
            const auto &line = sets[i].lines[j];
            if (line.valid)
            {
                if (line.modified)
                {
                    std::cout << "Set: " << i
                              << " | Tag:" << line.tag
                              << " | PhysAddr:" << line.phyAddr
                              << " | modified" << std::endl;
                }
                else
                {
                    std::cout << "Set: " << i
                              << " | Tag:" << line.tag
                              << " | PhysAddr:" << line.phyAddr << std::endl;
                }
            }
        }
    }
    std::cout << "------------------" << std::endl;
}

cacheHierarchy::cacheHierarchy(Cache *c1, Cache *c2)
    : l1Cache(c1), l2Cache(c2)
{
}
void cacheHierarchy::access(uint64_t physicalAddress, bool write)
{
    if (!(l1Cache->access(physicalAddress, write)))
    {
        l2Cache->access(physicalAddress, write);
    }
}

void cacheHierarchy::stats()
{
    // Constants for cycles
    const int L1_HIT_TIME = 1;
    const int L2_HIT_TIME = 10;
    const int RAM_ACCESS_TIME = 100;

    uint64_t l1Hits = l1Cache->getHits();
    uint64_t l1Misses = l1Cache->getMisses();
    uint64_t totalAccesses = l1Hits + l1Misses;

    double l1MissRate = (totalAccesses == 0) ? 0 : (double)l1Misses / totalAccesses;
    double l2MissRateLocal = (l2Cache->getHits() + l2Cache->getMisses() == 0) ? 0
                                                                              : (double)l2Cache->getMisses() / (l2Cache->getHits() + l2Cache->getMisses());

    // Calculate L1 Miss Penalty: L2_Hit + (L2_Miss_Rate * RAM_Time)
    double l1MissPenalty = L2_HIT_TIME + (l2MissRateLocal * RAM_ACCESS_TIME);

    // Calculate AAT: L1_Hit + (L1_Miss_Rate * L1_Miss_Penalty)
    double aat = L1_HIT_TIME + (l1MissRate * l1MissPenalty);

    std::cout << "\n========== Hierarchical Stats ==========\n";
    std::cout << "L1 Cache:\n";
    std::cout << "  Hits:             " << l1Hits << "\n";
    std::cout << "  Misses:           " << l1Misses << "\n";
    std::cout << "  Hit Rate:         " << std::fixed << std::setprecision(2) << l1Cache->getHitRate() * 100 << "%\n";

    std::cout << "\nL2 Cache:\n";
    std::cout << "  Hits:             " << l2Cache->getHits() << "\n";
    std::cout << "  Misses:           " << l2Cache->getMisses() << "\n";
    std::cout << "  Local Hit Rate:   " << l2Cache->getHitRate() * 100 << "%\n";

    std::cout << "\nPerformance Metrics:\n";
    std::cout << "  L1 Miss Penalty:  " << l1MissPenalty << " cycles\n";
    std::cout << "  Avg Access Time:  " << aat << " cycles\n";
    std::cout << "========================================\n";
}
