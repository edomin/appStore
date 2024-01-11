#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QThread>
#include <memory>

class QTimer;
class QSettings;
class QTcpSocket;

namespace appstoreapp
{
class Client : public QThread
{
    Q_OBJECT
private:
    const int interval_ {1000 * 2};
    std::unique_ptr<QTimer> connectTimerPtr_   {nullptr};
    std::shared_ptr<QSettings> appSettingsPtr_ {nullptr};
    std::unique_ptr<QTcpSocket> tcpSocketPtr_  {nullptr};
protected:
    virtual void run()override;
public:
    explicit Client(std::shared_ptr<QSettings> appSettingsPtr,QObject *parent = nullptr);
    virtual ~Client();

public slots:
    void getInstalledApps();
    void aptInstallApp(QString packageName);
    void aptRemoveApp(QString packageName);
    void startApp(QString packageName);
    void startAppUrl(QString packageName);
private slots:
    void timeoutSlot();
    void connectedSlot();
    void disconnectedSlot();
    void readyReadSlot();
    void finishedSlot();
signals:
    void notifyResult(QString method,QString text);
};
}

#endif // CLIENT_H
