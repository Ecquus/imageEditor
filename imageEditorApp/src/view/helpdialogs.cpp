#include <util.h>
#include "helpdialogs.h"
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>

using namespace util::types;

HelpDialogBase::HelpDialogBase(QWidget* parent)
    : QDialog{ parent }
    , formLayout{ new QFormLayout{ this }}
    , dialogButtonBox{ new QDialogButtonBox{ QDialogButtonBox::Ok, Qt::Orientation::Horizontal, this }}
{
}

AboutDialog::AboutDialog(QWidget* parent)
    : HelpDialogBase{ parent }
{
    setWindowTitle("About");
    
    formLayout->addRow("Compiled With Qt Version:", new QLabel{ QT_VERSION_STR, this });
    
    // TODO: make crossplatform
    // formLayout->addRow("Compiled With GCC Version:", new QLabel{ __VERSION__, this });
    
    formLayout->addRow("Minimum Required OpenGL Version:", new QLabel{ "2.1", this });
    formLayout->addWidget(dialogButtonBox);

    connect(dialogButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

KeybindingsDialog::KeybindingsDialog(QWidget* parent)
    : HelpDialogBase{ parent }
{
    setWindowTitle("Keybindings");
    
    const auto keybindings = std::vector<std::pair<QString, QString>> {
        { "Ctrl + O"    , "Open" },
        { "Ctrl + S"    , "Save" },
        { "Ctrl + Q"    , "Quit" },
        { "Ctrl + N"    , "Zoom In" },
        { "Ctrl + P"    , "Zoom Out" },
        { "Ctrl + Z"    , "Undo" },
        { "Ctrl + Y"    , "Redo" },
        { "Ctrl + G"    , "Cancel" },
        { "Ctrl + Space", "Confirm" },
        { "Ctrl + H"    , "Hide Dock" },
        { "Ctrl + C"    , "Copy" },
        { "Ctrl + X"    , "Cut" },
        { "Ctrl + Left" , "Rotate left by 90 degrees" },
        { "Ctrl + Right", "Rotate right by 90 degrees" },
        { "Ctrl + Down" , "Rotate by 180 degrees" },
        { "Ctrl + Up"   , "Reset rotation" }
    };

    tableWidget = new QTableWidget{ toInt(keybindings.size()), 2, this };
    tableWidget->setHorizontalHeaderLabels({ "Keybindings", "Description" });
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->setMinimumHeight(300);

    for (uint i = 0; i < keybindings.size(); ++i)
    {
        const auto key  = keybindings[i].first;
        const auto desc = keybindings[i].second;
        addRow(toInt(i), key, desc);
    }

    formLayout->addWidget(tableWidget);
    formLayout->addWidget(dialogButtonBox);

    connect(dialogButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

void KeybindingsDialog::addRow(int row, const QString& keybind, const QString& description)
{
    const auto keyItem  = new QTableWidgetItem{ keybind };
    const auto descItem = new QTableWidgetItem{ description };

    keyItem->setFlags(keyItem->flags() & ~Qt::ItemIsEditable);
    descItem->setFlags(descItem->flags() & ~Qt::ItemIsEditable);

    tableWidget->setItem(row, 0, keyItem);
    tableWidget->setItem(row, 1, descItem);
}