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
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QObject>

#include "cwsgiengine.h"

namespace CWSGI {

class WSGI;
class Protocol : public QObject
{
    Q_OBJECT
public:
    explicit Protocol(WSGI *wsgi, QObject *parent = 0);

    virtual void readyRead() = 0;

    WSGI *m_wsgi;
};

}

#endif // PROTOCOL_H
