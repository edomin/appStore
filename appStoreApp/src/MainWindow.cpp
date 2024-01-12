#include "MainWindow.h"

#include <QUrl>
#include <QSettings>
#include <QWebEnginePage>
#include <QWebEngineView>

appstoreapp::MainWindow::MainWindow(std::shared_ptr<QSettings> appSettingsPtr, QWidget *parent)
    : QMainWindow{parent},
      appSettingsPtr_{appSettingsPtr},
      webEnginePagePtr_{new QWebEnginePage{this}},
      webEngineViewPtr_{new QWebEngineView{this}}
{
    const auto appStoreUrl {appSettingsPtr_->value("appStoreUrl").toString()};
    webEnginePagePtr_->setUrl(QUrl(appStoreUrl));
    webEngineViewPtr_->setPage(webEnginePagePtr_);
    setCentralWidget(webEngineViewPtr_);
}

