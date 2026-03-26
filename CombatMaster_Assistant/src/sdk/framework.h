#pragma once
#include "pch.h"

// ─────────────────────────────────────────────────────────────────────────────
// External IL2CPP helpers — all memory accessed via RPM through Memory::Get()
// IL2CPP::CallRVA is NOT available in external mode.
// ─────────────────────────────────────────────────────────────────────────────
namespace IL2CPP {

    // Read a remote IL2CPP Array and access its vector field
    // Layout: klass(8) | monitor(8) | bounds(8) | max_length(4) | pad(4) | vector[0]
    template<typename T>
    struct Array {
        static constexpr uintptr_t VECTOR_OFFSET = 0x20; // after klass/monitor/bounds/max_length

        // Read a single element from the remote array pointer
        static T GetElement(uintptr_t arrayPtr, int index) {
            if (!arrayPtr || index < 0) return T{};
            uintptr_t elemAddr = arrayPtr + VECTOR_OFFSET + (uintptr_t)(index * sizeof(T));
            return Memory::Get().Read<T>(elemAddr);
        }

        static int GetLength(uintptr_t arrayPtr) {
            if (!arrayPtr) return 0;
            // max_length is at offset 0x18
            int len = Memory::Get().Read<int>(arrayPtr + 0x18);
            return (len < 0 || len > 1024) ? 0 : len;
        }
    };

    // Read a remote IL2CPP List<T>
    // Layout: klass(8) | monitor(8) | _items ptr(8) | _size(4)
    template<typename T>
    struct List {
        static int GetSize(uintptr_t listPtr) {
            if (!listPtr) return 0;
            int n = Memory::Get().Read<int>(listPtr + Offsets::List::listSize);
            return (n < 0 || n > 256) ? 0 : n;
        }

        // Returns pointer to the IL2CPP Array object holding items
        static uintptr_t GetItemsPtr(uintptr_t listPtr) {
            if (!listPtr) return 0;
            return Memory::Get().Read<uintptr_t>(listPtr + 0x10);
        }

        static T GetElement(uintptr_t listPtr, int index) {
            uintptr_t itemsPtr = GetItemsPtr(listPtr);
            return Array<T>::GetElement(itemsPtr, index);
        }
    };
}
