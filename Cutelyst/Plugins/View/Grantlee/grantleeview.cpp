/*
 * Copyright (C) 2013-2016 Daniel Nicoletti <dantti12@gmail.com>
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

#include "grantleeview_p.h"

#include "application.h"
#include "context.h"
#include "action.h"
#include "response.h"

#include <QString>
#include <QDirIterator>
#include <QtCore/QLoggingCategory>

Q_LOGGING_CATEGORY(CUTELYST_GRANTLEE, "cutelyst.grantlee")

using namespace Cutelyst;

GrantleeView::GrantleeView(QObject *parent, const QString &name) : View(parent, name)
  , d_ptr(new GrantleeViewPrivate)
{
    Q_D(GrantleeView);

    d->loader = QSharedPointer<Grantlee::FileSystemTemplateLoader>(new Grantlee::FileSystemTemplateLoader);

    d->engine = new Grantlee::Engine(this);
    d->engine->addTemplateLoader(d->loader);

    Application *app = qobject_cast<Application *>(parent);
    if (app) {
        // make sure templates can be found on the current directory
        setIncludePaths({ app->config(QStringLiteral("root")).toString() });

        // If CUTELYST_VAR is set the template might have become
        // {{ Cutelyst.req.base }} instead of {{ c.req.base }}
        d->cutelystVar = app->config(QStringLiteral("CUTELYST_VAR"), QStringLiteral("c")).toString();
    } else {
        // make sure templates can be found on the current directory
        setIncludePaths({ QDir::currentPath() });
    }
}

GrantleeView::~GrantleeView()
{
    delete d_ptr;
}

QStringList GrantleeView::includePaths() const
{
    Q_D(const GrantleeView);
    return d->includePaths;
}

void GrantleeView::setIncludePaths(const QStringList &paths)
{
    Q_D(GrantleeView);
    d->loader->setTemplateDirs(paths);
    d->includePaths = paths;
}

QString GrantleeView::templateExtension() const
{
    Q_D(const GrantleeView);
    return d->extension;
}

void GrantleeView::setTemplateExtension(const QString &extension)
{
    Q_D(GrantleeView);
    d->extension = extension;
}

QString GrantleeView::wrapper() const
{
    Q_D(const GrantleeView);
    return d->wrapper;
}

void GrantleeView::setWrapper(const QString &name)
{
    Q_D(GrantleeView);
    d->wrapper = name;
}

void GrantleeView::setCache(bool enable)
{
    Q_D(GrantleeView);

    if (enable != d->cache.isNull()) {
        return; // already enabled
    }

    delete d->engine;
    d->engine = new Grantlee::Engine(this);

    if (enable) {
        d->cache = QSharedPointer<Grantlee::CachingLoaderDecorator>(new Grantlee::CachingLoaderDecorator(d->loader));
        d->engine->addTemplateLoader(d->cache);
    } else {
        d->cache.clear();
        d->engine->addTemplateLoader(d->loader);
    }
}

void GrantleeView::preloadTemplates()
{
    Q_D(GrantleeView);

    if (!isCaching()) {
        setCache(true);
    }

    const auto includePaths = d->includePaths;
    for (const QString &includePath : includePaths) {
        QDirIterator it(includePath, {
                            QLatin1Char('*') + d->extension
                        },
                        QDir::Files | QDir::NoDotAndDotDot,
                        QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString path = it.next();
            path.remove(includePath);
            if (path.startsWith(QLatin1Char('/'))) {
                path.remove(0, 1);
            }

            if (d->cache->canLoadTemplate(path)) {
                d->cache->loadByName(path, d->engine);
            }
        }
    }
}

bool GrantleeView::isCaching() const
{
    Q_D(const GrantleeView);
    return !d->cache.isNull();
}

QByteArray GrantleeView::render(Context *c) const
{
    Q_D(const GrantleeView);

    QByteArray ret;
    QVariantHash stash = c->stash();
    QString templateFile = stash.value(QStringLiteral("template")).toString();
    if (templateFile.isEmpty()) {
        if (c->action() && !c->action()->reverse().isEmpty()) {
            templateFile = c->action()->reverse() + d->extension;
            if (templateFile.startsWith(QLatin1Char('/'))) {
                templateFile.remove(0, 1);
            }
        }

        if (templateFile.isEmpty()) {
            c->error(QStringLiteral("Cannot render template, template name or template stash key not defined"));
            return ret;
        }
    }

    qCDebug(CUTELYST_GRANTLEE) << "Rendering template" << templateFile;

    stash.insert(d->cutelystVar, QVariant::fromValue(c));

    Grantlee::Context gc(stash);

    Grantlee::Template tmpl = d->engine->loadByName(templateFile);
    QString content = tmpl->render(&gc);
    if (tmpl->error() != Grantlee::NoError) {
        c->res()->body() = tr("Internal server error.").toUtf8();
        c->error(QLatin1String("Error while rendering template: ") + tmpl->errorString());
        return ret;
    }

    if (!d->wrapper.isEmpty()) {
        Grantlee::Template wrapper = d->engine->loadByName(d->wrapper);
        Grantlee::SafeString safeContent(content, true);
        gc.insert(QStringLiteral("content"), safeContent);
        content = wrapper->render(&gc);

        if (wrapper->error() != Grantlee::NoError) {
            c->res()->body() = tr("Internal server error.").toUtf8();
            c->error(QLatin1String("Error while rendering template: ") + tmpl->errorString());
            return ret;
        }
    }

    ret = content.toUtf8();
    return ret;
}

#include "moc_grantleeview.cpp"
