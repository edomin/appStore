#include "DownloadHandler.h"

#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include <QTemporaryFile>

appstoreservice::DownloadHandler::DownloadHandler(
        const QString& pkgName,
        const QString& url,
        QObject *parent
)
    : QThread{parent}
    , pkgName_(pkgName)
    , url_(url)
{}

void appstoreservice::DownloadHandler::setCallbacks(
    DownloadStartCbk downloadStartCbk,
    DownloadProgressCbk downloadProgressCbk,
    DownloadFinishCbk downloadFinishCbk,
    InstallStartCbk installStartCbk,
    InstallFinishCbk installFinishCbk,
    ErrorCbk errorCbk
) {
    downloadStartCbk_ = downloadStartCbk;
    downloadProgressCbk_ = downloadProgressCbk;
    downloadFinishCbk_ = downloadFinishCbk;
    installStartCbk_ = installStartCbk;
    installFinishCbk_ = installFinishCbk;
    errorCbk_ = errorCbk;
}

appstoreservice::DownloadHandler::~DownloadHandler() {
    QThread::quit();
    QThread::wait();
}

void appstoreservice::DownloadHandler::abort() {
    if (replyPtr_)
        replyPtr_->abort();
}

void appstoreservice::DownloadHandler::downloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    if (downloadProgressCbk_)
        downloadProgressCbk_(100.0f / bytesTotal * bytesReceived);
}

void appstoreservice::DownloadHandler::downloaded(QNetworkReply *reply) {
    pkgFilePtr_->open();
    pkgFilePtr_->write(reply->readAll());
    pkgFilePtr_->close();
    if (downloadFinishCbk_)
        downloadFinishCbk_();

    processPtr_.reset(new QProcess());
    QObject::connect(processPtr_.get(), &QProcess::started, this, &DownloadHandler::installStarted);
    QObject::connect(processPtr_.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, &DownloadHandler::installFinished);
    QObject::connect(processPtr_.get(), &QProcess::readyReadStandardError, this, &DownloadHandler::readYumStderr);
    processPtr_->start("yum", QStringList() << "install" << "-y" << pkgFilePtr_->fileName());
}

void appstoreservice::DownloadHandler::networkError(QNetworkReply::NetworkError) {
    if (errorCbk_)
        errorCbk_(replyPtr_->errorString());
}

void appstoreservice::DownloadHandler::installStarted() {
    if (installStartCbk_)
        installStartCbk_();
}

void appstoreservice::DownloadHandler::installFinished(int exitcode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::CrashExit || exitcode != 0) {
        if (errorCbk_)
            errorCbk_(QString("Error occurred while installing \"%1\": %2").arg(pkgName_).arg(qPrintable(yumStderr_)));
    } else if (installFinishCbk_) {
        installFinishCbk_();
    }
}

void appstoreservice::DownloadHandler::readYumStderr() {
    yumStderr_.append(processPtr_->readAllStandardError());
}

void appstoreservice::DownloadHandler::run()
{
    netAccessPtr_.reset(new QNetworkAccessManager);
    auto *reply = netAccessPtr_->get(QNetworkRequest{url_});
    if (reply->error()) {
        if (errorCbk_)
            errorCbk_(QString("Error occurred while requesting url: ").append(url_));
    } else {
        replyPtr_.reset(reply);
        if (downloadStartCbk_)
            downloadStartCbk_();
        QObject::connect(replyPtr_.get(), &QNetworkReply::downloadProgress, this, &DownloadHandler::downloadProgress);
        QObject::connect(netAccessPtr_.get(), &QNetworkAccessManager::finished, this, &DownloadHandler::downloaded);
        QObject::connect(replyPtr_.get(), &QNetworkReply::errorOccurred, this, &DownloadHandler::networkError);
        pkgFilePtr_.reset(new QTemporaryFile(QDir::tempPath().append("/XXXXXX.rpm")));
    }
    QThread::exec();
}
