/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-15
 * Description : IO Jobs thread for file system jobs
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "iojobsthread.h"

// Qt includes

#include <QFileInfo>
#include <QDir>

// Local includes

#include "iojob.h"
#include "iojobdata.h"
#include "digikam_debug.h"
#include "coredb.h"
#include "coredbaccess.h"

namespace Digikam
{

class IOJobsThread::Private
{

public:

    Private()
        : jobsCount(0),
          isCanceled(false),
          jobData(0)
    {
    }

    int            jobsCount;
    bool           isCanceled;

    IOJobData*     jobData;

    QList<QString> errorsList;

};

IOJobsThread::IOJobsThread(QObject* const parent)
    : ActionThreadBase(parent),
      d(new Private)
{
}

IOJobsThread::~IOJobsThread()
{
    delete d->jobData;
    delete d;
}

void IOJobsThread::copy(IOJobData* const data, const QList<QUrl>& srcFiles, const QUrl destAlbum)
{
    d->jobData = data;
    ActionJobCollection collection;

    foreach (const QUrl& url, srcFiles)
    {
        CopyJob* const j = new CopyJob(url, destAlbum, false);

        connectOneJob(j);

        collection.insert(j, 0);
        d->jobsCount++;
    }

    appendJobs(collection);
}

void IOJobsThread::move(IOJobData* const data, const QList<QUrl>& srcFiles, const QUrl destAlbum)
{
    d->jobData = data;
    ActionJobCollection collection;

    foreach (const QUrl& url, srcFiles)
    {
        CopyJob* const j = new CopyJob(url, destAlbum, true);

        connectOneJob(j);

        collection.insert(j, 0);
        d->jobsCount++;
    }

    appendJobs(collection);
}

void IOJobsThread::deleteFiles(IOJobData* const data, const QList<QUrl>& srcsToDelete, bool useTrash)
{
    d->jobData = data;
    ActionJobCollection collection;

    foreach (const QUrl& url, srcsToDelete)
    {
        DeleteJob* const j = new DeleteJob(url, useTrash, true);

        connectOneJob(j);

        collection.insert(j, 0);
        d->jobsCount++;
    }

    appendJobs(collection);
}

void IOJobsThread::listDTrashItems(const QString& collectionPath)
{
    ActionJobCollection collection;

    DTrashItemsListingJob* const j = new DTrashItemsListingJob(collectionPath);

    connect(j, SIGNAL(trashItemInfo(DTrashItemInfo)),
            this, SIGNAL(collectionTrashItemInfo(DTrashItemInfo)));

    connect(j, SIGNAL(signalDone()),
            this, SIGNAL(finished()));

    collection.insert(j, 0);
    d->jobsCount++;

    appendJobs(collection);
}

void IOJobsThread::restoreDTrashItems(const DTrashItemInfoList& items)
{
    ActionJobCollection collection;

    RestoreDTrashItemsJob* const j = new RestoreDTrashItemsJob(items);

    connect(j, SIGNAL(signalDone()),
            this, SIGNAL(finished()));

    collection.insert(j, 0);
    d->jobsCount++;

    appendJobs(collection);
}

void IOJobsThread::deleteDTrashItems(const DTrashItemInfoList& items)
{
    ActionJobCollection collection;

    DeleteDTrashItemsJob* const j = new DeleteDTrashItemsJob(items);

    connect(j, SIGNAL(signalDone()),
            this, SIGNAL(finished()));

    collection.insert(j, 0);
    d->jobsCount++;

    appendJobs(collection);
}

void IOJobsThread::renameFile(IOJobData* const data, const QUrl& srcToRename, const QUrl& newName)
{
    d->jobData = data;
    ActionJobCollection collection;

    RenameFileJob* const j = new RenameFileJob(srcToRename, newName);

    connectOneJob(j);

    connect(j, SIGNAL(signalRenamed(QUrl,QUrl)),
            this, SIGNAL(signalRenamed(QUrl,QUrl)));

    connect(j, SIGNAL(signalRenameFailed(QUrl)),
            this, SIGNAL(signalRenameFailed(QUrl)));

    collection.insert(j, 0);
    d->jobsCount++;

    appendJobs(collection);
}

bool IOJobsThread::isCanceled()
{
    return d->isCanceled;
}

bool IOJobsThread::hasErrors()
{
    return !d->errorsList.isEmpty();
}

QList<QString>& IOJobsThread::errorsList()
{
    return d->errorsList;
}

void IOJobsThread::connectOneJob(IOJob* const j)
{
    connect(j, SIGNAL(error(QString)),
            this, SLOT(slotError(QString)));

    connect(j, SIGNAL(signalDone()),
            this, SLOT(slotOneJobFinished()));
}

void IOJobsThread::slotOneJobFinished()
{
    d->jobsCount--;

    if (d->jobData)
    {
        emit signalOneProccessed(d->jobData->operation());
    }

    if (d->jobsCount == 0)
    {
        emit finished();
        qCDebug(DIGIKAM_IOJOB_LOG) << "Thread Finished";
    }
}

void IOJobsThread::slotError(const QString& errString)
{
    d->errorsList.append(errString);
}

void IOJobsThread::slotCancel()
{
    d->isCanceled = true;
    ActionThreadBase::cancel();
}

IOJobData* IOJobsThread::jobData()
{
    return d->jobData;
}

} // namespace Digikam
