#include "mainwindow.h"
#include "settingswidget.h"
#include "ui_mainwindow.h"
#include <logger.h>
#include "openglexception.h"
#include <util.h>

#include <chrono>
#include <memory>
#include <QDebug>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QPushButton>
#include <QVBoxLayout>

#include <QShortcut>

using namespace util::types;
 
const int                   MainWindow::defaultWindowWidth{ 1366 };
const int                   MainWindow::defaultWindowHeight{ 768 };
const int                   MainWindow::messageTimeout{ 5000 };
const IEditor::InterpMethod MainWindow::defaultInterpMethod{ IEditor::InterpMethod::BILINEAR };

MainWindow::MainWindow(bool debug, QWidget *parent)
    : QMainWindow{ parent }
    , editor{ fact::makeEditor(defaultInterpMethod, debug) }
    , ui{ util::make_owner<Ui::MainWindow>() }
    , resizeTimer{ new QTimer{ this }}
    , aboutDialog{ new AboutDialog{ this }}
    , keybindingsDialog{ new KeybindingsDialog{ this }}
    , affineDockWidget{ new QDockWidget{ this }}
    , affineWidget{ new AffineWidget{ this }}
    , colorDockWidget{ new QDockWidget{ this }}
    , colorWidget{ new ColorWidget{ this }}
    , settingsDockWidget{ new QDockWidget{ this }}
    , settingsWidget{ new SettingsWidget{ defaultInterpMethod, this }}
    , confirmDockWidget{ new QDockWidget{ this }}
    , confirmWidget{ new ConfirmWidget{ this }}
    , statusBar{ new StatusBar{ this }}
{
    try
    {
        displayWidget = new DisplayWidget{ this, *this };
    }
    catch (const OpenGLException& ex)
    {
        Logger::error(ex.what());
        popupError(QString{ "Failed to initialize OpenGL, aborting. Error: " } + ex.what());
        quit();
    }
    
    ui->setupUi(this);
    setupDisplayWidget();
    setupActions();
    setupAffineWidget();
    setupColorWidget();
    setupSettingsWidget();
    setupConfirmWidget();
    setupStatusBar();

    Logger::setDebug(debug);

    connect(&Logger::getInstance(), &Logger::logToView, this, &MainWindow::status);
    connect(resizeTimer, &QTimer::timeout, this, [this] {
        enableWidgetUpdates(true);
        update();
        resizeTimer->stop();
    });
}

MainWindow::~MainWindow()
{
    ui.reset();
}

auto MainWindow::getImage() -> std::optional<QImage>
{
    return editor->getImage();
}

auto MainWindow::getColorData() -> ColorData
{
    const auto brightness = colorWidget->getBrightness() - 0.5f;
    auto contrast = colorWidget->getContrast();
    contrast = contrast <= 0.5f ? 2.0f * contrast : 8.0f * contrast - 3.0f;

    return { colorWidget->getRed(), colorWidget->getGreen(), colorWidget->getBlue(), brightness, contrast };
}
    
auto MainWindow::mergeImages(QImage lower, QImage upper, QRect upperRect, float upperAngle) -> QImage
{
    return editor->mergeImages(lower, upper, upperRect, upperAngle);
}

void MainWindow::setupDockWidget(QDockWidget* const dockWidget, QWidget* const widget, const QString& title, Qt::DockWidgetArea area)
{
    dockWidget->setMinimumSize({ 250, 0 });
    dockWidget->setWindowTitle(title);
    dockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
    dockWidget->setWidget(widget);
    addDockWidget(area, dockWidget);
}

