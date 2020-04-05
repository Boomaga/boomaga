#ifndef DEBUG_H
#define DEBUG_H

#include <QThread>
#include <QDebug>

#define DEBUG qDebug() << "[" << QThread::currentThreadId() << "]" << Q_FUNC_INFO << __LINE__ << ":"

#endif // DEBUG_H
