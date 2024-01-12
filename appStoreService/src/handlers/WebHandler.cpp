#include "WebHandler.h"


appstoreservice::WebHandler::WebHandler(std::shared_ptr<QSettings> appSettingsPtr, QObject *parent)
    :QObject{parent},appSettingsPtr_{appSettingsPtr}
{
}

void appstoreservice::WebHandler::getInstalledApps()
{
    qDebug("Executing 'getInstalledApps'");
}

void appstoreservice::WebHandler::aptInstallApp(QString packageName)
{
    qDebug("Executing 'aptInstallApp'");
}

void appstoreservice::WebHandler::aptRemoveApp(QString packageName)
{
    qDebug("Executing 'aptRemoveApp'");
}

void appstoreservice::WebHandler::startApp(QString packageName)
{
    qDebug("Executing 'startApp'");
}

void appstoreservice::WebHandler::startAppUrl(QString packageName)
{
    qDebug("Executing 'startAppUrl'");
}
