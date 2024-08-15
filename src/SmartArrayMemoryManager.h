//
// Created by andreas on 29/5/2024.
//

#ifndef SMARTARRAYMEMORYMANAGER_H
#define SMARTARRAYMEMORYMANAGER_H

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

// Defining constant variables as per project requirements.
constexpr size_t PAGE_SIZE = 4096; // 4kb
constexpr size_t SLOT_SIZE = 8; // 8b
constexpr size_t SLOTS_PER_PAGE = PAGE_SIZE / SLOT_SIZE; // Number of slots per page
constexpr size_t INITIAL_VIRTUAL_PAGES = 1024; // Number of virtual pages


// Defining the struct type that will hold array metadata for each slot.
struct ArrMetadata {
    uint16_t virtualPageId :15; // 15 bits for virtualPageId
    bool occupiedFlg;          // 1 bit for the occupied flg
                                // 15 + 1 = 16 in total -> 2KB
};


class MemoryManager {
public:
    // Constructor -> Initializes application.
    MemoryManager();

    // Deconstructor -> Cleans up unallocated resources.
    ~MemoryManager();

    /**
     * Allocates a slot within a virtual page.
     *
     * This function attempts to allocate a specific slot within a given virtual page. It
     * handles situations where the virtual page is not yet mapped or the slot is already
     * occupied by attempting conflict resolution.
     *
     * @param virtualPageId The ID of the virtual page in which to allocate the slot.
     * @param slotId The ID of the slot to allocate within the virtual page.
     * @return A pointer to the allocated slot if successful, otherwise nullptr.
     */
    void* allocateSlot(size_t virtualPageId, size_t slotId);

    /**
     * Retrieves a slot from a virtual page.
     *
     * This function fetches a specific slot within a given virtual page. It handles
     * situations where the virtual page is not yet mapped, the slot is unallocated,
     * or conflicts arise (e.g., the slot is currently occupied by a different virtual page).
     * In case of conflicts, it attempts to resolve them.
     *
     * @param virtualPageId The ID of the virtual page containing the desired slot.
     * @param slotId The ID of the slot to retrieve within the virtual page.
     * @return A pointer to the slot if successful, or nullptr if page is empty or invalid.
     *
     */
    void* getSlot(size_t virtualPageId, size_t slotId);



    const vector<void*>& getPhysicalPages() const {
        return physicalPages_;
    }

private:

    // Vectors/Maps for storing pointers to the virtual/physical memory pages and metadata.
    vector<void*> virtualPages_;
    vector<void*> physicalPages_;
    unordered_map<void*, ArrMetadata> arrMetadata_;


    /**
     * Allocates a physical page using mmap.
     *
     * This function attempts to allocate a new physical memory page of a fixed size
     * using the `mmap` system call. If successful, it adds the allocated page to an internal list
     * for tracking.
     *
     * @return A pointer to the starting address of the allocated page if successful,
     *         or nullptr if the allocation fails
     * @throws runtime_error If there's an error adding the allocated page to the internal tracking list.
     */
    void* allocatePhysicalPage();

    /**
     * Maps a virtual page to a physical page.
     *
     * This function associates a virtual page with a given physical page in the memory manager.
     * If the virtual page ID exceeds the current capacity, the internal virtual page table is
     * automatically expanded.
     *
     * @param virtualPageId The ID of the virtual page to be mapped.
     * @param physicalPage A pointer to the physical page to which the virtual page should be mapped.
     */
    void mapVirtualPageToPhysicalPage(size_t virtualPageId, void* physicalPage);

    /**
     * Resolves conflicts when allocating slots.
     *
     * This function attempts to find an existing physical page with a free slot for the given virtual page.
     * If none is found, it allocates a new physical page, maps the virtual page to it, and moves the
     * conflicting slot data to the new page.
     *
     * @param virtualPageId The ID of the virtual page experiencing the conflict.
     * @param slotId The ID of the conflicting slot.
     * @throws runtime_error If a new physical page cannot be allocated.
     */
    void resolveConflict(size_t virtualPageId, size_t slotId);



};

#endif //SMARTARRAYMEMORYMANAGER_H
