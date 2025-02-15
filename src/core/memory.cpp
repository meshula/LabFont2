#include "memory.h"
#include <cstdlib>
#include <iostream>

namespace labfont {

MemoryManager& MemoryManager::Instance() {
    static MemoryManager instance;
    return instance;
}

MemoryManager::MemoryManager() {
    // Initialize category usage counters
    for (int i = 0; i < static_cast<int>(MemoryCategory::Temporary) + 1; ++i) {
        auto category = static_cast<MemoryCategory>(i);
        m_categoryUsage[category] = 0;
    }
}

MemoryManager::~MemoryManager() {
    if (m_leakDetectionEnabled && m_currentUsage > 0) {
        DumpLeaks();
    }
}

void* MemoryManager::Allocate(size_t size, MemoryCategory category) {
    void* ptr = std::malloc(size);
    if (!ptr) {
        ErrorContext::Instance().SetError(LAB_ERROR_OUT_OF_MEMORY, "Failed to allocate memory");
        return nullptr;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_totalAllocated += size;
        m_currentUsage += size;
        m_categoryUsage[category] += size;
        
        if (m_currentUsage > m_peakUsage) {
            m_peakUsage = m_currentUsage;
        }

        m_allocations[ptr] = {
            size,
            category,
            __FILE__,
            __LINE__
        };
    }

    return ptr;
}

void MemoryManager::Free(void* ptr) {
    if (!ptr) return;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_allocations.find(ptr);
        if (it != m_allocations.end()) {
            const auto& info = it->second;
            m_totalFreed += info.size;
            m_currentUsage -= info.size;
            m_categoryUsage[info.category] -= info.size;
            m_allocations.erase(it);
        }
    }

    std::free(ptr);
}

MemoryStats MemoryManager::GetStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    MemoryStats stats;
    stats.totalAllocated = m_totalAllocated;
    stats.totalFreed = m_totalFreed;
    stats.currentUsage = m_currentUsage;
    stats.peakUsage = m_peakUsage;
    
    for (const auto& [category, usage] : m_categoryUsage) {
        stats.categoryUsage[category] = usage;
    }
    
    return stats;
}

void MemoryManager::ResetStats() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_totalAllocated = 0;
    m_totalFreed = 0;
    m_currentUsage = 0;
    m_peakUsage = 0;
    
    for (auto& [category, usage] : m_categoryUsage) {
        usage = 0;
    }
    
    m_allocations.clear();
}

void MemoryManager::EnableLeakDetection(bool enable) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_leakDetectionEnabled = enable;
}

void MemoryManager::DumpLeaks() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_allocations.empty()) {
        std::cout << "No memory leaks detected.\n";
        return;
    }

    std::cout << "Memory leaks detected:\n";
    for (const auto& [ptr, info] : m_allocations) {
        std::cout << "Leak at " << ptr << ":\n"
                  << "  Size: " << info.size << " bytes\n"
                  << "  Category: " << static_cast<int>(info.category) << "\n"
                  << "  Location: " << info.file << ":" << info.line << "\n";
    }
}

// Global allocation helpers
void* lab_allocate(size_t size, MemoryCategory category) {
    return MemoryManager::Instance().Allocate(size, category);
}

void lab_free(void* ptr) {
    MemoryManager::Instance().Free(ptr);
}

} // namespace labfont
