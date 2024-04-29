#include "QtHelpers/DialogEditSimpleValue.h"
#include "Configuration.h"
#include "QtPlugin.h"
#include "pluginmain.h"
#include <QDoubleValidator>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QIntValidator>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <iomanip>
#include <sstream>

S2Plugin::DialogEditSimpleValue::DialogEditSimpleValue(const QString& fieldName, uintptr_t memoryAddress, MemoryFieldType type, QWidget* parent)
    : QDialog(parent, Qt::WindowCloseButtonHint | Qt::WindowTitleHint), mMemoryAddress(memoryAddress), mFieldType(type)
{
    setModal(true);
    setWindowTitle("Change value");
    setWindowIcon(getCavemanIcon());
    auto layout = new QVBoxLayout(this);

    // FIELDS
    auto gridLayout = new QGridLayout(this);

    gridLayout->addWidget(new QLabel(QString("Change value of %1").arg(fieldName), this), 0, 0, 1, 2);

    gridLayout->addWidget(new QLabel("New value (dec):", this), 1, 0);
    gridLayout->addWidget(new QLabel("New value (hex):", this), 2, 0);

    mLineEditDecValue = new QLineEdit(this);
    mLineEditHexValue = new QLineEdit(this);
    mLineEditHexValue->setDisabled(true);
    QObject::connect(mLineEditDecValue, &QLineEdit::textChanged, this, &DialogEditSimpleValue::decValueChanged);

    switch (mFieldType)
    {
        case MemoryFieldType::Byte:
        {
            mLineEditDecValue->setValidator(new QIntValidator((std::numeric_limits<int8_t>::min)(), (std::numeric_limits<int8_t>::max)(), this));
            int8_t v = Script::Memory::ReadByte(mMemoryAddress);
            mLineEditDecValue->setText(QString("%1").arg(v));
            break;
        }
        case MemoryFieldType::UnsignedByte:
        {
            mLineEditDecValue->setValidator(new QIntValidator((std::numeric_limits<uint8_t>::min)(), (std::numeric_limits<uint8_t>::max)(), this));
            uint8_t v = Script::Memory::ReadByte(mMemoryAddress);
            mLineEditDecValue->setText(QString("%1").arg(v));
            break;
        }
        case MemoryFieldType::Word:
        {
            mLineEditDecValue->setValidator(new QIntValidator((std::numeric_limits<int16_t>::min)(), (std::numeric_limits<int16_t>::max)(), this));
            int16_t v = Script::Memory::ReadWord(mMemoryAddress);
            mLineEditDecValue->setText(QString("%1").arg(v));
            break;
        }
        case MemoryFieldType::UTF16Char:
        case MemoryFieldType::UnsignedWord:
        {
            mLineEditDecValue->setValidator(new QIntValidator((std::numeric_limits<uint16_t>::min)(), (std::numeric_limits<uint16_t>::max)(), this));
            uint16_t v = Script::Memory::ReadWord(mMemoryAddress);
            mLineEditDecValue->setText(QString("%1").arg(v));
            break;
        }
        case MemoryFieldType::Dword:
        {
            mLineEditDecValue->setValidator(new QIntValidator((std::numeric_limits<int32_t>::min)(), (std::numeric_limits<int32_t>::max)(), this));
            int32_t v = Script::Memory::ReadDword(mMemoryAddress);
            mLineEditDecValue->setText(QString("%1").arg(v));
            break;
        }
        case MemoryFieldType::UnsignedDword:
        case MemoryFieldType::StringsTableID:
        {
            // TODO: test why is this commented out?
            // mLineEditDecValue->setValidator(new QIntValidator((std::numeric_limits<uint32_t>::min)(), (std::numeric_limits<uint32_t>::max)(), this));
            uint32_t v = Script::Memory::ReadDword(mMemoryAddress);
            mLineEditDecValue->setText(QString("%1").arg(v));
            break;
        }
        case MemoryFieldType::Qword:
        {
            // mLineEditDecValue->setValidator(new QIntValidator((std::numeric_limits<int64_t>::min)(), (std::numeric_limits<int64_t>::max)(), this));
            int64_t v = Script::Memory::ReadQword(mMemoryAddress);
            mLineEditDecValue->setText(QString("%1").arg(v));
            break;
        }
        case MemoryFieldType::UnsignedQword:
        {
            // mLineEditDecValue->setValidator(new QIntValidator((std::numeric_limits<uint64_t>::min)(), (std::numeric_limits<uint64_t>::max)(), this));
            uint64_t v = Script::Memory::ReadQword(mMemoryAddress);
            mLineEditDecValue->setText(QString("%1").arg(v));
            break;
        }
        case MemoryFieldType::Float:
        {
            mLineEditDecValue->setValidator(new QDoubleValidator((std::numeric_limits<float>::max)() * -1, (std::numeric_limits<float>::max)(), 1000, this));
            uint32_t tmp = Script::Memory::ReadDword(mMemoryAddress);
            float v = reinterpret_cast<float&>(tmp);
            mLineEditDecValue->setText(QString("%1").arg(v));
            break;
        }
        case MemoryFieldType::Double:
        {
            mLineEditDecValue->setValidator(new QDoubleValidator((std::numeric_limits<double>::max)() * -1, (std::numeric_limits<double>::max)(), 1000, this));
            size_t tmp = Script::Memory::ReadQword(mMemoryAddress);
            double v = reinterpret_cast<double&>(tmp);
            mLineEditDecValue->setText(QString("%1").arg(v));
            break;
        }
    }
    gridLayout->addWidget(mLineEditDecValue, 1, 1);
    gridLayout->addWidget(mLineEditHexValue, 2, 1);

    // BUTTONS
    auto buttonLayout = new QHBoxLayout();

    auto cancelBtn = new QPushButton("Cancel", this);
    QObject::connect(cancelBtn, &QPushButton::clicked, this, &DialogEditSimpleValue::cancelBtnClicked);
    cancelBtn->setAutoDefault(false);
    auto changeBtn = new QPushButton("Change", this);
    QObject::connect(changeBtn, &QPushButton::clicked, this, &DialogEditSimpleValue::changeBtnClicked);
    changeBtn->setAutoDefault(true);

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(changeBtn);

    layout->addLayout(gridLayout);
    layout->addStretch();
    layout->addLayout(buttonLayout);

    mLineEditDecValue->setFocus();
    mLineEditDecValue->selectAll();
}

