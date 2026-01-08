#pragma once
#include <cstddef>
#include <list>
#include <set>
#include <iterator>
#include<unordered_map>
struct block {
    std::size_t addr; 
    std::size_t len;  
    bool is_free;     
    int id;
};

struct Compare {
    using is_transparent = void; // Enables searching by size_t

    // Standard comparison for the set internal ordering
    bool operator()(const std::list<block>::iterator& a, 
                    const std::list<block>::iterator& b) const {
        if (a->len != b->len) return a->len < b->len;
        return a->addr < b->addr;
    }

    //Comparison to allow searching by size
    bool operator()(const std::list<block>::iterator& a, std::size_t size) const {
        return a->len < size;
    }

    bool operator()(std::size_t size, const std::list<block>::iterator& a) const {
        return size < a->len;
    }
};

class Memory {
public:
    explicit Memory(std::size_t size);
    int allocate_firstfit(std::size_t size);
    int allocate_bestfit(std::size_t size);
    int allocate_worstfit(std::size_t size);
    void free(int id);
    void dump() const;
    double ext_frag() const;
    double alloc_success_rate() const;
    std::size_t total_memory() const;
    std::size_t used_memory() const;

private:
    std::size_t totalsize;
    int next_id;
    long long attempts;
    long long hits;
    std::list<block> mem_list;
    std::unordered_map<int, std::list<block>::iterator> id_map;
    std::set<std::list<block>::iterator, Compare> index;
    void add_index(std::list<block>::iterator it);
    void del_index(std::list<block>::iterator it);
    void addblock(std::list<block>::iterator it, std::size_t size);
    std::size_t usedmemory;
};
