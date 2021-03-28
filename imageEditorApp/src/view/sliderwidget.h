#pragma once

#include "ieditablewidget.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QSlider>
#include <QSpinBox>
#include <QWidget>

class SliderWidget : public QWidget, public virtual IEditableWidget
{
    Q_OBJECT
public:
    explicit SliderWidget(const QString& labelText, std::pair<int, int> sliderRange, int defaultValue, QWidget* parent);

    void setRange(int low, int high);
    void setValue(int value);
    auto getValue() const -> int;

    // inherited via IEditableWidget
    virtual void clearFocus() override;
    virtual void resetValues() override;

signals:
    void valueChanged(int value);

private:
    QHBoxLayout* const layout;
    QLabel* const      label;
    QSpinBox* const    spinBox;
    QSlider* const     slider;
    int                defaultValue;
};
