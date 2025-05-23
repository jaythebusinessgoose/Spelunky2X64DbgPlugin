#pragma once

#include "Configuration.h"
#include "QtHelpers/AbstractContainerView.h"
#include <cstdint>
#include <string>

namespace S2Plugin
{
    class ViewStdMap : public AbstractContainerView
    {
        Q_OBJECT
      public:
        ViewStdMap(uintptr_t address, const std::string& keyTypeName, const std::string& valueTypeName, QWidget* parent = nullptr);

      protected:
        void reloadContainer() override;

      protected slots:
        void onItemCollapsed(const QModelIndex& index);

      private:
        MemoryField mKeyField;
        MemoryField mValueField;
        uintptr_t mMapAddress;
        uint8_t mMapKeyAlignment;
        uint8_t mMapValueAlignment;
    };
} // namespace S2Plugin
