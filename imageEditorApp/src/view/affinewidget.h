#pragma once

#include "ieditablewidget.h"
#include "sliderwidget.h"

#include <QFormLayout>
#include <QWidget>

class AffineWidget : public QWidget, public virtual IEditableWidget
{
    Q_OBJECT
public:
    explicit AffineWidget(QWidget* parent = nullptr);
    ~AffineWidget() { }

    // inherited via IEditableWidget
    virtual void clearFocus() override;
    virtual void resetValues() override;

    auto getRotate() const -> float;
    void setRotate(float angle);

signals:
    void rotateChanged(int val);

private:
    QFormLayout* const  formLayout;
    SliderWidget* const rotateSlider;

    void setupRotate();
};
