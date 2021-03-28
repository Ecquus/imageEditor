#include "affinewidget.h"

#include <cmath>

AffineWidget::AffineWidget(QWidget* parent)
    : QWidget{ parent }
    , formLayout{ new QFormLayout{ this }}
    , rotateSlider{ new SliderWidget{ "Rotate", { -179, 180 }, 0, this }}
{
    setupRotate();
}

void AffineWidget::setupRotate()
{
    formLayout->addRow(rotateSlider);

    connect(rotateSlider, &SliderWidget::valueChanged, this, [this](int val){
        emit rotateChanged(val);
    });
}

void AffineWidget::clearFocus()
{
    rotateSlider->clearFocus();
}

void AffineWidget::resetValues()
{
    rotateSlider->resetValues();
}

auto AffineWidget::getRotate() const -> float
{
    return static_cast<float>(rotateSlider->getValue()) / 100.0f;
}

void AffineWidget::setRotate(float angle)
{
    rotateSlider->setValue(int(std::round(angle)));
}
