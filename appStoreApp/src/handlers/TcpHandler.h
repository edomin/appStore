#ifndef CLIENT_H
#define CLIENT_H

#include <QThread>
#include <memory>

class QTimer;
class QSettings;
class QTcpSocket;
class QJsonObject;

namespace appstoreapp
{
class TcpHandler : public QThread
{
    Q_OBJECT
private:
    enum class Commands
    {
        GET_INSTALLED,
        INSTALL_APP,
        REMOVE_APP,
        START_APP,
        START_APP_URL,
        UNKNOWN
    };
    const int interval_ {1000 * 2};
    std::shared_ptr<QTimer> connectTimerPtr_   {nullptr};
    std::shared_ptr<QSettings> appSettingsPtr_ {nullptr};
    std::shared_ptr<QTcpSocket> tcpSocketPtr_  {nullptr};
    void sendToService(const QJsonObject& requestObject);
protected:
    virtual void run()override;
public:
    explicit TcpHandler(std::shared_ptr<QSettings> appSettingsPtr,QObject *parent = nullptr);
    virtual ~TcpHandler();

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