void MainWindow::setupActions()
{
    auto openShortcut = new QShortcut{ QKeySequence{ tr("Ctrl+O", "File|Open") }, this };
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::open);
    connect(openShortcut, &QShortcut::activated, this, &MainWindow::open);

    auto saveShortcut = new QShortcut{ QKeySequence{ tr("Ctrl+S", "File|Save") }, this };
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::save);
    connect(saveShortcut, &QShortcut::activated, this, &MainWindow::save);

    auto quitShortcut = new QShortcut{ QKeySequence{ tr("Ctrl+Q", "File|Quit") }, this };
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::quit);
    connect(quitShortcut, &QShortcut::activated, this, &MainWindow::quit);

    auto zoomInShortcut = new QShortcut{ QKeySequence{ tr("Ctrl+N") }, this };
    connect(zoomInShortcut, &QShortcut::activated, this, &MainWindow::zoomIn);
    
    auto zoomOutShortcut = new QShortcut{ QKeySequence{ tr("Ctrl+P") }, this };
    connect(zoomOutShortcut, &QShortcut::activated, this, &MainWindow::zoomOut);
    
    auto undoShortcut = new QShortcut{ QKeySequence{ tr("Ctrl+Z", "Edit|Undo") }, this };
    connect(ui->actionUndo, &QAction::triggered, this, &MainWindow::undo);
    connect(undoShortcut, &QShortcut::activated, this, &MainWindow::undo);
    
    auto redoShortcut = new QShortcut{ QKeySequence{ tr("Ctrl+Y", "Edit|Redo") }, this };
    connect(ui->actionRedo, &QAction::triggered, this, &MainWindow::redo);
    connect(redoShortcut, &QShortcut::activated, this, &MainWindow::redo);
    
    auto confirmShortcut = new QShortcut{ QKeySequence{ tr("Ctrl+Space") }, this };
    connect(confirmWidget, &ConfirmWidget::actionConfirmed, this, &MainWindow::confirmAction);
    connect(confirmShortcut, &QShortcut::activated, this, &MainWindow::confirmAction);
    
    auto cancelShortcut = new QShortcut{ QKeySequence{ tr("Ctrl+G") }, this };
    connect(confirmWidget, &ConfirmWidget::actionCancelled, this, &MainWindow::cancelAction);
    connect(cancelShortcut, &QShortcut::activated, this, &MainWindow::cancelAction);
    
    auto toggleDockShortcut = new QShortcut{ QKeySequence{ tr("Ctrl+H") }, this };
    connect(toggleDockShortcut, &QShortcut::activated, this, &MainWindow::toggleDock);
    
    auto copyShortcut = new QShortcut{ QKeySequence{ tr("Ctrl+C", "Edit|Copy") }, this };
    connect(ui->actionCopy, &QAction::triggered,this, &MainWindow::copy);
    connect(copyShortcut, &QShortcut::activated, this, &MainWindow::copy);
    
    auto cutShortcut = new QShortcut{ QKeySequence{ tr("Ctrl+X", "Edit|Cut") }, this };
    connect(ui->actionCut, &QAction::triggered,this, &MainWindow::cut);
    connect(cutShortcut, &QShortcut::activated, this, &MainWindow::cut);

    auto rotate90LeftShortcut = new QShortcut{ QKeySequence{ tr("Ctrl+Left") }, this };
    connect(ui->actionRotate90Left, &QAction::triggered, [this]{ rotate(90.0f); });
    connect(rotate90LeftShortcut, &QShortcut::activated, [this]{ rotate(90.0f); });

    auto rotate90RightShortcut = new QShortcut{ QKeySequence{ tr("Ctrl+Right") }, this };
    connect(ui->actionRotate90Right, &QAction::triggered, [this]{ rotate(-90.0f); });
    connect(rotate90RightShortcut, &QShortcut::activated, [this]{ rotate(-90.0f); });

    auto rotate180Shortcut = new QShortcut{ QKeySequence{ tr("Ctrl+Down") }, this };
    connect(ui->actionRotate180, &QAction::triggered, [this]{ rotate(180.0f); });
    connect(rotate180Shortcut, &QShortcut::activated, [this]{ rotate(180.0f); });

    auto resetRotationShortcut = new QShortcut{ QKeySequence{ tr("Ctrl+Up") }, this };
    connect(resetRotationShortcut, &QShortcut::activated, [this]{  resetRotation(); });

    connect(ui->actionMirrorHorizontally, &QAction::triggered, this, &MainWindow::mirrorHorizontally);
    connect(ui->actionMirrorVertically, &QAction::triggered, this, &MainWindow::mirrorVertically);
    connect(ui->actionAbout, &QAction::triggered, [this]{ aboutDialog->show(); });
    connect(ui->actionKeybindings, &QAction::triggered, [this]{ keybindingsDialog->show(); });
}

