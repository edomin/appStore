#ifndef WEBHANDLER_H
#define WEBHANDLER_H

#include <QObject>
#include <memory>

class QSettings;
namespace appstoreservice
{
class WebHandler : public QObject
{
    Q_OBJECT
private:
    std::shared_ptr<QSettings> appSettingsPtr_ {nullptr};
public:
    explicit WebHandler(std::shared_ptr<QSettings> appSettingsPtr,QObject *parent = nullptr);
    virtual ~WebHandler()=default;
public slots:
    void getInstalledApps();
    void aptInstallApp(QString packageName);
    void aptRemoveApp(QString packageName);
    void startApp(QString packageName);
    void startAppUrl(QString packageName);

signals:
    void notifyResult(QString method,QString text);
};
}

#endif // WEBHANDLER_H
