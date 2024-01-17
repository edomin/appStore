#include "RepoHandler.h"

#include <QUrl>
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QString>
#include <QSettings>
#include <QTextStream>
#include <QFileInfo>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomAttr>
#include <QDebug>
#include <algorithm>
#include <vector>

std::map<QString, QString> appstoreservice::RepoHandler::getOsReleaseMap(QString &lastError) const
{
    std::map<QString,QString> osReleaseMap {};
    const auto osReleasePath {"/etc/os-release"};
    QFile file {osReleasePath};
    if(!file.open(QIODevice::ReadOnly)){
        lastError=file.errorString();
        return osReleaseMap;
    }
    QTextStream stream {&file};
    while(!stream.atEnd()){
        const auto line {stream.readLine()};
        const auto key {line.section('=',0,0).simplified()};
        const auto value {line.section('=',1,1).simplified()};
        if(!key.isEmpty() && !value.isEmpty()){
            osReleaseMap.emplace(key,value);
        }
    }
    return osReleaseMap;
}

std::map<QString, QString> appstoreservice::RepoHandler::getRepoCacheMap(const QString &releaseVer) const
{
    const auto repoFilesPath {"/etc/yum.repos.d"};
    const auto repoFilesList {QDir(repoFilesPath).entryInfoList({"*.repo"},QDir::Files)};
    std::vector<QJsonObject> repoObjects {};
    std::map<QString, QString> repoCacheMap {};

    std::for_each(repoFilesList.begin(),repoFilesList.end(),[&](const QFileInfo& entryInfo){
        const auto absolutePath {entryInfo.absoluteFilePath()};
        QFile file {absolutePath};
        if(file.open(QIODevice::ReadOnly)){
            const QString data {file.readAll()};
            const auto dataBlock {data.split("\n\n")};
            std::for_each(dataBlock.begin(),dataBlock.end(),[&](const QString block){
                QTextStream stream(block.toUtf8());
                QJsonObject repoObject {};
                while(!stream.atEnd()){
                    const auto line {stream.readLine()};
                    const auto key {line.section('=',0,0).simplified()};
                    const auto value {line.section('=',1,1).simplified()};
                    if(!key.isEmpty() && !value.isEmpty()){
                        repoObject.insert(key,value);
                    }
                }
                if(!repoObject.isEmpty()){
                    repoObjects.push_back(repoObject);
                }
            });
            file.close();
        }
    });

    const auto repoEnd {std::partition(repoObjects.begin(),repoObjects.end(),[](const QJsonObject& jsonObject){
            return jsonObject.value("enabled").toString()=="1" && jsonObject.value("name").toString().contains("x86_64");
        })};
    auto repoBegin {repoObjects.begin()};
    const auto releaseVerTag {"$releasever"};
    std::vector<QString> repoUrlList {};
    while(repoBegin!=repoEnd){
        const auto baseUrl {repoBegin->value("baseurl").toString()};
        const auto baseUrlList {baseUrl.split(",")};
        if(!baseUrlList.isEmpty()){
            std::transform(baseUrlList.begin(),baseUrlList.end(),std::back_inserter(repoUrlList),[&](const QString& url){
                auto repoUrl {url};
                repoUrl.replace(releaseVerTag,releaseVer);
                return repoUrl;
            });
        }
        ++repoBegin;
    }

    std::for_each(qAsConst(repoUrlList).begin(),qAsConst(repoUrlList).end(),[&](const QString& repoUrl){
        auto url {QUrl(repoUrl)};
        QNetworkAccessManager accessManegr {};
        QNetworkRequest request(url);
        QEventLoop eventLoop {};
        std::unique_ptr<QNetworkReply> replyPtr {accessManegr.get(request)};
        QObject::connect(replyPtr.get(),&QNetworkReply::finished,[&eventLoop](){
            eventLoop.quit();
        });
        eventLoop.exec();
        if(replyPtr->error()==QNetworkReply::NoError){
            const auto data {replyPtr->readAll()};
            QTextStream stream {data};
            while(!stream.atEnd()){
                const auto line {stream.readLine()};
                QDomDocument doc{};
                doc.setContent(line);
                const auto element {doc.documentElement()};
                if(!element.isNull() && element.tagName()=="a"){
                    const auto packageName {element.text()};
                    if(packageName.endsWith(".rpm")){
                        const auto packageLink {QStringLiteral("%1%2").arg(repoUrl,packageName)};
                        if(!packageLink.isEmpty() && !packageLink.isEmpty()){
                            repoCacheMap.emplace(packageName,packageLink);
                        }
                    }
                }
            }
        }
        else{
            qWarning(qPrintable(replyPtr->errorString()));
        }
    });
    qDebug("Packages cache created");
    return repoCacheMap;
}

void appstoreservice::RepoHandler::run()
{
    QString lastError {};
    const auto osReleaseMap {getOsReleaseMap(lastError)};
    repoCacheMap_=getRepoCacheMap(osReleaseMap.at("VERSION_ID"));
    if(!lastError.isEmpty()){
        QThread::quit();
        return;
    }

    timerPtr_.reset(new QTimer);
    timerPtr_->setInterval(interval_);
    QObject::connect(timerPtr_.get(),&QTimer::timeout,
                     this,&RepoHandler::timeoutSlot,Qt::DirectConnection);
    timerPtr_->start();
    QThread::exec();
}

appstoreservice::RepoHandler::RepoHandler(std::shared_ptr<QSettings> appSettingsPtr, QObject *parent)
    :QThread{parent},appSettingsPtr_{appSettingsPtr}
{
}

appstoreservice::RepoHandler::~RepoHandler()
{
    QThread::quit();
    QThread::wait();
}

void appstoreservice::RepoHandler::timeoutSlot()
{
    const auto isCacheExists {appSettingsPtr_->contains("repoCache")};
    timerPtr_->stop();
    if(isCacheExists){
    }
    timerPtr_->start();
}
