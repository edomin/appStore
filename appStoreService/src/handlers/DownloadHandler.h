#ifndef DOWNLOADHANDLER_H
#define DOWNLOADHANDLER_H

#include <QThread>

#include <memory>

class QNetworkAccessManager;
class QNetworkReply;

namespace appstoreservice {

class DownloadHandler : public QThread
{
    Q_OBJECT
public:
    explicit DownloadHandler(QObject *parent = nullptr);
    ~DownloadHandler();
    const QByteArray& getDownloadedData() const;

public slots:
    void download(const QString& url);
    void abort();

protected:
    virtual void run() override;

signals:
    void downloaded(const QByteArray& data) const;

private slots:
    void fileDownloaded(QNetworkReply *reply);

private:
    std::shared_ptr<QNetworkAccessManager> netAccessPtr_;
    std::shared_ptr<QNetworkReply>         replyPtr_       {nullptr};
    QByteArray                             downloadedData_ {nullptr};
};

} // namespace appstoreservice

#endif // DOWNLOADHANDLER_H
