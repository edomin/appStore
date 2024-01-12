#ifndef TCPHANDLER_H
#define TCPHANDLER_H

#include <QThread>
#include <memory>

class QSettings;
class QTcpServer;
class QTcpSocket;
namespace appstoreservice
{
class TcpHandler : public QThread
{
    Q_OBJECT
private:
    std::shared_ptr<QSettings> appSettingsPtr_ {nullptr};
    std::shared_ptr<QTcpServer> tcpServerPtr_  {nullptr};
    std::shared_ptr<QTcpSocket> tcpSocketPtr_  {nullptr};
protected:
    virtual void run()override;
public:
    explicit TcpHandler(std::shared_ptr<QSettings> appSettingsPtr,QObject *parent = nullptr);
    virtual ~TcpHandler();
};
}

#endif // TCPHANDLER_H
