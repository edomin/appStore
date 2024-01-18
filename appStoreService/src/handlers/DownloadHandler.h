#ifndef DOWNLOADHANDLER_H
#define DOWNLOADHANDLER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>

#include <memory>

namespace appstoreservice {

class DownloadHandler : public QThread
{
    Q_OBJECT
public:
    explicit DownloadHandler(QObject *parent = nullptr);
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
    QNetworkAccessManager          netAccess_;
    std::shared_ptr<QNetworkReply> replyPtr_ {nullptr};
    QByteArray                     downloadedData_;
};

} // namespace appstoreservice

#endif // DOWNLOADHANDLER_H
