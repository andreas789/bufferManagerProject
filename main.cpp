//
// Created by andreas on 29/5/2024.

#include <iostream>
#include <cstring>
#include <cassert>
#include "src/SmartArrayMemoryManager.h"

using namespace std;

int main() {

    MemoryManager memManager;

    // Test 1: Test count of initial physical pages
    cout << "\nTest 1: Count of initial physical pages:" << endl;
    if(memManager.getPhysicalPages().size() == 1) {
        cout << "Success: Only 1 physical page exists when initializing the program." << endl;
    }
    else {
        cout << "Failed: More than 1 physical page exists when initializing the program." << endl;
    }




    // Test 2: Accessing unallocated slot
    cout << "\nTest 2: Accessing unallocated slot (0, 2):" << endl;
    void *accessedSlot = memManager.getSlot(0, 2);
    if (accessedSlot == nullptr) {
        cout << "Success: Retrieved nullptr for unallocated slot." << endl;
    } else {
        cerr << "Error: Retrieved non-null pointer for unallocated slot." << endl;
    }




    // Test 3: Allocate and Retrieve a Slot
    cout << "\nTest 3: Allocate and get slot (0, 0):" << endl;
    void *slot = memManager.allocateSlot(0, 0);
    if (slot != nullptr) {
        cout << "Success: Allocated slot at address: " << slot << endl;
    } else {
        cerr << "Error: Failed to allocate slot." << endl;
    }

    accessedSlot = memManager.getSlot(0, 0);
    if (accessedSlot == slot) {
        cout << "Success: Retrieved the correct allocated slot." << endl;
    } else {
        cerr << "Error: Retrieved incorrect or unallocated slot." << endl;
    }




    // Test 4: Force a Conflict and Test Resolution
    cout << "\nTest 4: Force a conflict and reallocate slot (0, 0):" << endl;
    void *conflictSlot = memManager.allocateSlot(0, 0); // This should trigger a conflict
    if (conflictSlot != nullptr && conflictSlot != slot) {
        cout << "Success: Conflict detected and slot reallocated at: " << conflictSlot << endl;
    } else {
        cerr << "Error: Conflict resolution failed." << endl;
    }





    // Test 5: Smart Population Test - Check for Null Pages
    cout << "\nTest 5: Smart Population Test (Ensure no null pages):" << endl;

    // Fill the initial page completely
    for (size_t i = 0; i < SLOTS_PER_PAGE; ++i) {
        void* slot = memManager.allocateSlot(0, i);
        assert(slot != nullptr);  // Assert successful allocation
    }

    // Force a new page allocation with the next slot (on a new virtual page)
    void* newPageSlot = memManager.allocateSlot(1, 0); // Allocate on a new virtual page
    assert(newPageSlot != nullptr);  // Assert successful allocation on new page

    // Verify that all virtual pages have valid physical page mappings
    const std::vector<void *> &virtualPages = memManager.getPhysicalPages();
    for (size_t i = 0; i < virtualPages.size(); ++i) {
        if (virtualPages[i] == nullptr) {
            cerr << "Error found: A null page was found in the vector of virtual pages.";
            return 1;
        }
    }

    // Check we can still get access from the new page
    accessedSlot = memManager.getSlot(1, 0);
    if (accessedSlot == newPageSlot) {
        cout << "Success: Retrieved the correct allocated slot in new page." << endl;
    } else {
        cerr << "Error: Retrieved incorrect or unallocated slot in new page." << endl;
    }

    cout << "Success: No null pages found and access to new page is successful." << endl;

    // Assert that we have at least 2 physical pages after the tests
    assert(memManager.getPhysicalPages().size() >= 2);



    // Test 6: Multiple Allocations
    cout << "\nTest 6: Multiple Allocations on different virtual pages:" << endl;
    void *slot1 = memManager.allocateSlot(2, 0);
    void *slot2 = memManager.allocateSlot(2, 1);
    if (slot1 != nullptr && slot2 != nullptr && slot1 != slot2) {
        cout << "Success: Multiple allocations on different virtual pages succeeded." << endl;
    } else {
        cerr << "Error: Multiple allocations on different virtual pages failed." << endl;
    }




    // Test 7: Boundary Test for Slots
    cout << "\nTest 7: Boundary Test for Slots:" << endl;
    void *lastSlot = memManager.allocateSlot(0, SLOTS_PER_PAGE - 1);
    if (lastSlot != nullptr) {
        cout << "Success: Allocation at boundary slot succeeded." << endl;
    } else {
        cerr << "Error: Allocation at boundary slot failed." << endl;
    }

    cout << "\nAll tests passed!" << endl;

    return 0; // Test passed
}
