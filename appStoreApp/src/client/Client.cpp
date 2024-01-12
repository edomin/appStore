#include "Client.h"

#include <QTimer>
#include <QtGlobal>
#include <QSettings>
#include <QTcpSocket>
#include <QByteArray>
#include <QJsonObject>
#include <QHostAddress>
#include <QJsonDocument>

void appstoreapp::Client::sendToService(const QJsonObject &requestObject)
{
    const auto socketState {tcpSocketPtr_->state()};
    if(socketState!=QAbstractSocket::ConnectedState){
        qWarning("Tcp socket not connected to service!");
        return;
    }
    const auto WRITE_TIMEOUT {1000};
    const auto data {QJsonDocument{requestObject}.toJson()};
    tcpSocketPtr_->write(data);
    const auto written {tcpSocketPtr_->waitForBytesWritten(WRITE_TIMEOUT)};
    if(!written){
        qWarning("Write to TcpSocket failed by timeout!");
    }
    qInfo("Requext sended success, content: %s",qPrintable(QJsonDocument(requestObject).toJson()));
}

void appstoreapp::Client::run()
{
    connectTimerPtr_.reset(new QTimer);
    connectTimerPtr_->setInterval(interval_);
    tcpSocketPtr_.reset(new QTcpSocket);
    QObject::connect(connectTimerPtr_.get(),&QTimer::timeout,
                     this,&Client::timeoutSlot,Qt::DirectConnection);
    QObject::connect(tcpSocketPtr_.get(),&QTcpSocket::connected,
                     this,&Client::connectedSlot,Qt::DirectConnection);
    connectTimerPtr_->start();
    QObject::connect(this,&QThread::finished,this,
                     &Client::finishedSlot,Qt::DirectConnection);
    QThread::exec();
}

appstoreapp::Client::Client(std::shared_ptr<QSettings> appSettingsPtr, QObject *parent)
    : QThread(parent),
      appSettingsPtr_{appSettingsPtr}
{    
}

appstoreapp::Client::~Client()
{
    QThread::quit();
    QThread::wait();
}

void appstoreapp::Client::getInstalledApps()
{
    qDebug("Get installed apps list");
    const QJsonObject requestObject {
        {"command","getInstalledApps"}
    };
    sendToService(requestObject);
}

void appstoreapp::Client::aptInstallApp(QString packageName)
{
    qDebug("Install app package: %s",qPrintable(packageName));
    const QJsonObject requestObject {
        {"command","aptInstallApp"},
        {"param",packageName}
    };
    sendToService(requestObject);
}

void appstoreapp::Client::aptRemoveApp(QString packageName)
{
    qDebug("Remove app package: %s",qPrintable(packageName));
    const QJsonObject requestObject {
        {"command","aptRemoveApp"},
        {"param",packageName}
    };
    sendToService(requestObject);
}

void appstoreapp::Client::startApp(QString packageName)
{
    qDebug("Start app package: %s",qPrintable(packageName));
    const QJsonObject requestObject {
        {"command","startApp"},
        {"param",packageName}
    };
    sendToService(requestObject);
}

void appstoreapp::Client::startAppUrl(QString packageName)
{
    qDebug("Start url app package: %s",qPrintable(packageName));
    const QJsonObject requestObject {
        {"command","startAppUrl"},
        {"param",packageName}
    };
    sendToService(requestObject);
}

void appstoreapp::Client::timeoutSlot()
{
    const auto CONNECT_TIMEOUT {100};
    const auto appStoreServiceAddress {appSettingsPtr_->value("appStoreServiceAddress").toString()};
    const auto appStoreServicePort {appSettingsPtr_->value("appStoreServicePort").toInt()};
    tcpSocketPtr_->connectToHost(appStoreServiceAddress,appStoreServicePort);
    const auto connected {tcpSocketPtr_->waitForConnected(CONNECT_TIMEOUT)};
    if(!connected){
        qWarning("Fail to connect to: %s:%d, error: %s",
                 qPrintable(appStoreServiceAddress),appStoreServicePort,qPrintable(tcpSocketPtr_->errorString()));
        return;
    }
    connectTimerPtr_->stop();
}

void appstoreapp::Client::connectedSlot()
{
    qDebug("Connected to: %s:%d",qPrintable(tcpSocketPtr_->peerAddress().toString()),tcpSocketPtr_->peerPort());
    QObject::connect(tcpSocketPtr_.get(),&QTcpSocket::disconnected,
                     this,&Client::disconnectedSlot,Qt::DirectConnection);
    QObject::connect(tcpSocketPtr_.get(),&QTcpSocket::readyRead,
                     this,&Client::readyReadSlot,Qt::DirectConnection);
    const QJsonObject requestObject{
        {"command","getInstalledApps"}
    };
    sendToService(requestObject);
}

void appstoreapp::Client::disconnectedSlot()
{
    if(!connectTimerPtr_){
        connectTimerPtr_.reset(new QTimer);
        connectTimerPtr_->setInterval(interval_);
    }
    connectTimerPtr_->start();
}

void appstoreapp::Client::readyReadSlot()
{
    const auto getCommand{[](const QString& key){
            const std::map<QString,Client::Commands> commandsMap {
                {"getInstalledApps",Client::Commands::GET_INSTALLED},
                {"aptInstallApp",Client::Commands::INSTALL_APP},
                {"aptRemoveApp",Client::Commands::REMOVE_APP},
                {"startApp",Client::Commands::START_APP},
                {"startAppUrl",Client::Commands::START_APP_URL},
            };
            const auto found {commandsMap.find(key)};
            if(found!=commandsMap.end()){
                return found->second;
            }
            return Client::Commands::UNKNOWN;
        }
    };
    const auto data {tcpSocketPtr_->readAll()};
    const auto responseObject {QJsonDocument::fromJson(data).object()};
    if(!responseObject.isEmpty()){
        const auto key {responseObject.value("command").toString()};
        const auto command {getCommand(key)};
        switch(command){
        case Client::Commands::GET_INSTALLED:
            break;
        case Client::Commands::INSTALL_APP:
            break;
        case Client::Commands::REMOVE_APP:
            break;
        case Client::Commands::START_APP:
            break;
        case Client::Commands::START_APP_URL:
            break;
        case Client::Commands::UNKNOWN:
            qWarning("Unknown command: '%s'",qPrintable(key));
            break;
        }
        return;
    }
    qWarning("Empty response object!");
}

void appstoreapp::Client::finishedSlot()
{
    if(connectTimerPtr_ && connectTimerPtr_->isActive()){
        connectTimerPtr_->stop();
    }
    if(tcpSocketPtr_){
        const auto DISCONNECT_TIMEOUT {500};
        const auto socketState {tcpSocketPtr_->state()};
        switch(socketState){
        case QAbstractSocket::ConnectedState:
            tcpSocketPtr_->disconnectFromHost();
            tcpSocketPtr_->waitForDisconnected(DISCONNECT_TIMEOUT);
            break;
        default:
            return;
        }
    }
}
