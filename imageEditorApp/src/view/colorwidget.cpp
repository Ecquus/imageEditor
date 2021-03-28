#include "colorwidget.h"

#include <cmath>
#include <QPainter>
#include <QPaintEvent>

ColorWidget::ColorWidget(QWidget* parent)
    : QWidget{ parent }
    , vlayout{ new QVBoxLayout{ this  }}
    , formLayout{ new QFormLayout }
    , grayScalePushButton{ new QPushButton{ "Grayscale", this }}
    , redSlider{ new SliderWidget { "Red", { 0, 100 }, 50,  this }}
    , greenSlider{ new SliderWidget { "Green", { 0, 100 }, 50,  this }}
    , blueSlider{ new SliderWidget { "Blue", { 0, 100 }, 50,  this }}
    , brightSlider{ new SliderWidget { "Brightness", { 0, 100 }, 50,  this }}
    , contrastSlider{ new SliderWidget { "Contrast", { 0, 100 }, 50,  this }}
{
    setLayout(vlayout);

    setupGrayScale();
    setupSliders();
    vlayout->addLayout(formLayout);
}

void ColorWidget::setupGrayScale()
{
    formLayout->addRow(grayScalePushButton);
    connect(grayScalePushButton, &QPushButton::clicked, this, [this]{ emit grayScaleClicked(); });
}

void ColorWidget::setupSliders()
{
    for (auto slider : { redSlider, greenSlider, blueSlider, brightSlider, contrastSlider })
    {
        formLayout->addRow(slider);
        connect(slider, &SliderWidget::valueChanged, this, [this]{ emit colorChanged(); });   
    }
}

void ColorWidget::clearFocus()
{
    for (auto slider : { redSlider, greenSlider, blueSlider, brightSlider, contrastSlider })
        slider->clearFocus();
}

void ColorWidget::resetValues()
{
    for (auto slider : { redSlider, greenSlider, blueSlider, brightSlider, contrastSlider })
        slider->resetValues();
}

void ColorWidget::setRed(float value)
{
    redSlider->setValue(int(std::round(value * 100.0f)));
}

void ColorWidget::setGreen(float value)
{
    greenSlider->setValue(int(std::round(value * 100.0f)));
}

void ColorWidget::setBlue(float value)
{
    blueSlider->setValue(int(std::round(value * 100.0f)));
}

void ColorWidget::setBrightness(float value)
{
    brightSlider->setValue(int(std::round(value * 100.0f)));
}

void ColorWidget::setContrast(float value)
{
    contrastSlider->setValue(int(std::round(value * 100.0f)));
}

auto ColorWidget::getRed() const -> float
{
    return static_cast<float>(redSlider->getValue()) / 100.0f;
}

auto ColorWidget::getGreen() const -> float
{
    return static_cast<float>(greenSlider->getValue()) / 100.0f;
}

auto ColorWidget::getBlue() const -> float
{
    return static_cast<float>(blueSlider->getValue()) / 100.0f;
}

auto ColorWidget::getBrightness() const -> float
{
    return static_cast<float>(brightSlider->getValue()) / 100.0f;
}

auto ColorWidget::getContrast() const -> float
{
    return static_cast<float>(contrastSlider->getValue()) / 100.0f;
}
