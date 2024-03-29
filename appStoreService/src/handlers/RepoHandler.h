#ifndef REPOHANDLER_H
#define REPOHANDLER_H
#include <QDateTime>
#include <QThread>
#include <functional>
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
    const int timerInterval_ {1000 * 3};
    const int cacheUpdateDaysInterval_ {3};
    QDateTime lastCacheUpdatedDt_ {};

    std::shared_ptr<QTimer> timerPtr_ {nullptr};
    std::shared_ptr<QSettings> appSettingsPtr_ {nullptr};
    std::map<QString,QString> repoCacheMap_ {};
    std::function<void(std::map<QString,QString>&& repoCacheMap)> repoUpdatedCallback_ {nullptr};

    void updateRepoCache(QString& lastError);
    std::map<QString,QString> getOsReleaseMap(QString& lastError)const;
    std::map<QString,QString> getRepoCacheMap(const QString& releaseVer)const;
protected:
    virtual void run()override;
public:
    explicit RepoHandler(std::shared_ptr<QSettings> appSettingsPtr,QObject* parent=nullptr);
    virtual ~RepoHandler();
    inline void setRepoUpdatedCallback(std::function<void(std::map<QString,QString>&& repoCacheMap)> repoUpdatedCallback){
        repoUpdatedCallback_=repoUpdatedCallback;
    }
private slots:
    void timeoutSlot();
};
}
#endif // REPOHANDLER_H