QSize S2Plugin::DialogEditSimpleValue::minimumSizeHint() const
{

    return QSize(350, 150);
}

QSize S2Plugin::DialogEditSimpleValue::sizeHint() const
{
    return minimumSizeHint();
}

void S2Plugin::DialogEditSimpleValue::cancelBtnClicked()
{
    reject();
}

void S2Plugin::DialogEditSimpleValue::changeBtnClicked()
{
    switch (mFieldType)
    {
        case MemoryFieldType::Byte:
        {
            int8_t v = mLineEditDecValue->text().toInt();
            Script::Memory::WriteByte(mMemoryAddress, v);
            break;
        }
        case MemoryFieldType::UnsignedByte:
        {
            uint8_t v = mLineEditDecValue->text().toInt();
            Script::Memory::WriteByte(mMemoryAddress, v);
            break;
        }
        case MemoryFieldType::Word:
        {
            int16_t v = mLineEditDecValue->text().toShort();
            Script::Memory::WriteWord(mMemoryAddress, v);
            break;
        }
        case MemoryFieldType::UnsignedWord:
        case MemoryFieldType::UTF16Char:
        {
            uint16_t v = mLineEditDecValue->text().toUShort();
            Script::Memory::WriteWord(mMemoryAddress, v);
            break;
        }
        case MemoryFieldType::Dword:
        {
            int32_t v = mLineEditDecValue->text().toLong();
            Script::Memory::WriteDword(mMemoryAddress, v);
            break;
        }
        case MemoryFieldType::UnsignedDword:
        case MemoryFieldType::StringsTableID:
        {
            uint32_t v = mLineEditDecValue->text().toULong();
            Script::Memory::WriteDword(mMemoryAddress, v);
            break;
        }
        case MemoryFieldType::Qword:
        {
            int64_t v = mLineEditDecValue->text().toLongLong();
            Script::Memory::WriteQword(mMemoryAddress, v);
            break;
        }
        case MemoryFieldType::UnsignedQword:
        {
            uint64_t v = mLineEditDecValue->text().toULongLong();
            Script::Memory::WriteQword(mMemoryAddress, v);
            break;
        }
        case MemoryFieldType::Float:
        {
            float v = mLineEditDecValue->text().toFloat();
            uint32_t tmp = reinterpret_cast<uint32_t&>(v);
            Script::Memory::WriteDword(mMemoryAddress, tmp);
            break;
        }
        case MemoryFieldType::Double:
        {
            double v = mLineEditDecValue->text().toDouble();
            size_t tmp = reinterpret_cast<size_t&>(v);
            Script::Memory::WriteQword(mMemoryAddress, tmp);
            break;
        }
    }
    accept();
}

