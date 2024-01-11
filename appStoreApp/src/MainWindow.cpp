#include "MainWindow.h"
#include "client/Client.h"
#include "shared/websockettransport.h"
#include "shared/websocketclientwrapper.h"

#include <QUrl>
#include <QWidget>
#include <QSettings>
#include <QVBoxLayout>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QWebSocketServer>

appstoreapp::MainWindow::MainWindow(std::shared_ptr<QSettings> appSettingsPtr, QWidget *parent)
    : QMainWindow{parent},
      appSettingsPtr_{appSettingsPtr},
      webChannelPtr{new QWebChannel{this}},
      webEnginePagePtr_{new QWebEnginePage{this}},
      webEngineViewPtr_{new QWebEngineView{this}},
      clientPtr_{new Client{appSettingsPtr}},
      webSocketServerPtr_{new QWebSocketServer("websocket_server",QWebSocketServer::NonSecureMode)},
      webSocketClientWrapperPtr_{new WebSocketClientWrapper(webSocketServerPtr_.get())}
{
    if(!webSocketServerPtr_->listen(QHostAddress::LocalHost,3000)){
        qFatal("Fail to start WebSocketServer!");
    }
    QObject::connect(webSocketClientWrapperPtr_.get(),&WebSocketClientWrapper::clientConnected,
                     webChannelPtr,&QWebChannel::connectTo);
    webChannelPtr->registerObject("clientapp",clientPtr_.get());
    clientPtr_->start();

    const auto appStoreUrl {appSettingsPtr_->value("appStoreUrl").toString()};
    webEnginePagePtr_->setUrl(QUrl(appStoreUrl));
    webEngineViewPtr_->setPage(webEnginePagePtr_);
    setCentralWidget(webEngineViewPtr_);
}

appstoreapp::MainWindow::~MainWindow()=default;

