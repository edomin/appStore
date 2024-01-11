#include "Client.h"

#include <QTimer>
#include <QtGlobal>
#include <QSettings>
#include <QTcpSocket>
#include <QMetaObject>
#include <QAbstractEventDispatcher>

void appstoreapp::Client::run()
{
    connectTimerPtr_.reset(new QTimer);
    connectTimerPtr_->setInterval(interval_);
    tcpSocketPtr_.reset(new QTcpSocket);
    QObject::connect(connectTimerPtr_.get(),&QTimer::timeout,
                     this,&Client::timeoutSlot,Qt::DirectConnection);
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
    qInfo("Get installed apps list");
}

void appstoreapp::Client::aptInstallApp(QString packageName)
{
    qInfo("Install app package: %s",qPrintable(packageName));
}

void appstoreapp::Client::aptRemoveApp(QString packageName)
{
    qInfo("Remove app package: %s",qPrintable(packageName));
}

void appstoreapp::Client::startApp(QString packageName)
{
    qInfo("Start app package: %s",qPrintable(packageName));
}

void appstoreapp::Client::startAppUrl(QString packageName)
{
    qInfo("Start url app package: %s",qPrintable(packageName));
}

void appstoreapp::Client::timeoutSlot()
{
    const auto CONNECT_TIMEOUT {500};
    const auto appStoreServiceAddress {appSettingsPtr_->value("appStoreServiceAddress").toString()};
    const auto appStoreServicePort {appSettingsPtr_->value("appStoreServicePort").toInt()};
    tcpSocketPtr_->connectToHost(appStoreServiceAddress,appStoreServicePort);
    const auto connected {tcpSocketPtr_->waitForConnected(CONNECT_TIMEOUT)};
    if(connected){
        connectTimerPtr_->stop();
        qDebug("Connected to: %s:%d",qPrintable(appStoreServiceAddress),appStoreServicePort);
        QObject::connect(tcpSocketPtr_.get(),&QTcpSocket::disconnected,
                         this,&Client::disconnectedSlot,Qt::DirectConnection);
        QObject::connect(tcpSocketPtr_.get(),&QTcpSocket::readyRead,
                         this,&Client::readyReadSlot,Qt::DirectConnection);
        return;
    }
    qWarning("Fail to connect to: %s:%d, error: %s",
             qPrintable(appStoreServiceAddress),appStoreServicePort,qPrintable(tcpSocketPtr_->errorString()));
}

void appstoreapp::Client::connectedSlot()
{

}

void appstoreapp::Client::disconnectedSlot()
{

}

void appstoreapp::Client::readyReadSlot()
{

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