void S2Plugin::DialogEditSimpleValue::decValueChanged()
{
    std::stringstream ss;
    switch (mFieldType)
    {
        case MemoryFieldType::Byte:
        {
            int8_t v = mLineEditDecValue->text().toInt();
            ss << QString::asprintf("0x%02x", static_cast<uint8_t>(v)).toStdString();
            break;
        }
        case MemoryFieldType::UnsignedByte:
        {
            uint8_t v = mLineEditDecValue->text().toInt();
            ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(v);
            break;
        }
        case MemoryFieldType::Word:
        {
            int16_t v = mLineEditDecValue->text().toShort();
            ss << "0x" << std::hex << std::setw(4) << std::setfill('0') << v;
            break;
        }
        case MemoryFieldType::UnsignedWord:
        case MemoryFieldType::UTF16Char:
        {
            uint16_t v = mLineEditDecValue->text().toUShort();
            ss << "0x" << std::hex << std::setw(4) << std::setfill('0') << v;
            break;
        }
        case MemoryFieldType::Dword:
        {
            int32_t v = mLineEditDecValue->text().toLong();
            ss << "0x" << std::hex << std::setw(8) << std::setfill('0') << v;
            break;
        }
        case MemoryFieldType::UnsignedDword:
        case MemoryFieldType::StringsTableID:
        {
            uint32_t v = mLineEditDecValue->text().toULong();
            ss << "0x" << std::hex << std::setw(8) << std::setfill('0') << v;
            break;
        }
        case MemoryFieldType::Qword:
        {
            int64_t v = mLineEditDecValue->text().toLongLong();
            ss << "0x" << std::hex << std::setw(16) << std::setfill('0') << v;
            break;
        }
        case MemoryFieldType::UnsignedQword:
        {
            uint64_t v = mLineEditDecValue->text().toULongLong();
            ss << "0x" << std::hex << std::setw(16) << std::setfill('0') << v;
            break;
        }
        case MemoryFieldType::Float:
        {
            float v = mLineEditDecValue->text().toFloat();
            uint32_t tmp = reinterpret_cast<uint32_t&>(v);
            ss << "0x" << std::hex << std::setw(8) << std::setfill('0') << tmp;
            break;
        }
        case MemoryFieldType::Double:
        {
            double v = mLineEditDecValue->text().toDouble();
            size_t tmp = reinterpret_cast<size_t&>(v);
            ss << "0x" << std::hex << std::setw(16) << std::setfill('0') << tmp;
            break;
        }
    }
    mLineEditHexValue->setText(QString::fromStdString(ss.str()));
}
