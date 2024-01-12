#include "TcpHandler.h"

#include <QSettings>
#include <QTcpServer>
#include <QTcpSocket>

void appstoreservice::TcpHandler::run()
{
    QThread::exec();
}

appstoreservice::TcpHandler::TcpHandler(std::shared_ptr<QSettings> appSettingsPtr, QObject *parent)
    :QThread{parent},appSettingsPtr_{appSettingsPtr}
{
}

appstoreservice::TcpHandler::~TcpHandler()
{
    QThread::quit();
    QThread::wait();
}
