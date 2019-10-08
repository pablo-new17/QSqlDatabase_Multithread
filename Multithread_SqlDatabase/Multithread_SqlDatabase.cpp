#include "Multithread_SqlDatabase.h"

Multithread_SqlDatabase::Multithread_SqlDatabase()
{
}



using namespace JasonQt_Database;

// DatabaseSettings
DatabaseSettings::DatabaseSettings(const DatabaseModeEnum &databastMode, const QString &databaseType, const QString &connectionName)
{
    m_databaseMode = databastMode;
    m_databaseType = databaseType;
    m_connectionName = connectionName;
}

DatabaseSettings::DatabaseSettings(const QString &databaseType, const QString &connectionName, const QString &nameModeName):
    DatabaseSettings(DatabaseNameMode, databaseType, connectionName)
{
    m_nameModeName = nameModeName;
}

DatabaseSettings::DatabaseSettings(const QString &databaseType, const QString &connectionName, const QString &hostModeHostName, const QString &hostModeDatabaseName, const QString &hostModeUserName, const QString &hostModePassword):
    DatabaseSettings(DatabaseHostMode, databaseType, connectionName)
{
    m_hostModeHostName = hostModeHostName;
    m_hostModeDatabaseName = hostModeDatabaseName;
    m_hostModeUserName = hostModeUserName;
    m_hostModePassword = hostModePassword;
}

// ConnectSettings
ConnectSettings::ConnectSettings(const int &maxOpenTime, const QueryMode &queryMode, const int &minWaitTime)
{
    m_maxOpenTime = maxOpenTime;
    m_queryMode = queryMode;
    m_minWaitTime = minWaitTime;
}

// Query
Query::Query(QSqlDatabase &dataBase, QMutex *mutex):
    m_query(new QSqlQuery(dataBase))
{
    m_mutex = mutex;
}

Query::Query(Query &&other)
{
    m_query = other.takeQuery();
    m_mutex = other.takeMutex();
}

Query::~Query(void)
{
    if(m_query)
    {
        delete m_query;
    }
    if(m_mutex)
    {
        m_mutex->unlock();
    }
}

QSqlQuery *Query::takeQuery(void)
{
    auto buf = m_query;
    m_query = NULL;
    return buf;
}

QMutex *Query::takeMutex(void)
{
    auto buf = m_mutex;
    m_mutex = NULL;
    return buf;
}

// ConnectNode
ConnectNode::ConnectNode(const DatabaseSettings &dataBaseSettings, const ConnectSettings &connectSettings):
    m_dataBaseSettings(dataBaseSettings),
    m_connectSettings(connectSettings)
{
    m_connectionName = QString("%1(%2)").arg(m_dataBaseSettings.connectionName()).arg(QString::number(qint64(QThread::currentThread()), 16));

    m_mutex = new QMutex(QMutex::Recursive);

    if(m_connectSettings.maxOpenTime())
    {
        m_autoClose = new QTimer;
        m_autoClose->setSingleShot(true);
        m_autoClose->setInterval(m_connectSettings.maxOpenTime());
        m_autoClose->moveToThread(qApp->thread());
        m_autoClose->setParent(qApp);

        connect(m_autoClose, SIGNAL(timeout()), this, SLOT(close()), Qt::DirectConnection);
        connect(this, SIGNAL(controlStartAutoClose()), m_autoClose, SLOT(start()));
        connect(this, SIGNAL(controlStopAutoClose()), m_autoClose, SLOT(stop()));
    }

    this->addDataBase();
}

ConnectNode::~ConnectNode(void)
{
    if(m_mutex){ m_mutex->lock(); }

    if(m_autoClose)
    {
        m_autoClose->deleteLater();
    }

    this->removeDataBase();

    if(m_mutex){ m_mutex->unlock(); }
    if(m_mutex){ delete m_mutex; }
}

Query ConnectNode::query(void)
{
    if(!m_database)
    {
        this->addDataBase();
    }

    if(!m_database->isOpen())
    {
        m_database->open();
    }

    if(m_mutex){ m_mutex->lock(); }

    Query buf(*m_database, m_mutex);
    emit controlStartAutoClose();
    return buf;
}

