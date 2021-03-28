#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QTableWidget>

class HelpDialogBase : public QDialog
{
protected:
    QFormLayout* const      formLayout;
    QDialogButtonBox* const dialogButtonBox;

    explicit HelpDialogBase(QWidget* parent = nullptr);
};

class AboutDialog : public HelpDialogBase
{
public:
    explicit AboutDialog(QWidget* parent = nullptr);
};

class KeybindingsDialog : public HelpDialogBase
{
public:
    explicit KeybindingsDialog(QWidget* parent = nullptr);

private:
    QTableWidget* tableWidget;

    void addRow(int row, const QString& keybind, const QString& description);
};
