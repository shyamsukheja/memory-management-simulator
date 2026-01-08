#pragma once

#include <vector>
#include <list>
#include <unordered_map>
#include <cstdint>
#include <cstddef>

class Cache {
public:
    Cache(size_t cacheSize, size_t blockSize, int associativity, bool useLRU);
    Cache(size_t cSize, size_t bSize, int assoc);
    bool access(uint64_t physicalAddress,bool write);
    uint64_t getHits() const;
    uint64_t getMisses() const;
    double getHitRate() const;
    void dump() const;

private:
    struct DecodedAddress {
        uint64_t tag;
        uint64_t setIndex;
        uint64_t offset;
    };

    struct CacheLine {
        bool valid = false;
        uint64_t tag = 0;
        uint64_t phyAddr = 0;
        bool modified = false;
    };

    struct CacheSet {
        std::vector<CacheLine> lines;
    };
    std::vector<CacheSet> sets;
    size_t blockSize;
    int associativity;
    size_t numSets;
    bool isLRU;
    int offsetBits;
    int indexBits;
    uint64_t hits = 0;
    uint64_t misses = 0;
    std::vector<int> fifoNextVictim;
    std::vector<std::list<int>> lruLists; 
    std::vector<std::unordered_map<int, std::list<int>::iterator>> lruMaps;
    void initFIFO();
    void initLRU();
    DecodedAddress decodeAddress(uint64_t physicalAddress) const;
    void updatePolicyOnHit(int setIndex, int lineIndex);
    void updatePolicyOnReplace(int setIndex, int lineIndex);
    int getVictimIndex(int setIndex);
};
class cacheHierarchy{
    public:
    cacheHierarchy(Cache *c1 , Cache*c2);
    void stats();
    void access(uint64_t physicalAddress,bool write);
    private:
    Cache *l1Cache;
    Cache *l2Cache;
};