bool ConnectNode::addDataBase(void)
{
    if(m_mutex){ m_mutex->lock(); }

    if(m_database)
    {
        this->removeDataBase();
    }

    m_database = new QSqlDatabase(QSqlDatabase::addDatabase(m_dataBaseSettings.databaseType(), m_connectionName));

    switch(m_dataBaseSettings.databaseMode())
    {
        case DatabaseNameMode:
        {
            m_database->setDatabaseName(m_dataBaseSettings.nameModeName());
            break;
        }
        case DatabaseHostMode:
        {
            m_database->setHostName(m_dataBaseSettings.hostModeHostName());
            m_database->setDatabaseName(m_dataBaseSettings.hostModeDatabaseName());
            m_database->setUserName(m_dataBaseSettings.hostModeUserName());
            m_database->setPassword(m_dataBaseSettings.hostModePassword());
            break;
        }
        default:
        {
            if(m_mutex){ m_mutex->unlock(); }
            return false;
        }
    }

    const auto &&flag = this->open();

    if(m_mutex){ m_mutex->unlock(); }

    return flag;
}

void ConnectNode::removeDataBase(void)
{
    if(m_mutex){ m_mutex->lock(); }

    delete m_database;
    m_database = NULL;
    QSqlDatabase::removeDatabase(m_connectionName);

    if(m_mutex){ m_mutex->unlock(); }
}

bool ConnectNode::open(void)
{
    if(!m_database)
    {
        this->addDataBase();
    }

    if(m_mutex){ m_mutex->lock(); }

    emit controlStartAutoClose();
    const auto &&Flag = m_database->open();

    if(m_mutex){ m_mutex->unlock(); }

    return Flag;
}

void ConnectNode::close(void)
{
    if(m_mutex)
    {
        if(m_mutex->tryLock())
        {
            m_mutex->unlock();
            emit controlStopAutoClose();
            m_database->close();
        }
        else
        {
            emit controlStartAutoClose();
        }
    }
    else
    {
        emit controlStopAutoClose();
        m_database->close();
    }
}

// Control
Control::Control(const DatabaseSettings &databaseSettings, const ConnectSettings &connectSettings):
    m_databaseSettings(databaseSettings),
    m_connectSettings(connectSettings)
{
    if(m_connectSettings.queryMode() == QueryAutoMode)
    {
        if(databaseSettings.databaseType() == "QMYSQL")
        {
            m_connectSettings.setQueryMode(QueryMultiMode);
        }
        else if(databaseSettings.databaseType() == "QODBC")
        {
            m_connectSettings.setQueryMode(QueryMultiMode);
        }
        else
        {
            m_connectSettings.setQueryMode(QuerySingleMode);
        }
    }
    if(m_connectSettings.queryMode() == QuerySingleMode)
    {
        this->insert(qint64(QThread::currentThread()));
    }

    if(m_connectSettings.minWaitTime() == -1)
    {
        if(databaseSettings.databaseType() == "QMYSQL")
        {
            m_connectSettings.setMinWaitTime(0);
        }
        else if(databaseSettings.databaseType() == "QODBC")
        {
            m_connectSettings.setMinWaitTime(0);
        }
        else
        {
            m_connectSettings.setMinWaitTime(5);
            m_wait = new QTime;
            m_wait->start();
        }
    }
    else
    {
        m_wait = new QTime;
        m_wait->start();
    }
}

Control::~Control(void)
{
    for(auto &now: m_node)
    {
        now->deleteLater();
    }
    if(m_wait)
    {
        delete m_wait;
    }
}

void Control::removeAll(void)
{
    m_mutex.lock();

    for(auto &Now: m_node)
    {
        Now->removeDataBase();
    }

    m_mutex.unlock();
}

Query Control::query(void)
{
    if(m_wait)
    {
        const auto &&flag = m_connectSettings.minWaitTime() - m_wait->elapsed();
        m_wait->restart();
        if(flag > 0)
        {
            QThread::msleep(flag);
        }
    }

    if(m_connectSettings.queryMode() == QueryMultiMode)
    {
        m_mutex.lock();

        const auto &currentThread = qint64(QThread::currentThread());
        const auto &&now = m_node.find(currentThread);
        if(now != m_node.end())
        {
            auto buf((*now)->query());

            m_mutex.unlock();
            return buf;
        }
        else
        {
            this->insert(qint64(QThread::currentThread()));

            m_mutex.unlock();
            return this->query();
        }
    }
    else
    {
        return (*m_node.begin())->query();
    }
}

void Control::insert(const qint64 &key)
{
    m_node[key] = new ConnectNode(m_databaseSettings, m_connectSettings);
}
//————————————————
//版权声明：本文为CSDN博主「Jason188080501」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
//原文链接：https://blog.csdn.net/wsj18808050/article/details/44891715
