#pragma once

#include <ieditor.h>
#include "ieditablewidget.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>

class SettingsWidget : public QWidget, public virtual IEditableWidget
{
    Q_OBJECT
public:
    enum InterpIndex { NEAREST = 0, BILINEAR = 1, COUNT };

    explicit SettingsWidget(IEditor::InterpMethod interpMethod, QWidget* parent = nullptr);

    // inherited via IEditableWidget
    virtual void clearFocus() override;
    virtual void resetValues() override { }

    auto toInterpIndex(IEditor::InterpMethod method) const -> std::optional<InterpIndex>;

signals:
    void interpChanged(InterpIndex index);
    void overlayColorChanged(const QString& msg);

private:
    QFormLayout* const layout;

    QHBoxLayout* const interpLayout;
    QLabel* const      interpLabel;
    QComboBox* const   interpComboBox;

    QHBoxLayout* const overlayColorLayout;
    QLabel* const      overlayColorLabel;
    QComboBox* const   overlayColorComboBox;    

    void setupInterp(IEditor::InterpMethod method);
    void setupOverlayColor();
};
