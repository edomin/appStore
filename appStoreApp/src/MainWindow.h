#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

class QSettings;
class QWebEnginePage;
class QWebEngineView;

namespace appstoreapp
{
class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    std::shared_ptr<QSettings> appSettingsPtr_ {nullptr};
    QWebEnginePage* webEnginePagePtr_ {nullptr};
    QWebEngineView* webEngineViewPtr_ {nullptr};

public:
    explicit MainWindow(std::shared_ptr<QSettings> appSettingsPtr,QWidget *parent = nullptr);
    virtual ~MainWindow()=default;
};
}

#endif // MAINWINDOW_H
