#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>

namespace appstoreservice
{
class Service : public QObject
{
    Q_OBJECT
public:
    explicit Service(QObject *parent = nullptr);
    virtual ~Service()=default;

signals:

};
}

#endif // SERVICE_H
