#pragma once

#include "affinewidget.h"
#include "colorwidget.h"
#include "confirmwidget.h"
#include "displaywidget.h"
#include <editorfactory.h>
#include <ieditor.h>
#include "imainwindow.h"
#include "helpdialogs.h"
#include "settingswidget.h"
#include "statusbar.h"
#include <util.h>
#include "ui_mainwindow.h"

#include <QDockWidget>
#include <QMessageBox>
#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <qnamespace.h>
#include <qwidget.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow, public virtual IMainWindow
{
    Q_OBJECT
public:
    MainWindow(bool debug = false, QWidget *parent = nullptr);
    ~MainWindow();

    // inherited via IMainWindow
    virtual auto getImage() -> std::optional<QImage> override;
    virtual auto getColorData() -> ColorData override;
    virtual auto mergeImages(QImage lower, QImage upper, QRect upperRect, float upperAngle)
        -> QImage override;

protected:
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    static const int                   defaultWindowWidth;
    static const int                   defaultWindowHeight;
    static const int                   messageTimeout;
    static const IEditor::InterpMethod defaultInterpMethod;

    const std::unique_ptr<IEditor>  editor;
    util::owner_ptr<Ui::MainWindow> ui;
    QTimer* const                   resizeTimer;
    AboutDialog* const              aboutDialog;
    KeybindingsDialog* const        keybindingsDialog;
    DisplayWidget*                  displayWidget;
    QDockWidget* const              affineDockWidget;
    AffineWidget* const             affineWidget;
    QDockWidget* const              colorDockWidget;
    ColorWidget* const              colorWidget;
    QDockWidget* const              settingsDockWidget;
    SettingsWidget* const           settingsWidget;
    QDockWidget* const              confirmDockWidget;
    ConfirmWidget* const            confirmWidget;
    StatusBar* const                statusBar;

    void setupDockWidget(QDockWidget* const dockWidget, QWidget* const widget, const QString& title,
                         Qt::DockWidgetArea area = Qt::RightDockWidgetArea);

    void setupActions();
    void setupDisplayWidget();
    void setupAffineWidget();
    void setupColorWidget();
    void setupSettingsWidget();
    void setupConfirmWidget();
    void setupStatusBar();

    void open();
    void save();
    void quit();
    void zoomIn();
    void zoomOut();
    void undo();
    void redo();
    void confirmAction();
    void cancelAction();
    void toggleDock();
    void copy();
    void cut();
    void mirrorHorizontally();
    void mirrorVertically();
    
    void loadImage(const QImage& img);
    void resetSettings();
    void setZoom(int zoom);
    void setEditingDocksEnabled(bool enable);
    void enableWidgetUpdates(bool enable);
    void rotate(float angle);
    void resetRotation();

    auto getEditingDockWidgets() const -> std::vector<QDockWidget*>;
    auto getDockWidgets() const -> std::vector<QDockWidget*>;
    auto getEditableWidgets() const -> std::vector<IEditableWidget*>;
    
    auto toInterpMethod(SettingsWidget::InterpIndex index) const -> std::optional<IEditor::InterpMethod>;
    auto zoomToString(float zoom) const -> QString;

    auto popupInformation(const QString& message, const QString& title = "Information") -> int;
    auto popupWarning(const QString& message, const QString& title = "Warning") -> int;
    auto popupError(const QString& message, const QString& title = "Error") -> int;
    void status(const QString& message);
};
