#include "confirmwidget.h"

ConfirmWidget::ConfirmWidget(QWidget* parent)
    : QWidget{ parent }
    , layout{ new QFormLayout{ this }}
    , buttonLayout{ new QHBoxLayout}
    , okButton{ new QPushButton{ "Ok", this}}
    , cancelButton{ new QPushButton{ "Cancel", this}}
{
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addRow(buttonLayout);

    connect(okButton, &QPushButton::pressed, this, [this]{ emit actionConfirmed(); });
    connect(cancelButton, &QPushButton::pressed, this, [this]{ emit actionCancelled(); });
}

