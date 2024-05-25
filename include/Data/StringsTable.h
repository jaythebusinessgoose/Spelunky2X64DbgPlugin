#pragma once

#include <QStandardItemModel>
#include <QString>
#include <cstdint>
#include <string>

namespace S2Plugin
{
    class StringsTable
    {
      public:
        QString stringForIndex(uint32_t idx) const;
        bool isValid() const
        {
            return (ptr != 0);
        }
        // returns address in the table, not address for the string itself
        uintptr_t addressOfIndex(uint32_t idx) const
        {
            return ptr + idx * sizeof(uintptr_t);
        }
        uintptr_t stringAddressOfIndex(uint32_t idx) const;
        size_t count() const
        {
            return size;
        }
        QStandardItemModel* modelCache() const
        {
            return const_cast<QStandardItemModel*>(&cache);
        }

      private:
        uintptr_t ptr{0};
        size_t size{0};
        // Use the model as cache
        QStandardItemModel cache;

        StringsTable(){};
        ~StringsTable(){};
        StringsTable(const StringsTable&) = delete;
        StringsTable& operator=(const StringsTable&) = delete;

        friend struct Spelunky2;
    };
} // namespace S2Plugin
