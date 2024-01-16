#include "TcpHandler.h"

#include <QtGlobal>
#include <QSettings>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QJsonObject>
#include <QHostAddress>
#include <QJsonDocument>
#include <map>

void appstoreservice::TcpHandler::getInstalledApps()
{
    qDebug("Executing 'getInstalledApps'");
}

void appstoreservice::TcpHandler::aptInstallApp(const QJsonObject &requestObject)
{
    qDebug("Executing 'aptInstallApp'; Request: %s",qPrintable(QJsonDocument(requestObject).toJson()));
}

void appstoreservice::TcpHandler::aptRemoveApp(const QJsonObject &requestObject)
{
    qDebug("Executing 'aptRemoveApp'; Request: %s",qPrintable(QJsonDocument(requestObject).toJson()));
}

void appstoreservice::TcpHandler::startApp(const QJsonObject &requestObject)
{
    qDebug("Executing 'startApp'; Request: %s",qPrintable(QJsonDocument(requestObject).toJson()));
}

void appstoreservice::TcpHandler::startAppUrl(const QJsonObject& requestObject)
{
    qDebug("Executing 'startAppUrl'; Request: %s",qPrintable(QJsonDocument(requestObject).toJson()));
}

void appstoreservice::TcpHandler::run()
{
    const auto tcpAddress {appSettingsPtr_->value("tcpAddress").toString().isEmpty() ?
                    QHostAddress::LocalHost :
                    QHostAddress(appSettingsPtr_->value("tcpAddress").toString())};
    const auto tcpPort {appSettingsPtr_->value("tcpPort").toInt()};
    tcpServerPtr_.reset(new QTcpServer);
    const auto listen {tcpServerPtr_->listen(tcpAddress,tcpPort)};
    if(!listen){
        qWarning("Fail to listen on: :%s:%d",qPrintable(tcpServerPtr_->serverAddress().toString()),tcpPort);
        return;
    }
    qInfo("Start listening on :%s:%d",qPrintable(tcpServerPtr_->serverAddress().toString()),tcpPort);
    QObject::connect(tcpServerPtr_.get(),&QTcpServer::newConnection,
                     this,&TcpHandler::newConnectionSlot,Qt::DirectConnection);
    QThread::exec();
}

appstoreservice::TcpHandler::TcpHandler(std::shared_ptr<QSettings> appSettingsPtr, QObject *parent)
    : QThread(parent),
      appSettingsPtr_{appSettingsPtr}
{
}

appstoreservice::TcpHandler::~TcpHandler()
{
    QThread::quit();
    QThread::wait();
}

void appstoreservice::TcpHandler::newConnectionSlot()
{
    tcpSocketPtr_.reset(tcpServerPtr_->nextPendingConnection());
    QObject::connect(tcpSocketPtr_.get(),&QTcpSocket::disconnected,
                     this,&TcpHandler::disconnectSlot,Qt::DirectConnection);
    QObject::connect(tcpSocketPtr_.get(),&QTcpSocket::readyRead,
                     this,&TcpHandler::readyReadSlot,Qt::DirectConnection);
    qDebug("TcpSocket connected on: %s:%d",
           qPrintable(tcpSocketPtr_->localAddress().toString()),tcpSocketPtr_->localPort());
}

void appstoreservice::TcpHandler::disconnectSlot()
{
    if(tcpSocketPtr_){
        const auto socketState {tcpSocketPtr_->state()};
        if(socketState==QAbstractSocket::ConnectedState){
            tcpSocketPtr_->disconnectFromHost();
        }
    }
    qDebug("TcpSocket disconnected!");
}

void appstoreservice::TcpHandler::readyReadSlot()
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
    const auto requestObject {QJsonDocument::fromJson(data).object()};
    if(!requestObject.isEmpty()){
        const auto key {requestObject.value("command").toString()};
        const auto command {getCommand(key)};
        switch(command){
        case TcpHandler::Commands::GET_INSTALLED:
            getInstalledApps();
            break;
        case TcpHandler::Commands::INSTALL_APP:
            aptInstallApp(requestObject);
            break;
        case TcpHandler::Commands::REMOVE_APP:
            aptRemoveApp(requestObject);
            break;
        case TcpHandler::Commands::START_APP:
            startApp(requestObject);
            break;
        case TcpHandler::Commands::START_APP_URL:
            startAppUrl(requestObject);
            break;
        case TcpHandler::Commands::UNKNOWN:
            qWarning("Unknown command: '%s'",qPrintable(key));
            break;
        }
        return;
    }
    qWarning("Empty request object!");
}
