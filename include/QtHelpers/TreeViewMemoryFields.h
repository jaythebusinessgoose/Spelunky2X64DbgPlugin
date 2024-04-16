#pragma once

#include "QtHelpers/StyledItemDelegateHTML.h"
#include <QStandardItemModel>
#include <QTreeView>
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace S2Plugin
{
    struct MemoryField;

    struct ColumnFilter
    {
        constexpr ColumnFilter& enable(uint8_t h)
        {
            activeColumns = activeColumns | (1U << h);
            return *this;
        }
        constexpr ColumnFilter& disable(uint8_t h)
        {
            activeColumns = activeColumns & ~(1U << h);
            return *this;
        }
        constexpr bool test(uint8_t h) const
        {
            return (activeColumns & (1U << h)) != 0;
        }

      private:
        uint16_t activeColumns{0xFFFF};
    };

    class TreeViewMemoryFields : public QTreeView
    {
        Q_OBJECT
      public:
        TreeViewMemoryFields(QWidget* parent = nullptr);

        void addMemoryFields(const std::vector<MemoryField>& fields, const std::string& mainName, uintptr_t structAddr, size_t initialDelta = 0, QStandardItem* parent = nullptr);
        QStandardItem* addMemoryField(const MemoryField& field, const std::string& fieldNameOverride, uintptr_t memoryAddress, size_t delta, QStandardItem* parent = nullptr);
        void clear();
        void updateTableHeader(bool restoreColumnWidths = true);
        void setEnableChangeHighlighting(bool b) noexcept;

        void updateTree(uintptr_t newAddr = 0, uintptr_t newComparisonAddr = 0, bool initial = false);
        void updateRow(int row, std::optional<uintptr_t> newAddr = std::nullopt, std::optional<uintptr_t> newAddrComparison = std::nullopt, QStandardItem* parent = nullptr,
                       bool disableChangeHighlightingForField = false);

        void labelAll(std::string_view prefix = {});

        ColumnFilter activeColumns;

      protected:
        void dragEnterEvent(QDragEnterEvent* event) override;
        void dragMoveEvent(QDragMoveEvent* event) override;
        void dropEvent(QDropEvent* event) override;
        void startDrag(Qt::DropActions supportedActions) override;

      signals:
        void memoryFieldValueUpdated(int row, QStandardItem* parrent);
        void levelGenRoomsPointerClicked();
        void entityOffsetDropped(size_t offset);

      private slots:
        void cellClicked(const QModelIndex& index);

      private:
        QStandardItemModel* mModel;
        StyledItemDelegateHTML mHTMLDelegate;
        std::array<uint32_t, 9> mSavedColumnWidths = {0};
        bool mEnableChangeHighlighting = true;
    };
} // namespace S2Plugin