void MainWindow::setupDisplayWidget()
{
    setCentralWidget(displayWidget);

    const auto desktopSize = QApplication::desktop()->availableGeometry();

    const auto w = std::min(defaultWindowWidth, desktopSize.width());
    const auto h = std::min(defaultWindowHeight, desktopSize.height());

    centralWidget()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    if (w >= 1366 || h >= 768)
        resize(w, h);

    connect(displayWidget, &DisplayWidget::zoomChanged, [this](float zoom){ setZoom(toInt(zoom * 100.0f)); });

    connect(displayWidget, &DisplayWidget::displayFocused, this, [this] {
        for (auto w : getEditableWidgets())
            w->clearFocus();
    });

    connect(displayWidget, &DisplayWidget::cursorPositionChanged, statusBar, &StatusBar::setCursorPosition);

    connect(displayWidget, &DisplayWidget::actionStarted, this, [this] {
        colorWidget->resetValues();
        colorDockWidget->setEnabled(false);
        confirmDockWidget->setEnabled(true);
    });
}

void MainWindow::setupAffineWidget()
{
    setupDockWidget(affineDockWidget, affineWidget, "Affine");

    affineDockWidget->setEnabled(false);

    connect(affineWidget, &AffineWidget::rotateChanged, this, [this](int val) {
        if (!displayWidget->rotate(toFloat(val)))
            status("Failed to rotate!");
        else
            confirmDockWidget->setEnabled(true);
    });
}

void MainWindow::setupColorWidget()
{
    setupDockWidget(colorDockWidget, colorWidget, "Color");

    colorDockWidget->setEnabled(false);

    connect(colorWidget, &ColorWidget::grayScaleClicked, this, [this] {
        displayWidget->setGrayscale(true);

        QImage img;
        try
        {
            img = displayWidget->mergeLayers();
        }
        catch (const OpenGLException& ex)
        {
            Logger::error(ex.what());
            popupError(QString{ "Failed to save image! (" } + ex.what() + ")");
            return;
        }
        
        displayWidget->displayImage(img);
        editor->appendHistory(img);
        confirmDockWidget->setEnabled(true);

        displayWidget->setGrayscale(false);
    });
    
    connect(colorWidget, &ColorWidget::colorChanged, this, [this] {
        confirmDockWidget->setEnabled(true);
        displayWidget->update();
    });
}

void MainWindow::setupSettingsWidget()
{
    setupDockWidget(settingsDockWidget, settingsWidget, "Settings");
    settingsDockWidget->setEnabled(false);

    connect(settingsWidget, &SettingsWidget::interpChanged, this, [this](SettingsWidget::InterpIndex index) {
        const auto interpMethod = toInterpMethod(index);
        if (!interpMethod)
            return;
        
        editor->setInterpolationMethod(*interpMethod);
    });

    connect(settingsWidget, &SettingsWidget::overlayColorChanged, this, [this](const QString& msg) {
        if (msg == "Dark")
            displayWidget->setOverlayColor( DisplayWidget::darkOverlayColor );
        else
            displayWidget->setOverlayColor( DisplayWidget::lightOverlayColor );

        status("Switched to " + msg + " overlay color");
    });
}

void MainWindow::setupConfirmWidget()
{
    setupDockWidget(confirmDockWidget, confirmWidget, "Confirm");
    confirmDockWidget->setMaximumHeight(100);
    confirmDockWidget->setEnabled(false);

    connect(confirmWidget, &ConfirmWidget::actionConfirmed, this, &MainWindow::confirmAction);
    connect(confirmWidget, &ConfirmWidget::actionCancelled, this, &MainWindow::cancelAction);
}

