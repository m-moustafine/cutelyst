/*
 * Copyright (C) 2016 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef WSGI_P_H
#define WSGI_P_H

#include "wsgi.h"

#include <QProcess>

#include <Cutelyst/Application>

class QTcpServer;

namespace CWSGI {

class CWsgiEngine;
class WSGIPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(WSGI)
public:
    inline WSGIPrivate(WSGI *parent) : QObject(parent), q_ptr(parent) { }

    bool listenTcp(const QString &line);
    bool listenSocket(const QString &address);
    void proc();
    int parseCommandLine(Cutelyst::Application *app);
    int setupApplication(Cutelyst::Application *app);
    void childFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void engineInitted();

    CWsgiEngine *createEngine(Cutelyst::Application *app, int core);

    bool loadConfig();

    WSGI *q_ptr;
    std::vector<QTcpServer *> sockets;
    std::vector<Cutelyst::Engine *> engines;
    CWsgiEngine *engine;

    QString application;
    QString chdir;
    QString chdir2;
    QString ini;
    int bufferSize = 4096;
    qint64 postBuffering = -1;
    qint64 postBufferingBufsize = 4096;
    int enginesInitted = 1;
    int threads = 0;
    int process = 0;
    bool master = false;

Q_SIGNALS:
    void forked();
};

}

#endif // WSGI_P_H

