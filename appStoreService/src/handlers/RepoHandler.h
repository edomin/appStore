#ifndef REPOHANDLER_H
#define REPOHANDLER_H
#include <QThread>
#include <memory>
#include <map>

class QTimer;
class QString;
class QSettings;
namespace appstoreservice
{
class RepoHandler:public QThread
{
private:
    int interval_ {1000 * 3};
    std::shared_ptr<QTimer> timerPtr_ {nullptr};
    std::shared_ptr<QSettings> appSettingsPtr_ {nullptr};
    std::map<QString,QString> repoCacheMap_ {};

    std::map<QString,QString> getOsReleaseMap(QString& lastError)const;
    std::map<QString,QString> getRepoCacheMap(const QString& releaseVer)const;
protected:
    virtual void run()override;
public:
    explicit RepoHandler(std::shared_ptr<QSettings> appSettingsPtr,QObject* parent=nullptr);
    virtual ~RepoHandler();
private slots:
    void timeoutSlot();
};
}
#endif // REPOHANDLER_H
