#include <QApplication>
#include <QQmlContext>
#include <QSettings>
#include <QVariant>
#include <QtGlobal>
#include <QString>
#include <QDir>
#include <map>
#include <memory>
#include <cassert>

#include "MainWindow.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <spdlog/sinks/rotating_file_sink.h>

const QString appName {"AppStoreApp"};
const QString orgName {"MosHub"};
std::shared_ptr<spdlog::logger> loggerPtr_ {nullptr};
std::shared_ptr<QSettings> appSettingsPtr_ {nullptr};

void initLogger();
void initSettings();
void msgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
    QApplication app {argc,argv};
    initLogger();
    initSettings();
    qInstallMessageHandler(msgHandler);

    //assert(loggerPtr_);
    //assert(appSettingsPtr_);

    appstoreapp::MainWindow mainWindow {appSettingsPtr_};
    mainWindow.resize(800,600);
    mainWindow.show();
    return app.exec();
}

void initLogger()
{
    const QString appDir {QCoreApplication::applicationDirPath()};
    const QString varLogAppDir {QString("%1/../.var/log/appstoreapp").arg(appDir)};
    const bool isLogDirOk {QDir{}.mkpath(varLogAppDir)};
    if(!isLogDirOk){
        qWarning("Fail to create log directory!");
        return;
    }
    const int logFilescount {5};
    const int logFilesize {1024 * 1024 * 50};
    const QString logName {"AppStoreApp"};
    const QString logfilenamePath {varLogAppDir + "/appstoreapp.log"};
    const spdlog::level::level_enum logLevel {spdlog::level::debug};
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
#ifdef Q_OS_WIN
    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logfilenamePath.toStdString(), logFilesize, logFilescount));
#endif
#ifdef Q_OS_LINUX
    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logfilenamePath.toStdString(), logFilesize, logFilescount));
#endif
    loggerPtr_.reset(new spdlog::logger(logName.toStdString(), sinks.begin(),sinks.end()));
    spdlog::register_logger(loggerPtr_);
    loggerPtr_->set_level(logLevel);
    loggerPtr_->flush_on(logLevel);
}

void initSettings()
{
    const QString appDir {QCoreApplication::applicationDirPath()};
    const QString etcAppDir {QStringLiteral("%1/../.etc/appstoreapp").arg(appDir)};
    const QString appSettingsFilename {"appstoreapp.cfg"};
    appSettingsPtr_.reset(new QSettings(QStringLiteral("%1/%2").arg(etcAppDir,appSettingsFilename),QSettings::IniFormat));
    const std::map<QString,QVariant> defaultMap {
        {"appStoreUrl","https://school.mos.ru/app/"},
        {"appStoreServiceAddress","127.0.0.1"},
        {"appStoreServicePort",56789}
    };
    std::for_each(defaultMap.begin(),defaultMap.end(),
                  [&](const std::pair<const QString,QVariant>& pair){
        if(!appSettingsPtr_->contains(pair.first) | appSettingsPtr_->value(pair.first).isNull()){
            appSettingsPtr_->setValue(pair.first,pair.second);
        }
    });
}

void msgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    const qint32 logLine {context.line};
    const char *logFile {context.file ? context.file : ""};
    const char *logFunction {context.function ? context.function : ""};
    if(!loggerPtr_){
        qDebug("File: %s; Line: %d; Function: %s; Message: %s",
               logFile,logLine,logFunction,qPrintable(msg));
        return;
    }
    switch(type){
    case QtMsgType::QtDebugMsg:
        loggerPtr_->debug("{}; {}",logFunction,msg.toStdString());
        break;
    case QtMsgType::QtInfoMsg:
        loggerPtr_->info("{}; {}",logFunction,msg.toStdString());
        break;
    case QtMsgType::QtWarningMsg:
        loggerPtr_->warn("{}; {}",logFunction,msg.toStdString());
        break;
    case QtMsgType::QtCriticalMsg:
        loggerPtr_->error("{}; {}",logFunction,msg.toStdString());
        break;
    case QtMsgType::QtFatalMsg:
        loggerPtr_->error("File: {}; Line: {}; Function: {}; Message: {}",logFile,logLine,logFunction,msg.toStdString());
        break;
    }
}
