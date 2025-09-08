#pragma once

#include "Log.h"

#include <string>

inline std::string formatWithDots(size_t value)
{
    std::string s = std::to_string(value);
    std::string result;

    int count = 0;
    for (int i = static_cast<int>(s.size()) - 1; i >= 0; --i)
    {
        result.insert(result.begin(), s[i]);
        if (++count % 3 == 0 && i != 0)
            result.insert(result.begin(), '.');
    }
    return result;
}

namespace Delta
{
    class ArenaAllocator
    {
    public:
        inline explicit ArenaAllocator(size_t bytes)
            : m_size(bytes)
        {
            m_buffer = static_cast<std::byte *>(malloc(m_size));
            m_offset = m_buffer;
        }

        template <typename T, typename... Args>
        T *alloc(Args &&...args)
        {
            void *offset = m_offset;
            m_offset += sizeof(T);
            return new (offset) T(std::forward<Args>(args)...);
        }

        inline ArenaAllocator(const ArenaAllocator &other) = delete;

        inline ArenaAllocator operator=(const ArenaAllocator &other) = delete;

        inline ~ArenaAllocator()
        {
            size_t used = static_cast<size_t>(m_offset - m_buffer);
            LOG_TRACE("Parser used {} / {} bytes",
                      formatWithDots(used),
                      formatWithDots(m_size));
            free(m_buffer);
        }

    private:
        size_t m_size;
        std::byte *m_buffer;
        std::byte *m_offset;
    };

}