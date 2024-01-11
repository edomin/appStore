#include "Service.h"

#include <QtGlobal>
#include <QSettings>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QJsonObject>
#include <QHostAddress>
#include <QJsonDocument>
#include <map>

void appstoreservice::Service::getInstalledApps()
{
    qDebug("Executing 'getInstalledApps'");
}

void appstoreservice::Service::aptInstallApp(const QJsonObject &requestObject)
{
    qDebug("Executing 'aptInstallApp'");
}

void appstoreservice::Service::aptRemoveApp(const QJsonObject &requestObject)
{
    qDebug("Executing 'aptRemoveApp'");
}

void appstoreservice::Service::startApp(const QJsonObject &requestObject)
{
    qDebug("Executing 'startApp'");
}

void appstoreservice::Service::startAppUrl(const QJsonObject& requestObject)
{
    qDebug("Executing 'startAppUrl'");
}

void appstoreservice::Service::run()
{
    const auto serviceAddress {appSettingsPtr_->value("serviceAddress").toString()};
    const auto servicePort {appSettingsPtr_->value("servicePort").toInt()};
    tcpServerPtr_.reset(new QTcpServer);
    const auto listen {tcpServerPtr_->listen(serviceAddress.isEmpty() ? QHostAddress::LocalHost : QHostAddress(serviceAddress),servicePort)};
    if(!listen){
        qWarning("Fail to listen on: :%s:%d",qPrintable(tcpServerPtr_->serverAddress().toString()),servicePort);
        return;
    }
    qInfo("Startr listening on :%s:%d",qPrintable(tcpServerPtr_->serverAddress().toString()),servicePort);
    QObject::connect(tcpServerPtr_.get(),&QTcpServer::newConnection,
                     this,&Service::newConnectionSlot,Qt::DirectConnection);
    QThread::exec();
}

appstoreservice::Service::Service(std::shared_ptr<QSettings> appSettingsPtr, QObject *parent)
    : QThread(parent),
      appSettingsPtr_{appSettingsPtr}
{

}

appstoreservice::Service::~Service()
{
    QThread::quit();
    QThread::wait();
}

void appstoreservice::Service::newConnectionSlot()
{
    tcpSocketPtr_.reset(tcpServerPtr_->nextPendingConnection());
    QObject::connect(tcpSocketPtr_.get(),&QTcpSocket::disconnected,
                     this,&Service::disconnectSlot,Qt::DirectConnection);
    QObject::connect(tcpSocketPtr_.get(),&QTcpSocket::readyRead,
                     this,&Service::readyReadSlot,Qt::DirectConnection);
    qDebug("TcpSocket connected on: %s:%d",
           qPrintable(tcpSocketPtr_->localAddress().toString()),tcpSocketPtr_->localPort());
}

void appstoreservice::Service::disconnectSlot()
{
    if(tcpSocketPtr_){
        const auto socketState {tcpSocketPtr_->state()};
        if(socketState==QAbstractSocket::ConnectedState){
            tcpSocketPtr_->disconnectFromHost();
        }
    }
    qDebug("TcpSocket disconnected!");
}

void appstoreservice::Service::readyReadSlot()
{
    const auto getCommand{[](const QString& key){
            const std::map<QString,Service::Commands> commandsMap {
                {"getInstalledApps",Service::Commands::GET_INSTALLED},
                {"aptInstallApp",Service::Commands::INSTALL_APP},
                {"aptRemoveApp",Service::Commands::REMOVE_APP},
                {"startApp",Service::Commands::START_APP},
                {"startAppUrl",Service::Commands::START_APP_URL},
            };
            const auto found {commandsMap.find(key)};
            if(found!=commandsMap.end()){
                return found->second;
            }
            return Service::Commands::UNKNOWN;
        }
    };
    const auto data {tcpSocketPtr_->readAll()};
    const auto requestObject {QJsonDocument::fromJson(data).object()};
    if(!requestObject.isEmpty()){
        const auto key {requestObject.value("command").toString()};
        const auto command{getCommand(key)};
        switch(command){
        case Service::Commands::GET_INSTALLED:
            getInstalledApps();
            break;
        case Service::Commands::INSTALL_APP:
            aptInstallApp(requestObject);
            break;
        case Service::Commands::REMOVE_APP:
            aptRemoveApp(requestObject);
            break;
        case Service::Commands::START_APP:
            startApp(requestObject);
            break;
        case Service::Commands::START_APP_URL:
            startAppUrl(requestObject);
            break;
        case Service::Commands::UNKNOWN:
            qWarning("Unknown command: '%s'",qPrintable(key));
            break;
        }
    }
}
