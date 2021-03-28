#pragma once

#include "ieditablewidget.h"
#include "sliderwidget.h"

#include <QFormLayout>
#include <QImage>
#include <QPushButton>
#include <QWidget>

class ColorWidget : public QWidget, public virtual IEditableWidget
{
    Q_OBJECT
public:
    explicit ColorWidget(QWidget* parent = nullptr);

    virtual void clearFocus() override;
    virtual void resetValues() override;

    void setRed(float value);
    void setGreen(float value);
    void setBlue(float value);
    void setBrightness(float value);
    void setContrast(float value);

    auto getRed() const -> float;
    auto getGreen() const -> float;
    auto getBlue() const -> float;
    auto getBrightness() const -> float;
    auto getContrast() const -> float;

signals:
    void grayScaleClicked();
    void colorChanged();

private:
    QVBoxLayout* const  vlayout;
    QFormLayout* const  formLayout;
    QPushButton* const  grayScalePushButton;
    SliderWidget* const redSlider;
    SliderWidget* const greenSlider;
    SliderWidget* const blueSlider;
    SliderWidget* const brightSlider;
    SliderWidget* const contrastSlider;
    
    void setupGrayScale();
    void setupSliders();
};
