#include <QCoreApplication>

//int main(int argc, char *argv[])
//{
//	QCoreApplication a(argc, argv);

//	return a.exec();
//}



// Qt lib import
#include <QCoreApplication>
#include <QtConcurrent>
#include <QSqlError>

// JasonQt lib import
#include "../Multithread_SqlDatabase/Multithread_SqlDatabase.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    /*
     * 註：關於附加參數
     * 這是可以不寫的，如果要寫的話，可以參考這個：
     *
     * 單次打開數據庫最大時間：也就是最大open的時間，對於某些數據庫，長時間open但不使用，不僅會造成資源浪費還會意外斷開。在設置了60 * 1000後，且60秒內未進行查詢，就自動斷開。
     * 多線程支持：簡單的說就是高級點的數據庫，比如 MySql 寫 JasonQt_Database::QueryMultiMode ；低級的，比如 Sqlite ，寫 JasonQt_Database::QuerySingleMode ,就可以了。
     * 最小等待時間：對於某些數據庫，比如Sqlite，密集查詢時可能出錯，此時可以適當的提升兩次查詢之間的最小等待時間，比如10ms
     */

    // Sqlite的連接方式                    類型        連接名        Sqlite文件路徑      單次打開數據庫最大時間                多線程支持           最小等待時間
    JasonQt_Database::Control control({ "QSQLITE", "TestDB", "/Users/Jason/test.db" }, { 60 * 1000,        JasonQt_Database::QuerySingleMode, 10});

    // MySql的連接方式                      類型      連接名        IP        數據庫    用戶名        密碼
//  JasonQt_Database::Control control({ "QMYSQL", "TestDB", "localhost", "JasonDB", "root", "YourPassword" });


    // SqlServer的連接方式                  類型      連接名                                    數據庫              數據庫名   用戶名         密碼
//  JasonQt_Database::Control control({ "QODBC", "TestDB", "Driver={SQL SERVER};server=iZ23kn6vmZ\\TEST;database=test;uid=sa;pwd=YourPassword;" });

    auto insert = [&]()
    {
        auto query(control.query()); // 這裡的query在解引用（ -> 或者 * ）後返回的是 QSqlQuery ，直接用就可以了，不需要單獨打開數據庫或者其他的初始化

        query->prepare("insert into Test1 values(?)"); // 模擬插入操作

        query->addBindValue(rand() % 1280);

        if(!query->exec())
        {
            qDebug() << "Error" << __LINE__;
        }

        query->clear();

        query->prepare("insert into Test2 values(NULL, ?, ?)");

        query->addBindValue(rand() % 1280);
        QString buf;
        for(int now = 0; now < 50; now++)
        {
            buf.append('a' + (rand() % 26));
        }
        query->addBindValue(buf);

        if(!query->exec())
        {
            qDebug() << "Error" << __LINE__;
        }
    };
    auto delete_ = [&]()
    {
        auto query(control.query());

        query->prepare("delete from Test1 where data = ?");

        query->addBindValue(rand() % 1280);

        if(!query->exec())
        {
            qDebug() << "Error" << __LINE__;
        }

        query->clear();

        query->prepare("delete from Test2 where data1 = ?");

        query->addBindValue(rand() % 1280);

        if(!query->exec())
        {
            qDebug() << "Error" << __LINE__;
        }
    };
    auto update = [&]()
    {
        auto query(control.query());

        query->prepare("update Test1 set data = ? where data = ?");

        query->addBindValue(rand() % 1280);
        query->addBindValue(rand() % 1280);

        if(!query->exec())
        {
            qDebug() << "Error" << __LINE__;
        }

        query->clear();

        query->prepare("update Test2 set data1 = ?, data2 = ? where data1 = ?");

        query->addBindValue(rand() % 1280 + 1);
        QString buf;
        for(int now = 0; now < 50; now++)
        {
            buf.append('a' + (rand() % 26));
        }
        query->addBindValue(buf);
        query->addBindValue(rand() % 1280 + 1);

        if(!query->exec())
        {
            qDebug() << "Error" << __LINE__;
        }
    };
    auto select = [&]()
    {
        auto query(control.query());

        query->prepare("select * from Test1 where data = ?");

        query->addBindValue(rand() % 1280);

        if(!query->exec())
        {
            qDebug() << "Error" << __LINE__;
        }

        query->clear();

        query->prepare("select * from Test2 where data1 = ?");

        query->addBindValue(rand() % 1280);

        if(!query->exec())
        {
            qDebug() << "Error" << __LINE__;
        }
    };

    volatile int count = 0, last = 0;

    QTime time;
    time.start();

    QThreadPool::globalInstance()->setMaxThreadCount(10);

    for(int now = 0; now < 3; now++) // 開啟3個線程測試
    {
        QtConcurrent::run([&]()
        {
            while(true)
            {
                count++;
                if(!(count % 1000))
                {
                    const auto &&now = time.elapsed();
                    qDebug() << now - last; // 打印每完成1000語句的時間
                    last = now;
                }

                switch(rand() % 20)
                {
                    case 0:  { insert(); break; }
                    case 1:  { delete_(); break; }
                    case 2:  { update(); break; }
                    default: { select(); break; }
                }
            }
            QThread::msleep(10);
        });
    }

    return a.exec();
}

//————————————————
//版权声明：本文为CSDN博主「Jason188080501」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
//原文链接：https://blog.csdn.net/wsj18808050/article/details/44891715
