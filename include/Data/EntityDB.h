#pragma once

#include <cstdint>

namespace S2Plugin
{
    class EntityDB
    {
      public:
        uintptr_t offsetForIndex(uint32_t idx) const // as of right now id == index
        {
            return ptr == 0ull ? 0ull : ptr + idx * entitySize();
        }
        bool isValid() const
        {
            return (ptr != 0);
        }
        static size_t entitySize();

      private:
        uintptr_t ptr{0};

        friend struct Spelunky2;
    };
} // namespace S2Plugin
