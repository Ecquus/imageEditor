#pragma once

#include "ieditablewidget.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QStatusBar>

class StatusBar : public QStatusBar, public virtual IEditableWidget
{
    Q_OBJECT
public:
    explicit StatusBar(QWidget* parent = nullptr);

    // inherited via IEditableWidget
    virtual void clearFocus() override { zoomComboBox->clearFocus(); }
    virtual void resetValues() override { }
    
    void setZoom(int zoom);
    void setCursorPosition(const QPoint& pos);
    void setDetails(const QString& fileName, const QString& fileResolution, const QString& fileSize);
    void showMessage(const QString& msg);
    void showZoomMessage(int zoom);

signals:
    void zoomChanged(float val);

private:
    QLabel* const           positionLabel;
    QComboBox* const        zoomComboBox;
    QRegExpValidator* const zoomValidator;
    QLabel* const           detailsLabel;
    QLabel* const           statusLabel;

    auto makeSeparatorWidget() -> QWidget*;
};
