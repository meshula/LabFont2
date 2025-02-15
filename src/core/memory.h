#ifndef LABFONT_MEMORY_H
#define LABFONT_MEMORY_H

#include <labfont/labfont_types.h>
#include "error.h"
#include <cstddef>
#include <atomic>
#include <unordered_map>
#include <mutex>

namespace labfont {

// Memory allocation categories for tracking
enum class MemoryCategory {
    General,
    Graphics,
    Text,
    Resources,
    Temporary
};

// Memory statistics
struct MemoryStats {
    size_t totalAllocated;
    size_t totalFreed;
    size_t currentUsage;
    size_t peakUsage;
    std::unordered_map<MemoryCategory, size_t> categoryUsage;
};

// Memory manager interface
class MemoryManager {
public:
    static MemoryManager& Instance();

    void* Allocate(size_t size, MemoryCategory category);
    void Free(void* ptr);
    
    MemoryStats GetStats() const;
    void ResetStats();

    // Debug features
    void EnableLeakDetection(bool enable);
    void DumpLeaks() const;

private:
    MemoryManager();
    ~MemoryManager();

    struct AllocationInfo {
        size_t size;
        MemoryCategory category;
        const char* file;
        int line;
    };

    size_t m_totalAllocated{0};
    size_t m_totalFreed{0};
    size_t m_currentUsage{0};
    size_t m_peakUsage{0};
    std::unordered_map<MemoryCategory, size_t> m_categoryUsage;
    std::unordered_map<void*, AllocationInfo> m_allocations;
    
    bool m_leakDetectionEnabled{false};
    mutable std::mutex m_mutex;
};

// Allocation helpers
void* lab_allocate(size_t size, MemoryCategory category = MemoryCategory::General);
void lab_free(void* ptr);

// Custom allocator for STL containers
template<typename T>
class StlAllocator {
public:
    using value_type = T;

    StlAllocator(MemoryCategory category = MemoryCategory::General) 
        : m_category(category) {}

    template<typename U>
    StlAllocator(const StlAllocator<U>& other) 
        : m_category(other.GetCategory()) {}

    T* allocate(size_t n) {
        return static_cast<T*>(lab_allocate(n * sizeof(T), m_category));
    }

    void deallocate(T* p, size_t) {
        lab_free(p);
    }

    MemoryCategory GetCategory() const { return m_category; }

private:
    MemoryCategory m_category;
};

} // namespace labfont

// Memory tracking macros
#ifdef LABFONT_DEBUG
    #define LAB_NEW(type, category) \
        new (lab_allocate(sizeof(type), category)) type
    #define LAB_DELETE(ptr) \
        do { if (ptr) { ptr->~T(); lab_free(ptr); } } while(0)
#else
    #define LAB_NEW(type, category) new type
    #define LAB_DELETE(ptr) delete ptr
#endif

#endif // LABFONT_MEMORY_H
