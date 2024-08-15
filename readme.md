

# Overview of the Smart Array Implementation

The smart array implementation aims to reduce physical memory waste by sharing physical pages among multiple virtual pages, while at the same time, resolving conflicts during slot allocation, and ensuring efficient access to stored data.

# How the MemoryManager Class Works  ?

The `MemoryManager` class is responsible to manage the allocation, and mapping of virtual and physical memory pages. Here’s a high-level explanation of the class:

1. **Initialization**: When an instance of the `MemoryManager` class is created, it initializes a vector of virtual pages and allocates the first physical page using the `mmap` system call. All `mmap` flags used for this project are explained bellow.

2. **Slot Allocation**: The `allocateSlot` method allocates a slot within a specified virtual page. If a conflict is detected (i.e. the specific slot is already occupied by a different virtual page), the method resolves the conflict by allocating a new physical page and remapping the virtual page.

3. **Slot Retrieval**: The `getSlot` method retrieves a slot from a virtual page. It ensures that the correct physical page is mapped and accessible, otherwise errors are raised.

4. **Conflict Resolution**: When a conflict occurs during slot allocation, the `resolveConflict` method is invoked. This method reallocates the slot to a new physical page and updates the metadata to reflect the changes.

5. **Metadata Management**: Metadata is stored in a struct associated with each slot. This metadata includes the virtual page ID and an occupancy flag.

6. **Memory Management**: Physical memory pages are managed using Linux's `mmap` system call. Pages are allocated and mapped as needed, minimizing physical memory waste and optimizing access times.

The `MemoryManager` class encapsulates all these functionalities. A section describing the project structure will follow.

# Implementation Details

### Memory Management

Memory management is mainly handled using Linux's `mmap` system call to allocate physical pages and manage their lifecycle throughout runtime.

As per project requirements the mmap system call has used the following flags.

```cpp  
mmap(nullptr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);  
```  
The reason why those flags are used, is discussed below:

**`PROT_READ | PROT_WRITE`**: Allocated memory region should be readable and writable. This  ensures that the allocated pages can be both read from and written to by the application.

**`MAP_PRIVATE | MAP_ANONYMOUS`**: 1st flag implies that the changes to the memory region are private to the calling application and not visible to other processes that may map to  the same region. 2nd flag implies that the memory region is not backed by any file.

**`-1`**: Implies that the kernel should choose the address at which to create the mapping.

**`-0`**: Implies that the offset within the anonymous memory should be 0.

### Virtual and Physical Page Mapping

Virtual pages are mapped to physical pages using a vector container (`vector<void*> virtualPages_`). Each member of this vector is used to track mappings.

### Conflict Resolution Strategy

Conflicts during slot allocation are resolved by identifying and reallocating slots to different physical pages, ensuring that each virtual page maintains its integrity and minimizing data movement.

The conflict resolution algorith works as:

1. ****Identify Conflict:**** Check if the slot is occupied (`occupiedFlg` is `true`) by a different virtual page (`virtualPageId` mismatch).
2.   **Allocate New Physical Page:** If conflict detected, allocate a new physical page (`allocatePhysicalPage`) using `mmap`.
3. **Remap Virtual Pages:** Move the conflicting virtual page to the new physical page (`mapVirtualPageToPhysicalPage`).
4.  **Move Conflicting Entry:** Transfer the data from the old physical page to the new one to resolve the conflict with ```memcpy()```.
5. **Update Metadata:** Update metadata (`arrMetadata_`) to reflect the new slot location and occupancy status.

### Metadata Handling

Metadata in the implementation is managed using the `ArrMetadata` struct. This structure is associated with each slot in the array and stores essential information needed for efficient slot management and conflict resolution.
```cpp  
struct ArrMetadata {
    uint16_t virtualPageId : 15;
	bool occupiedFlg;
}
```  

### Size Constraints (8B Slots)

Each slot in is designed to accommodate 8 bytes (8B) of data, with 2B allocated for metadata and 6B available for data storage.

#### Metadata Storage
- **Structure**: The `ArrMetadata` struct has:
	- `uint16_t virtualPageId : 15` (15 bits for virtualPageId)
	- `bool occupiedFlg` (1 bit for occupancy flag)
- **Total Size**: 16 bits = 2 bytes

#### Data Storage
- **Slot Size**: Each slot accommodates 6 bytes of data.
- **Alignment**: Ensures data slots are efficiently packed without fragmentation.

This allocation strategy optimizes memory usage by dedicating 2 bytes to metadata per slot, leaving 6 bytes available for data storage.

### Error Handling

Error mechanisms do exists throughout the code and will raise exceptions/errors.

#  Organization & Class Functions

### Description of Files

This project is built/organized using CMake.

-   **Files**:
	1.  **src/SmartArrayMemoryManager.cpp**: Implements the `MemoryManager` class with constructors and destructors.
	2.  **src/SmartArrayMemoryManager.h**: Header file defining constants, the `MemoryManager` class, its members, and basic function documentation.
	3.  **/main.cpp**: Executes the application, providing comprehensive tests and demonstrations of `MemoryManager` functionalities, showcasing slot allocation, conflict resolution, and management of virtual-to-physical page mappings.

# Performance Considerations

### Memory Footprint

The implementation minimizes physical memory waste by sharing pages among virtual pages, optimizing memory usage per slot (`SLOT_SIZE`) and page size (`PAGE_SIZE`).

### Computational Efficiency

Efficient conflict resolution strategies and metadata handling ensure low computational overhead during slot allocation & retrieval operations.


# Testing Approach

### Basic explanation

- The ``/main.cpp`` file tests the basic functionality of the implementation. Specifically tests for conflict resolution, memory management, edge cases etc.

### **Test Cases & Expected Outcome Overview**

- Test 1: Verify initial physical page count.
  ----> **Outcome:** Only 1 physical page should exist when initializing the program.

- Test 2: Access an unallocated slot.  
  ----> **Outcome:** Retrieving an unallocated slot should return `nullptr`.

- Test 3: Allocate and retrieve a slot.  
  ----> **Outcome:** Allocation should succeed, and the correct slot should be retrieved.

- Test 4: Force a conflict and test resolution.  
  ----> **Outcome:** Conflict should be detected, and the slot should be reallocated correctly.

- Test 5: Ensure no null pages are present after allocations.  
  ----> **Outcome:** No null pages should be found.

- Test 6: Perform multiple allocations on different virtual pages.  
  ----> **Outcome:** Multiple allocations should succeed, with proper memory management.

- Test 7: Test slot allocation at boundary.  
  ----> **Outcome:** Slot allocation at boundary conditions should be handled correctly.


## Results and Observations

Tests confirm the robustness and reliability of the smart array implementation, highlighting areas of improvement and additional testing needs.

# Conclusion

The smart array implementation optimizes memory usage, implements efficient conflict resolution strategies, and ensures data integrity during slot allocation and access operations.
