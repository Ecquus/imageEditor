#include <util.h>
#include "settingswidget.h"
#include <ieditor.h>
#include <qcombobox.h>

using namespace util::types;

SettingsWidget::SettingsWidget(IEditor::InterpMethod interpMethod, QWidget* parent)
    : QWidget{ parent }
    , layout{ new QFormLayout{ this }}
    , interpLayout{ new QHBoxLayout }
    , interpLabel{ new QLabel{ "Interpolation", this }}
    , interpComboBox{ new QComboBox{ this }}      
    , overlayColorLayout{ new QHBoxLayout }
    , overlayColorLabel{ new QLabel{ "Selection", this }}
    , overlayColorComboBox{ new QComboBox{ this }}      
{
    setupInterp(interpMethod);
    setupOverlayColor();
}

void SettingsWidget::setupInterp(IEditor::InterpMethod method)
{
    interpComboBox->addItem("Nearest neighbour");
    interpComboBox->addItem("Bilinear");

    const auto index = toInterpIndex(method);
    if (!index)
        interpComboBox->setCurrentIndex(InterpIndex::NEAREST);
    else
        interpComboBox->setCurrentIndex(*index);

    interpLayout->addWidget(interpLabel);
    interpLayout->addWidget(interpComboBox);

    layout->addRow(interpLayout);

    connect(interpComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index < 0 || index >= InterpIndex::COUNT)
            return;
        
        emit interpChanged((InterpIndex)index);
    });
}

void SettingsWidget::setupOverlayColor()
{
    overlayColorComboBox->addItem("Dark");
    overlayColorComboBox->addItem("Light");

    overlayColorComboBox->setCurrentIndex(0);

    overlayColorLayout->addWidget(overlayColorLabel);
    overlayColorLayout->addWidget( overlayColorComboBox);

    layout->addRow(overlayColorLayout);

    connect(overlayColorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index < 0 || index >= 2)
            return;
        
        emit overlayColorChanged(overlayColorComboBox->itemText(index));
    });
}

auto SettingsWidget::toInterpIndex(IEditor::InterpMethod method) const -> std::optional<InterpIndex>
{
    switch (method)
    {
        case IEditor::InterpMethod::NEAREST:  return InterpIndex::NEAREST;
        case IEditor::InterpMethod::BILINEAR: return InterpIndex::BILINEAR;
        default:                              return {};
    }
}

void SettingsWidget::clearFocus()
{
    interpComboBox->clearFocus();
    overlayColorComboBox->clearFocus();
}
