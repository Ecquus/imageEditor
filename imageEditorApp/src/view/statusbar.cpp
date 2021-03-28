#include "statusbar.h"
#include <util.h>

using namespace util::types;

StatusBar::StatusBar(QWidget* parent)
    : QStatusBar{ parent }
    , positionLabel{ new QLabel{ "(0, 0)", this }}
    , zoomComboBox{ new QComboBox{ this }}
    , zoomValidator{ new QRegExpValidator{ this }}
    , detailsLabel{ new QLabel{ "", this }}
    , statusLabel{ new QLabel{ "Status:", this }}
{
    const QStringList zoomPresets {
        "25%",  "50%",  "75%",  "100%", "125%",
        "150%", "200%", "400%", "800%", "1600%"
    };
    
    zoomComboBox->addItems(zoomPresets);
    zoomComboBox->setCurrentIndex(zoomComboBox->findText("100%"));
    
    zoomComboBox->setEditable(true);

    zoomValidator->setRegExp(QRegExp{ "^[0-9]+%$" });
    zoomComboBox->setValidator(zoomValidator);

    connect(zoomComboBox, &QComboBox::currentTextChanged, [this](const QString& text){
        if (!zoomValidator->regExp().exactMatch(text))
            return;

        bool ok{ false };
        const auto zoom = text.left(text.length() - 1).toInt(&ok);
        if (!ok)
            return;
        
        showZoomMessage(zoom);
        emit zoomChanged(toFloat(zoom) / 100.0f);
    });
    
    addWidget(positionLabel);
    addWidget(makeSeparatorWidget());
    addWidget(zoomComboBox);
    addWidget(makeSeparatorWidget());
    addWidget(detailsLabel);
    addWidget(makeSeparatorWidget());
    addWidget(statusLabel);
}

auto StatusBar::makeSeparatorWidget() -> QWidget*
{
    auto widget = new QWidget{ this };
    widget->setFixedWidth(50);
    widget->setHidden(1);
    widget->setVisible(1);

    return widget;
}

void StatusBar::setZoom(int zoom)
{
    zoomComboBox->setCurrentText(QString::number(zoom) + "%");
    showZoomMessage(zoom);
}

void StatusBar::setCursorPosition(const QPoint& pos)
{
    positionLabel->setText("(" + QString::number(pos.x()) + ", " + QString::number(pos.y()) + ")");
}

void StatusBar::setDetails(const QString& fileName, const QString& fileResolution, const QString& fileSize)
{
    detailsLabel->setText(fileName + " (" + fileResolution + ", " + fileSize + " )");
}

void StatusBar::showMessage(const QString& msg)
{
    statusLabel->setText("Status: " + msg);
}

void StatusBar::showZoomMessage(int zoom)
{
    showMessage("Set zoom to " + QString::number(zoom) + "%");
}