void MainWindow::setupStatusBar()
{
    ui->statusbar->addWidget(statusBar);
    statusBar->setEnabled(false);

    connect(statusBar, &StatusBar::zoomChanged, this, [this](float val) {
        displayWidget->setZoom(val);
        setZoom(toInt(val * 100.0));
    });
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    using namespace std::chrono_literals;
    
    resizeTimer->start(150ms);
    enableWidgetUpdates(false);
    QMainWindow::resizeEvent(event);
}

void MainWindow::open()
{
    const auto filePath = QFileDialog::getOpenFileName(this, "Open Image", "./",
                                                       "Images (*.png *.bmp *.ppm *.xpm *.jpg)");
    if (!editor->loadImage(filePath))
    {
        status("Image not opened");
        return;
    }

    const auto img = editor->getImage();
    if (!img)
    {
        status("Invalid image!");
        return;
    }
            
    loadImage(*img);
                
    // enable UI
    ui->actionSave->setEnabled(true);
    ui->menuEdit->setEnabled(true);
    ui->menuImage->setEnabled(true);
    statusBar->setEnabled(true);

    const auto fileInfo = QFileInfo{ filePath };
    const auto fileName = fileInfo.completeBaseName() + "." + fileInfo.suffix();
    const auto fileResolution = QString::number(img->width()) + "x" + QString::number(img->height());
    const auto fileSize = this->locale().formattedDataSize(fileInfo.size());
    statusBar->setDetails(fileName, fileResolution, fileSize);
    
    setWindowTitle("Image Editor - " + fileName);
}

void MainWindow::save()
{
    const auto fileName = QFileDialog::getSaveFileName(this);
    editor->saveImage(fileName);
    setWindowTitle("Image Editor - " + fileName);
}

void MainWindow::quit()
{
    QApplication::quit();
}

void MainWindow::zoomIn()
{
    const auto z = displayWidget->zoomIn();
    if (!z)
        return;

    setZoom(toInt(*z * 100.0f));
}

void MainWindow::zoomOut()
{
    const auto z = displayWidget->zoomOut();
    if (!z)
        return;

    setZoom(toInt(*z * 100.0f));
}

void MainWindow::undo()
{
    const auto img = editor->undo();
    if (!img)
        return;

    displayWidget->displayImage(*img);
    colorDockWidget->setEnabled(true);
    confirmDockWidget->setEnabled(false);

    status("Undo");
}

void MainWindow::redo()
{
    const auto img = editor->redo();
    if (!img)
        return;

    displayWidget->displayImage(*img);

    status("Redo");
}

void MainWindow::confirmAction()
{
    QImage img;
    
    try
    {
        img = displayWidget->mergeLayers();
    }
    catch (OpenGLException& ex)
    {
        Logger::error(ex.what());
        popupError(QString{ "Internal Error! Failed to confirm action (" } + ex.what() + ")");
        return;
    }
    
    displayWidget->displayImage(img);
    editor->appendHistory(img);

    resetSettings();
    
    colorDockWidget->setEnabled(true);
    confirmDockWidget->setEnabled(false);
}

void MainWindow::cancelAction()
{
    if (!displayWidget->revertChanges())
        return;
    
    resetSettings();
    colorDockWidget->setEnabled(true);
    confirmDockWidget->setEnabled(false);
    
    status("Action cancelled");
}

void MainWindow::toggleDock()
{
    for (auto dockWidget : getDockWidgets())
    {
        if (dockWidget->isVisible())
            dockWidget->hide();
        else
            dockWidget->show();
    }
}

void MainWindow::copy()
{
    if (!displayWidget->copy())
    {
        status("Failed to copy!");
        displayWidget->revertChanges();
        colorDockWidget->setEnabled(true);
        confirmDockWidget->setEnabled(false);
        return;
    }

    status("Copying");
}

void MainWindow::cut()
{
    if (!displayWidget->cut())
    {
        status("Failed to cut!");
        displayWidget->revertChanges();
        colorDockWidget->setEnabled(true);
        confirmDockWidget->setEnabled(false);
        return;
    }
    
    status("Cutting");
}

