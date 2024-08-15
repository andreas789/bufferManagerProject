//
// Created by andreas on 29/5/2024.

#include "SmartArrayMemoryManager.h"
#include <iostream>
#include <sys/mman.h>
#include <cstring>

using namespace std;


MemoryManager::MemoryManager() {
    // Initialize all initial virtual pages
    void* initialPhysicalPage = allocatePhysicalPage();
    for(size_t i = 0; i < INITIAL_VIRTUAL_PAGES; i++) {
        mapVirtualPageToPhysicalPage(i,  initialPhysicalPage);
    }
}


MemoryManager::~MemoryManager() {
    // Iterate over all physical pages
    for (void* page : physicalPages_) {
        // Unmap and free the current page
        if (munmap(page, PAGE_SIZE) != 0) {
            cerr << "Error unmapping page at " << page << ": " << strerror(errno) << endl;
            throw runtime_error("Failed to unmap physical page");
        }
    }
}


void* MemoryManager::allocatePhysicalPage() {

    void* page = mmap(nullptr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (page == MAP_FAILED) {
        cerr << "Error while mmapping page " << page << ": " << strerror(errno) << endl;
        return nullptr;
    }

    try {
        physicalPages_.push_back(page); // pushing page records to container physicalPages_
    } catch (const bad_alloc &e) {
        // Try to free up mmaped space if pushback fails.
        if (munmap(page, PAGE_SIZE) != 0) {
            cerr << "Error unmapping page during exception handling: " << strerror(errno) << endl;
        }
        throw runtime_error("Error adding page to vector: " + string(e.what()));
    }

    return page;
}


void MemoryManager::mapVirtualPageToPhysicalPage(size_t virtualPageId, void *physicalPage) {
    if(virtualPageId >= virtualPages_.size()) {
        virtualPages_.resize(virtualPageId + 1, nullptr);
    }
    virtualPages_[virtualPageId] = physicalPage;
}


void* MemoryManager::allocateSlot(size_t virtualPageId, size_t slotId) {

    if(virtualPageId >= virtualPages_.size()) {
        cerr << "Invalid page ID. ID should be less than " << INITIAL_VIRTUAL_PAGES << endl ;
        return nullptr;
    }

    void* physicalPage = virtualPages_[virtualPageId];
    if(physicalPage == nullptr) {
        cerr << "No physical page as mapped to this virtual page!";
        return nullptr;

    }

    size_t offset = slotId * SLOT_SIZE;
    void* slotAddress = static_cast<char*>(physicalPage) + offset;

    if(arrMetadata_[slotAddress].occupiedFlg) {
        // The specified slot is already occupied! Will try to resolve conflict
        resolveConflict(virtualPageId, slotId);
        physicalPage = virtualPages_[virtualPageId];
        slotAddress = static_cast<char*>(physicalPage) + offset;
    }

    arrMetadata_[slotAddress] = {
        static_cast<uint16_t>(virtualPageId),
        true
    };

    return slotAddress;
}


void* MemoryManager::getSlot(size_t virtualPageId, size_t slotId) {

    if (virtualPageId >= virtualPages_.size()) {
        cerr << "Invalid virtual page ID" << endl;
        return nullptr;
    }

    void* physicalPage = virtualPages_[virtualPageId];

    if (physicalPage == nullptr) {
        cerr << "No physical page mapped to this virtual page!" << endl;
        return nullptr;
    }

    size_t offset = slotId * SLOT_SIZE;

    void* slotAddress = static_cast<char*>(physicalPage) + offset;

    // Check if slot address exists in the map
    auto metadataIt = arrMetadata_.find(slotAddress);
    if (metadataIt == arrMetadata_.end()) {
        return nullptr; // Slot isn't allocated so return null ptr.
    }

    // Check for conflict and resolve if necessary
    if (metadataIt->second.occupiedFlg && metadataIt->second.virtualPageId != virtualPageId) {
        resolveConflict(virtualPageId, slotId);
        physicalPage = virtualPages_[virtualPageId]; // Update physicalPage after conflict resolution
        slotAddress = static_cast<char*>(physicalPage) + offset; // Recalculate slotAddress

        // Re-fetch metadata after conflict resolution
        metadataIt = arrMetadata_.find(slotAddress);
    }

    if (metadataIt != arrMetadata_.end() &&
        metadataIt->second.occupiedFlg) {
        return slotAddress;
    }
    else {
        return nullptr; // Slot is empty or invalid after conflict resolution
    }

}


void MemoryManager::resolveConflict(size_t virtualPageId, size_t slotId) {
    if (virtualPageId >= virtualPages_.size()) {
        virtualPages_.resize(virtualPageId + 1, nullptr);
    }

    try {
        // Attempt to find an existing physical page with the same slot unoccupied (no need in this case)

        // If no suitable page found, allocate a new one
        void* newPhysicalPage = allocatePhysicalPage();
        if (newPhysicalPage == nullptr) {
            throw runtime_error("Failed to allocate new physical page in resolveConflict");
        }
        mapVirtualPageToPhysicalPage(virtualPageId, newPhysicalPage);

        // Move ONLY the conflicting entry to the new page
        void* oldPhysicalPage = virtualPages_[virtualPageId];

        size_t offset = slotId * SLOT_SIZE;

        void* oldSlotAddress = static_cast<char*>(oldPhysicalPage) + offset;
        void* newSlotAddress = static_cast<char*>(newPhysicalPage) + offset;

        memcpy(newSlotAddress, oldSlotAddress, SLOT_SIZE);
        arrMetadata_[newSlotAddress] = arrMetadata_[oldSlotAddress];

        // Update arrMetadata_ for the old slot, mark it as empty.
        arrMetadata_[oldSlotAddress] = {0, false};

    } catch (const exception& e) {
        cerr << "Error resolving conflict: " << e.what() << endl;
    }
}
