#include "Views/ViewState.h"

#include "Configuration.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtPlugin.h"
#include "Spelunky2.h"
#include <QHeaderView>
#include <QLabel>

S2Plugin::ViewState::ViewState(uintptr_t state, QWidget* parent) : QWidget(parent), mState(state)
{
    initializeUI();
    setWindowIcon(getCavemanIcon());
    setWindowTitle("State");
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddressDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);
    toggleAutoRefresh(Qt::Checked);
}

void S2Plugin::ViewState::initializeUI()
{
    mMainLayout = new QVBoxLayout(this);
    mRefreshLayout = new QHBoxLayout();
    mMainLayout->addLayout(mRefreshLayout);

    mRefreshButton = new QPushButton("Refresh", this);
    mRefreshLayout->addWidget(mRefreshButton);
    QObject::connect(mRefreshButton, &QPushButton::clicked, this, &ViewState::refreshState);

    mAutoRefreshTimer = std::make_unique<QTimer>(this);
    QObject::connect(mAutoRefreshTimer.get(), &QTimer::timeout, this, &ViewState::refreshState);

    mAutoRefreshCheckBox = new QCheckBox("Auto-refresh every", this);
    mAutoRefreshCheckBox->setCheckState(Qt::Checked);
    mRefreshLayout->addWidget(mAutoRefreshCheckBox);
    QObject::connect(mAutoRefreshCheckBox, &QCheckBox::clicked, this, &ViewState::toggleAutoRefresh);

    mAutoRefreshIntervalLineEdit = new QLineEdit(this);
    mAutoRefreshIntervalLineEdit->setFixedWidth(50);
    mAutoRefreshIntervalLineEdit->setValidator(new QIntValidator(100, 5000, this));
    mAutoRefreshIntervalLineEdit->setText("100");
    mRefreshLayout->addWidget(mAutoRefreshIntervalLineEdit);
    QObject::connect(mAutoRefreshIntervalLineEdit, &QLineEdit::textChanged, this, &ViewState::autoRefreshIntervalChanged);

    mRefreshLayout->addWidget(new QLabel("milliseconds", this));

    mRefreshLayout->addStretch();

    auto labelButton = new QPushButton("Label", this);
    QObject::connect(labelButton, &QPushButton::clicked, this, &ViewState::label);
    mRefreshLayout->addWidget(labelButton);

    mMainTreeView = new TreeViewMemoryFields(this);
    mMainTreeView->addMemoryFields(Configuration::get()->typeFields(MemoryFieldType::State), "State", mState);

    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex);
    mMainLayout->addWidget(mMainTreeView);

    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->updateTableHeader();

    mMainLayout->setMargin(5);
    mMainTreeView->setVisible(true);
}

void S2Plugin::ViewState::refreshState()
{
    mMainTreeView->updateTree();
}

void S2Plugin::ViewState::toggleAutoRefresh(int newState)
{
    if (newState == Qt::Unchecked)
    {
        mAutoRefreshTimer->stop();
        mRefreshButton->setEnabled(true);
    }
    else
    {
        mAutoRefreshTimer->setInterval(mAutoRefreshIntervalLineEdit->text().toUInt());
        mAutoRefreshTimer->start();
        mRefreshButton->setEnabled(false);
    }
}

void S2Plugin::ViewState::autoRefreshIntervalChanged(const QString& text)
{
    if (mAutoRefreshCheckBox->checkState() == Qt::Checked)
    {
        mAutoRefreshTimer->setInterval(text.toUInt());
    }
}

QSize S2Plugin::ViewState::sizeHint() const
{
    return QSize(750, 1050);
}

QSize S2Plugin::ViewState::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewState::label()
{
    mMainTreeView->labelAll();
}
