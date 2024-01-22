#ifndef DOWNLOADHANDLER_H
#define DOWNLOADHANDLER_H

#include <QNetworkReply>
#include <QProcess>
#include <QThread>

#include <memory>

class QNetworkAccessManager;
class QTemporaryFile;

namespace appstoreservice {

class DownloadHandler : public QThread
{
    Q_OBJECT
public:
    using DownloadStartCbk = std::function<void(void)>;
    using DownloadProgressCbk = std::function<void(int)>;
    using DownloadFinishCbk = std::function<void(void)>;
    using InstallStartCbk = std::function<void(void)>;
    using InstallFinishCbk = std::function<void(void)>;
    using ErrorCbk = std::function<void(const QString& msg)>;

    explicit DownloadHandler(const QString& pkgName, const QString& url, QObject *parent = nullptr);
    void setCallbacks(
        DownloadStartCbk downloadStartCbk,
        DownloadProgressCbk downloadProgressCbk,
        DownloadFinishCbk downloadFinishCbk,
        InstallStartCbk installStartCbk,
        InstallFinishCbk installFinishCbk,
        ErrorCbk errorCbk
    );
    ~DownloadHandler();

public slots:
    void abort();

protected:
    virtual void run() override;

private slots:
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloaded(QNetworkReply *reply);
    void networkError(QNetworkReply::NetworkError);
    void installStarted();
    void installFinished(int exitcode, QProcess::ExitStatus exitStatus);
    void readYumStderr();

private:
    const QString pkgName_;
    const QString url_;
    std::shared_ptr<QNetworkAccessManager> netAccessPtr_ {nullptr};
    std::shared_ptr<QNetworkReply> replyPtr_ {nullptr};
    std::shared_ptr<QTemporaryFile> pkgFilePtr_ {nullptr};
    std::shared_ptr<QProcess> processPtr_ {nullptr};
    QByteArray yumStderr_ {};

    DownloadStartCbk downloadStartCbk_ {nullptr};
    DownloadProgressCbk downloadProgressCbk_ {nullptr};
    DownloadFinishCbk downloadFinishCbk_ {nullptr};
    InstallStartCbk installStartCbk_ {nullptr};
    InstallFinishCbk installFinishCbk_ {nullptr};
    ErrorCbk errorCbk_ {nullptr};
};

} // namespace appstoreservice

#endif // DOWNLOADHANDLER_H
