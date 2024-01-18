#include "DownloadHandler.h"

appstoreservice::DownloadHandler::DownloadHandler(QObject *parent)
    : QThread{parent}
{}

const QByteArray& appstoreservice::DownloadHandler::getDownloadedData() const {
    return downloadedData_;
}

void appstoreservice::DownloadHandler::download(const QString& url) {
    auto *reply = netAccess_.get(QNetworkRequest{url});
    if (!reply->error())
        replyPtr_ = std::shared_ptr<QNetworkReply>{reply};
}

void appstoreservice::DownloadHandler::abort() {
    if (replyPtr_)
        replyPtr_->abort();
}

void appstoreservice::DownloadHandler::fileDownloaded(QNetworkReply *reply) {
    downloadedData_ = reply->readAll();
    emit downloaded(downloadedData_);
    qWarning("Downloaded");
}

void appstoreservice::DownloadHandler::run()
{
    QObject::connect(&netAccess_, SIGNAL(finished(QNetworkReply *)),
                     this, SLOT(fileDownloaded(QNetworkReply *)));
    QThread::exec();
}
