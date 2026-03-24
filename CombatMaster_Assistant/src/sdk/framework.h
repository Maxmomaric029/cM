#pragma once
#include "pch.h"

namespace IL2CPP {
    // Standard IL2CPP String
    struct String {
        void* klass;
        void* monitor;
        int length;
        wchar_t chars[1];

        std::wstring ToString() {
            if (!this || length <= 0 || length > 2048) return L"";
            return std::wstring(chars, length);
        }
    };

    // Standard IL2CPP Array
    template<typename T>
    struct Array {
        void* klass;
        void* monitor;
        void* bounds;
        int max_length;
        T vector[1];
    };

    // IL2CPP List Helper (System.Collections.Generic.List<T>)
    template<typename T>
    struct List {
    private:
        // Size of the specific List object fields might vary minimally, but vector array is generally at 0x10
        // And size/count is generally at 0x18 as per user's listSize offset
    public:
        int GetSize() {
            return *(int*)((uintptr_t)this + Offsets::List::listSize);
        }

        Array<T>* GetItems() {
            return *(Array<T>**)((uintptr_t)this + 0x10);
        }
    };

    // Macro utility to cast and execute RVA functions
    template<typename Ret, typename... Args>
    inline Ret CallRVA(uintptr_t rva, Args... args) {
        auto func = reinterpret_cast<Ret(__fastcall*)(Args...)>(Memory::Get().GetBaseAddress() + rva);
        return func(args...);
    }
}
