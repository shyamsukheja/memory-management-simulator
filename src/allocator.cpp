#include"allocator.h"
#include<iostream>
Memory::Memory(size_t total_size)
:totalsize(total_size),
next_id(1),
attempts(0),
hits(0),
usedmemory(0)
{
    block initial;
    initial.addr= 0 ;
    initial.len = totalsize;
    initial.id = -1; // not assigned
    initial.is_free = true;
    mem_list.push_back(initial);
    add_index(mem_list.begin());
}
void Memory::add_index(std::list<block>::iterator it){
    if (it->is_free) {
        index.insert(it);
    }
}
void Memory::del_index(std::list<block>::iterator it ){
    auto found_index = index.find(it);
    if(found_index!=index.end()){
        index.erase(found_index);
    }
}
void Memory::addblock(std::list<block>::iterator it, std::size_t size){
    del_index(it);
    long long rem_space=it->len - size;
    it->is_free=false;
    it->id = next_id++;
    id_map[it->id] = it;
    hits++;
    if(rem_space>0){
        block newblock;
        newblock.addr = it->addr + size;
        newblock.len = rem_space;
        newblock.id = -1;
        newblock.is_free = true;
        auto new_it = mem_list.insert(std::next(it),newblock);
        add_index(new_it);
        it->len= size;
    }
    usedmemory+=size;
}
int Memory::allocate_firstfit(std::size_t size){
    attempts++;
    for(auto it = mem_list.begin();it!=mem_list.end();++it){
        if(it->is_free&&it->len>=size){
            addblock(it,size);
            return it->id;
        }
    }
    return -1;
}
int Memory::allocate_bestfit(std::size_t size) {
    attempts++;
    auto it_ = index.lower_bound(size); 
    
    if (it_ != index.end()) {
        std::list<block>::iterator target = *it_;
        addblock(target, size);
        return target->id;
    }
    return -1;
}
int Memory :: allocate_worstfit(std::size_t size){
    attempts++;
    if (index.empty()){
        return -1;
    }
    auto it_ = index.rbegin();
    if((*it_)->len>=size){
        addblock(*it_,size);
        return (*it_)->id;
    }
    return -1;
}
void Memory::free(int id){
    auto map_it = id_map.find(id);
    if(map_it == id_map.end()) return;
    auto it = map_it->second;
    id_map.erase(map_it);
    it->is_free=true;
    it->id=-1;
    usedmemory-=it->len;
       if (it != mem_list.begin()) {
        auto prev = std::prev(it);
        if (prev->is_free) {
            del_index(prev);
            prev->len += it->len;
            mem_list.erase(it);
            it = prev;
        }
    }
    auto next = std::next(it);
    if (next != mem_list.end() && next->is_free) {
        del_index(next);
        it->len += next->len;
        mem_list.erase(next);
    }
    add_index(it);
}
void Memory::dump() const {
    std::cout << "--- Memory Dump ---" << std::endl;
    for (const auto& b : mem_list) {
        std::cout << "[" << b.addr << " - " << (b.addr + b.len - 1) 
                  << "] Size: " << b.len;
        if (b.is_free) std::cout << " (FREE)";
        else std::cout << " (ID: " << b.id << ")";
        std::cout << std::endl;
    }
}
std::size_t Memory::total_memory() const {
    return totalsize;
}
std::size_t Memory::used_memory() const {
    return usedmemory;
}
double Memory::alloc_success_rate() const {
    if (attempts == 0) return 0.0;
    return (static_cast<double>(hits) / attempts) * 100.0;
}
double Memory::ext_frag() const {
    std::size_t total_free = totalsize - usedmemory;
    if (index.empty()) return 0.0; 
    if (total_free == 0) return 0.0;
    std::size_t largest = (*index.rbegin())->len;
    // Formula: 1 - (Largest / Total Free)
    return 1.0 - (static_cast<double>(largest) / total_free);
}

// int main(){
//     Memory m1(1024);
//     int a = m1.allocate_firstfit(100);
//     int b = m1.allocate_firstfit(900);
//     m1.free(a);
//     m1.allocate_bestfit(20);
//     m1.allocate_worstfit(3);
//     m1.allocate_bestfit(100);
//     m1.dump();
//     std::cout<<m1.ext_frag()<<std::endl<<m1.used_memory()<<std::endl<<m1.alloc_success_rate();
//     return 0;
// }