void MainWindow::mirrorHorizontally()
{
    auto img = displayWidget->getBackgroundImage();
    if (!img)
    {
        status("Failed to rotate!");
        return;
    }

    *img = img->mirrored(true, false);

    loadImage(*img);
    editor->appendHistory(*img);
}

void MainWindow::mirrorVertically()
{
    auto img = displayWidget->getBackgroundImage();
    if (!img)
    {
        status("Failed to rotate!");
        return;
    }

    *img = img->mirrored(false, true);

    loadImage(*img);
    editor->appendHistory(*img);
}

void MainWindow::loadImage(const QImage& img)
{
    displayWidget->displayImage(img);
    displayWidget->setZoom(displayWidget->getDefaultZoom());
    statusBar->setZoom(toInt(displayWidget->getDefaultZoom() * 100.0f));

    status("Image loaded");
    setEditingDocksEnabled(true);
}

void MainWindow::resetSettings()
{
    for (auto w : std::array<IEditableWidget*, 2>{ affineWidget, colorWidget })
        w->resetValues();
}
    
void MainWindow::setZoom(int zoom)
{
    statusBar->setZoom(zoom);
}

void MainWindow::setEditingDocksEnabled(bool enable)
{
    for (auto dockWidget : getEditingDockWidgets())
        dockWidget->setEnabled(enable);
}

void MainWindow::enableWidgetUpdates(bool enable)
{
    for (auto dockWidget : getDockWidgets())
    {
        if (enable)
            dockWidget->show();
        else
            dockWidget->hide();
    }

    setUpdatesEnabled(enable);
    displayWidget->setUpdatesEnabled(enable);
}

void MainWindow::rotate(float angle)
{
    const auto rotate = displayWidget->getRotate();
    if (!displayWidget->rotate(rotate + angle))
        status("Rotation failed!");
    else
        confirmDockWidget->setEnabled(true);
}

void MainWindow::resetRotation()
{
    if (!displayWidget->rotate(0.0f))
        status("Rotation failed!");
    else
        confirmDockWidget->setEnabled(true);
}

auto MainWindow::getEditingDockWidgets() const -> std::vector<QDockWidget*>
{
    return { affineDockWidget, colorDockWidget,settingsDockWidget };
}

auto MainWindow::getDockWidgets() const -> std::vector<QDockWidget*>
{
    auto docks = std::vector<QDockWidget*>{ getEditingDockWidgets() };
    docks.push_back(confirmDockWidget);
    return docks;
}

auto MainWindow::getEditableWidgets() const -> std::vector<IEditableWidget*>
{
    return { affineWidget, colorWidget, settingsWidget, statusBar };
}

auto MainWindow::toInterpMethod(SettingsWidget::InterpIndex index) const -> std::optional<IEditor::InterpMethod>
{
    switch (index)
    {
        case SettingsWidget::InterpIndex::NEAREST:  return IEditor::InterpMethod::NEAREST;
        case SettingsWidget::InterpIndex::BILINEAR: return IEditor::InterpMethod::BILINEAR;
        default:                                    return {};
    }
}

auto MainWindow::zoomToString(float zoom) const -> QString
{
    return QString::number(100 * zoom) + " %";
}

auto MainWindow::popupInformation(const QString& message, const QString& title) -> int
{
    return QMessageBox::information(this, title, message, QMessageBox::Ok, QMessageBox::Ok);
}

auto MainWindow::popupWarning(const QString& message, const QString& title) -> int
{
    return QMessageBox::warning(this, title, message, QMessageBox::Ok, QMessageBox::Ok);
}

auto MainWindow::popupError(const QString& message, const QString& title) -> int
{
    return QMessageBox::critical(this, title, message, QMessageBox::Ok, QMessageBox::Ok);
}

void MainWindow::status(const QString& message)
{
    statusBar->showMessage(message);
    Logger::debug(message);
}
