# Memory Management Simulator

A comprehensive simulation of Operating System memory management components, including a Physical Memory Allocator, Multilevel Cache Hierarchy, and Virtual Memory Paging system.

## 🚀 Features

### 1. Physical Memory Allocator (allocator.h)

Simulates a contiguous heap memory manager.

- **Strategies**: First Fit, Best Fit (Optimized with BST), and Worst Fit.
- **Deallocation**: Instant O(1) access to blocks via ID lookup.
- **Coalescing**: Automatic merging of adjacent free blocks to reduce external fragmentation.
- **Metrics**: Tracks external fragmentation and allocation success rates.

### 2. Cache Hierarchy (cache.h)

Simulates an N-way set-associative L1 and L2 cache system.

- **Architecture**: Configurable Block Size, Associativity, and Cache Sizes.
- **Replacement Policies**:
  - **FIFO**: First-In, First-Out eviction.
  - **LRU**: Least Recently Used (Optimized with std::list splicing and unordered_map iterators for O(1) updates).
- **Analysis**: Reports Hit Rates, Miss Rates, and Average Access Time (AAT).

### 3. Virtual Memory (virtualmemory.h)

Simulates Paging and Address Translation.

- **MMU**: Translates Virtual Addresses to Physical Addresses.
- **Page Tables**: Sparse implementation using Hash Maps to save simulation memory.
- **Page Fault Handling**:
  - Auto-allocation of frames on faults.
  - **Global Replacement**: Evicts frames when RAM is full using FIFO or LRU.
  - **Reverse Mapping**: Automatically invalidates the victim process's page table entry.

### 4. Integrated Mode (Full System Simulation)

Simulates the complete memory access pipeline of a modern CPU.

- **Flow**: Virtual Address → Page Table (MMU) → Physical Address → L1 Cache → L2 Cache → Physical RAM.
- **Interaction**: Accessing a virtual address triggers address translation (handling page faults if necessary), followed by a cache hierarchy lookup using the translated physical address.

## 🎥 Demo Video

https://github.com/user-attachments/assets/78eeb793-d248-4ed9-9824-94d735592cf4

## 📂 Project Structure

```
memory-simulator/
├── Makefile            # Build script
├── run_tests.sh        # Script to run all test cases
├── docs/               # Documentation files
├── include/            # Header files
│   ├── allocator.h
│   ├── cache.h
│   └── virtualmemory.h
├── src/                # Source code
│   ├── allocator.cpp
│   ├── cache.cpp
│   ├── main.cpp
│   └── virtualmemory.cpp
├── tests/              # Test input scripts
│   ├── error1-3.txt        # Error handling scenarios
│   ├── test_allocator*.txt # Allocation strategy tests
│   ├── test_cache*.txt     # L1/L2 hierarchy tests
│   ├── test_vm*.txt        # Paging/Translation tests
│   └── test_integrated*.txt # Full system tests
└── outputs/            # Output logs from test runs
```

## 🛠️ Installation & Build

### Prerequisites

Before running the simulator, ensure your system meets the following requirements:

- **C++ Compiler**: g++ (supports C++17 or later) or clang++.
- **Build Tool**: make (for automated compilation).
- **Version Control**: git (to clone the repository).
- **Environment**: Linux Terminal or Git Bash (Windows).

### 1. Clone the Repository

To get started, clone the repository to your local machine:

```bash
git clone https://github.com/shyamsukheja/memory-management-simulator.git
cd memory-management-simulator
```

### 2. Build the Project

The project comes with a Makefile. To build the executable from source:

```bash
make
```

This will compile source files from `src/`, place object files in `build/`, and create the `memsim` executable.

*(Note: On Windows with MinGW, if `make` is not found, try `mingw32-make`.)*

### 3. Run Interactively

Once compiled, execute the program from your terminal:

```bash
./memsim
```

## 📖 User Guide (CLI Commands)

The simulator operates in four distinct modes. Upon running `./memsim`, select a mode from the menu.

### Navigation Commands (Available in all modes)

- `back`: Return to the main menu.
- `exit`: Terminate the program immediately.

### 1. Allocator Mode

- `init <size>`: Initialize memory pool (bytes).
- `mode <first|best|worst>`: Select allocation strategy.
- `malloc <size>`: Allocate memory block. Returns ID.
- `free <id>`: Release memory block.
- `stats`: Show fragmentation and usage.
- `dump`: Visualize memory layout.

### 2. Cache Mode

- `init <L1> <L2> <blk> <assoc> <pol>`: Setup cache (Sizes in bytes, Pol: 0=FIFO, 1=LRU).
- `access <addr> <0|1>`: Access address (0=Read, 1=Write).
- `stats`: Show Hit/Miss rates and AAT.
- `dump`: Show valid lines in L1 and L2 caches.

### 3. Virtual Memory Mode

- `init <ram> <pg> <pol>`: Setup RAM and Page Size (bytes, Pol: 0=FIFO, 1=LRU).
- `access <pid> <vAddr>`: Access virtual address for specific Process ID.
- `status`: Show status of physical frames (Used/Free, PID owner).

### 4. Integrated Mode

Combines all subsystems.

- `init <ram> <pg> <l1> <l2> <blk> <assoc>`: Full system setup (LRU policy is default).
- `access <pid> <vAddr> <0|1>`: Perform a full memory access (0=Read, 1=Write).
- `stats`: Show hierarchy performance metrics.
- `dump`: Show state of all components (L1, L2, RAM).

## 🧪 Testing

The `tests/` directory contains pre-written scenarios to verify system correctness.

### Running Individual Tests

You can pipe any test file directly into the simulator:

```bash
./memsim < tests/test_allocator1.txt
```

### Running All Tests

A shell script is provided to automate regression testing. It runs all files in `tests/` and saves the results to the `outputs/` directory.

```bash
./run_tests.sh
```

### Test Suite Description:

- `test_allocator*.txt`: Verifies First/Best/Worst fit logic and coalescing.
- `test_cache*.txt`: Verifies Hit/Miss logic, LRU eviction order, and Associativity limits.
- `test_vm*.txt`: Verifies Page Fault handling, Frame allocation, and Page Table updates.
- `test_integrated*.txt`: Verifies the end-to-end pipeline (Translation + Caching).
- `error*.txt`: Tests boundary conditions (OOM, Invalid IDs, Bad Inputs).
