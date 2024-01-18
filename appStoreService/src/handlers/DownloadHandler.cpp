#include "DownloadHandler.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

appstoreservice::DownloadHandler::DownloadHandler(QObject *parent)
    : QThread{parent}
{}

appstoreservice::DownloadHandler::~DownloadHandler() {
    QThread::quit();
    QThread::wait();
}

const QByteArray& appstoreservice::DownloadHandler::getDownloadedData() const {
    return downloadedData_;
}

void appstoreservice::DownloadHandler::download(const QString& url) {
    auto *reply = netAccessPtr_->get(QNetworkRequest{url});
    if (!reply->error())
        replyPtr_.reset(reply);
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
    netAccessPtr_.reset(new QNetworkAccessManager);
    QObject::connect(netAccessPtr_.get(), &QNetworkAccessManager::finished,
                     this, &appstoreservice::DownloadHandler::fileDownloaded);
    QThread::exec();
}
