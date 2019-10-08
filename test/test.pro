QT -= gui
QT += sql concurrent

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Multithread_SqlDatabase/release/ -lMultithread_SqlDatabase
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Multithread_SqlDatabase/debug/ -lMultithread_SqlDatabase
else:unix: LIBS += -L$$OUT_PWD/../Multithread_SqlDatabase/ -lMultithread_SqlDatabase

INCLUDEPATH += $$PWD/../Multithread_SqlDatabase
DEPENDPATH += $$PWD/../Multithread_SqlDatabase

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Multithread_SqlDatabase/release/libMultithread_SqlDatabase.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Multithread_SqlDatabase/debug/libMultithread_SqlDatabase.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Multithread_SqlDatabase/release/Multithread_SqlDatabase.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Multithread_SqlDatabase/debug/Multithread_SqlDatabase.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../Multithread_SqlDatabase/libMultithread_SqlDatabase.a
