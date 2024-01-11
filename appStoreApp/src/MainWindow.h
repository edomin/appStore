#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>


class QSettings;
class QWebChannel;
class QWebEnginePage;
class QWebEngineView;
class QWebSocketServer;
class WebSocketClientWrapper;

namespace appstoreapp
{
class Client;
class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    std::shared_ptr<QSettings> appSettingsPtr_ {nullptr};
    QWebChannel* webChannelPtr        {nullptr};
    QWebEnginePage* webEnginePagePtr_ {nullptr};
    QWebEngineView* webEngineViewPtr_ {nullptr};

    std::shared_ptr<Client> clientPtr_ {nullptr};
    std::shared_ptr<QWebSocketServer> webSocketServerPtr_ {nullptr};
    std::shared_ptr<WebSocketClientWrapper> webSocketClientWrapperPtr_ {nullptr};
public:
    explicit MainWindow(std::shared_ptr<QSettings> appSettingsPtr,QWidget *parent = nullptr);
    virtual ~MainWindow();

signals:

};
}


#endif // MAINWINDOW_H
