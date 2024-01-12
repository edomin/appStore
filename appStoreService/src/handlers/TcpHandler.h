#ifndef SERVICE_H
#define SERVICE_H

#include <QThread>
#include <memory>

class QSettings;
class QTcpServer;
class QTcpSocket;
class QJsonObject;
namespace appstoreservice
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
    std::shared_ptr<QSettings> appSettingsPtr_ {nullptr};
    std::shared_ptr<QTcpServer> tcpServerPtr_  {nullptr};
    std::shared_ptr<QTcpSocket> tcpSocketPtr_  {nullptr};

    void getInstalledApps();
    void aptInstallApp(const QJsonObject& requestObject);
    void aptRemoveApp(const QJsonObject& requestObject);
    void startApp(const QJsonObject& requestObject);
    void startAppUrl(const QJsonObject& requestObject);
protected:
    virtual void run()override;
public:
    explicit TcpHandler(std::shared_ptr<QSettings> appSettingsPtr,QObject *parent = nullptr);
    virtual ~TcpHandler();
private slots:
    void newConnectionSlot();
    void disconnectSlot();
    void readyReadSlot();
signals:

};
}

#endif // SERVICE_H
