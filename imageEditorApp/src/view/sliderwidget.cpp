#include "sliderwidget.h"

SliderWidget::SliderWidget(const QString&     labelText,
                          std::pair<int, int> sliderRange,
                          int                 defaultValue,
                          QWidget*            parent)
    : QWidget{ parent }
    , layout{ new QHBoxLayout{ this }}
    , label{ new QLabel{ labelText, this }}
    , spinBox{ new QSpinBox{ this }}
    , slider{ new QSlider{ Qt::Horizontal, this }}
    , defaultValue{ defaultValue }
{
    label->setMinimumWidth(75);

    slider->setRange(sliderRange.first, sliderRange.second);
    spinBox->setRange(sliderRange.first, sliderRange.second);

    slider->setValue(defaultValue);
    spinBox->setValue(defaultValue);
    
    layout->addWidget(label);
    layout->addWidget(spinBox);
    layout->addWidget(slider);

    connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), slider, &QSlider::setValue);
    connect(slider, &QSlider::valueChanged, spinBox, &QSpinBox::setValue);

    connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), this, [this](int val){
        emit valueChanged(val);
    });
}

void SliderWidget::setRange(int low, int high)
{
    slider->setRange(low, high);
    spinBox->setRange(low, high);
}

void SliderWidget::setValue(int value)
{
    slider->setValue(value);
    spinBox->setValue(value);
}

auto SliderWidget::getValue() const -> int
{
    return slider->value();
}

void SliderWidget::clearFocus()
{
    spinBox->clearFocus();
}

void SliderWidget::resetValues()
{
    spinBox->setValue(defaultValue);
}
