#include "TcpHandler.h"

#include <QTimer>
#include <QtGlobal>
#include <QSettings>
#include <QTcpSocket>
#include <QByteArray>
#include <QJsonObject>
#include <QHostAddress>
#include <QJsonDocument>
#include <QMetaObject>
#include <QAbstractEventDispatcher>

void appstoreapp::TcpHandler::sendToService(const QJsonObject &requestObject)
{
    const auto socketState {tcpSocketPtr_->state()};
    if(socketState!=QAbstractSocket::ConnectedState){
        qWarning("TcpSocket not connected to service!");
        return;
    }
    const auto WRITE_TIMEOUT {1000};
    const auto data {QJsonDocument{requestObject}.toJson()};
    QMetaObject::invokeMethod(QAbstractEventDispatcher::instance(tcpSocketPtr_->thread()),
        [=](){
            tcpSocketPtr_->write(data);
            const auto written {tcpSocketPtr_->waitForBytesWritten(WRITE_TIMEOUT)};
            if(!written){
                qWarning("Write to TcpSocket failed by timeout!");
            }
    });
    qInfo("Requext sended success, content: %s",qPrintable(QJsonDocument(requestObject).toJson()));
}

void appstoreapp::TcpHandler::run()
{
    connectTimerPtr_.reset(new QTimer);
    connectTimerPtr_->setInterval(interval_);
    tcpSocketPtr_.reset(new QTcpSocket);
    QObject::connect(connectTimerPtr_.get(),&QTimer::timeout,
                     this,&TcpHandler::timeoutSlot,Qt::DirectConnection);
    QObject::connect(tcpSocketPtr_.get(),&QTcpSocket::connected,
                     this,&TcpHandler::connectedSlot,Qt::DirectConnection);
    connectTimerPtr_->start();
    QObject::connect(this,&QThread::finished,this,
                     &TcpHandler::finishedSlot,Qt::DirectConnection);
    QThread::exec();
}

appstoreapp::TcpHandler::TcpHandler(std::shared_ptr<QSettings> appSettingsPtr, QObject *parent)
    : QThread(parent),
      appSettingsPtr_{appSettingsPtr}
{    
}

appstoreapp::TcpHandler::~TcpHandler()
{
    QThread::quit();
    QThread::wait();
}

void appstoreapp::TcpHandler::getInstalledApps()
{
    const QJsonObject requestObject {
        {"command","getInstalledApps"}
    };
    sendToService(requestObject);
}

void appstoreapp::TcpHandler::aptInstallApp(QString packageName)
{
    const QJsonObject requestObject {
        {"command","aptInstallApp"},
        {"param",packageName}
    };
    sendToService(requestObject);
}

void appstoreapp::TcpHandler::aptRemoveApp(QString packageName)
{
    const QJsonObject requestObject {
        {"command","aptRemoveApp"},
        {"param",packageName}
    };
    sendToService(requestObject);
}

void appstoreapp::TcpHandler::startApp(QString packageName)
{
    const QJsonObject requestObject {
        {"command","startApp"},
        {"param",packageName}
    };
    sendToService(requestObject);
}

void appstoreapp::TcpHandler::startAppUrl(QString packageName)
{
    const QJsonObject requestObject {
        {"command","startAppUrl"},
        {"param",packageName}
    };
    sendToService(requestObject);
}

void appstoreapp::TcpHandler::timeoutSlot()
{
    const auto CONNECT_TIMEOUT {100};
    const auto tcpAddress {appSettingsPtr_->value("tcpAddress").toString()};
    const auto tcpPort {appSettingsPtr_->value("tcpPort").toInt()};
    tcpSocketPtr_->connectToHost(tcpAddress,tcpPort);
    const auto connected {tcpSocketPtr_->waitForConnected(CONNECT_TIMEOUT)};
    if(!connected){
        qWarning("Fail to connect to: %s:%d, error: %s",
                 qPrintable(tcpAddress),tcpPort,qPrintable(tcpSocketPtr_->errorString()));
        return;
    }
    connectTimerPtr_->stop();
}

void appstoreapp::TcpHandler::connectedSlot()
{
    qDebug("Connected to: %s:%d",qPrintable(tcpSocketPtr_->peerAddress().toString()),tcpSocketPtr_->peerPort());
    QObject::connect(tcpSocketPtr_.get(),&QTcpSocket::disconnected,
                     this,&TcpHandler::disconnectedSlot,Qt::DirectConnection);
    QObject::connect(tcpSocketPtr_.get(),&QTcpSocket::readyRead,
                     this,&TcpHandler::readyReadSlot,Qt::DirectConnection);

    const QJsonObject requestObject{
        {"command","getInstalledApps"}
    };
    sendToService(requestObject);
}

void appstoreapp::TcpHandler::disconnectedSlot()
{
    if(!connectTimerPtr_){
        connectTimerPtr_.reset(new QTimer);
        connectTimerPtr_->setInterval(interval_);
    }
    connectTimerPtr_->start();
}

void appstoreapp::TcpHandler::readyReadSlot()
{
    const auto getCommand{[](const QString& key){
            const std::map<QString,TcpHandler::Commands> commandsMap {
                {"getInstalledApps",TcpHandler::Commands::GET_INSTALLED},
                {"aptInstallApp",TcpHandler::Commands::INSTALL_APP},
                {"aptRemoveApp",TcpHandler::Commands::REMOVE_APP},
                {"startApp",TcpHandler::Commands::START_APP},
                {"startAppUrl",TcpHandler::Commands::START_APP_URL},
            };
            const auto found {commandsMap.find(key)};
            if(found!=commandsMap.end()){
                return found->second;
            }
            return TcpHandler::Commands::UNKNOWN;
        }
    };
    const auto data {tcpSocketPtr_->readAll()};
    const auto responseObject {QJsonDocument::fromJson(data).object()};
    if(!responseObject.isEmpty()){
        const auto key {responseObject.value("command").toString()};
        const auto command {getCommand(key)};
        switch(command){
        case TcpHandler::Commands::GET_INSTALLED:
            break;
        case TcpHandler::Commands::INSTALL_APP:
            break;
        case TcpHandler::Commands::REMOVE_APP:
            break;
        case TcpHandler::Commands::START_APP:
            break;
        case TcpHandler::Commands::START_APP_URL:
            break;
        case TcpHandler::Commands::UNKNOWN:
            qWarning("Unknown command: '%s'",qPrintable(key));
            break;
        }
        return;
    }
    qWarning("Empty response object!");
}

void appstoreapp::TcpHandler::finishedSlot()
{
    if(connectTimerPtr_ && connectTimerPtr_->isActive()){
        connectTimerPtr_->stop();
    }
    if(tcpSocketPtr_){;
        const auto socketState {tcpSocketPtr_->state()};
        switch(socketState){
        case QAbstractSocket::ConnectedState:
            tcpSocketPtr_->disconnectFromHost();
            break;
        default:
            return;
        }
    }
}
