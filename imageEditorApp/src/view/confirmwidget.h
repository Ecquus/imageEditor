#pragma once

#include <QFormLayout>
#include <QHBoxLayout>
#include <QPushButton>

class ConfirmWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConfirmWidget(QWidget* parent = nullptr);

signals:
    void actionConfirmed();
    void actionCancelled();

private:
    QFormLayout* const layout;
    QHBoxLayout* const buttonLayout;
    QPushButton* const okButton;
    QPushButton* const cancelButton;
};
