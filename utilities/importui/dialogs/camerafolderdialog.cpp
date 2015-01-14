/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-07-24
 * Description : a dialog to select a camera folders.
 *
 * Copyright (C) 2006-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "camerafolderdialog.h"

// Qt includes

#include <QLabel>
#include <QFrame>
#include <QGridLayout>
#include <QStandardPaths>
#include <QApplication>
#include <QStyle>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <khelpclient.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "camerafolderitem.h"
#include "camerafolderview.h"

namespace Digikam
{

class CameraFolderDialog::Private
{
public:

    Private() :
        buttons(0),
        folderView(0)
    {
    }

    QString           rootPath;
    QDialogButtonBox* buttons;

    CameraFolderView* folderView;
};

CameraFolderDialog::CameraFolderDialog(QWidget* const parent, const QMap<QString, int>& map,
                                       const QString& cameraName, const QString& rootPath)
    : QDialog(parent),
      d(new Private)
{
    setModal(true);
    setWindowTitle(i18nc("@title:window %1: name of the camera", "%1 - Select Camera Folder", cameraName));

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(false);

    d->rootPath        = rootPath;
    QFrame* const page = new QFrame(this);

    QGridLayout* const grid = new QGridLayout(page);
    d->folderView           = new CameraFolderView(page);
    QLabel* const logo      = new QLabel(page);
    QLabel* const message   = new QLabel(page);

    logo->setPixmap(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "digikam/data/logo-digikam.png"))
                    .scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    message->setText(i18n("<p>Please select the camera folder "
                          "where you want to upload the images.</p>"));
    message->setWordWrap(true);

    grid->addWidget(logo,          0, 0, 1, 1);
    grid->addWidget(message,       1, 0, 1, 1);
    grid->addWidget(d->folderView, 0, 1, 3, 1);
    grid->setRowStretch(2, 10);
    grid->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    d->folderView->addVirtualFolder(cameraName);
    d->folderView->addRootFolder(QString("/"));

    for (QMap<QString, int>::const_iterator it = map.constBegin(); it != map.constEnd(); ++it)
    {
        QString folder(it.key());

        if (folder != QString("/"))
        {
            if (folder.startsWith(rootPath) && rootPath != QString("/"))
            {
                folder.remove(0, rootPath.length());
            }

            if (folder != QString("/") && !folder.isEmpty())
            {
                QString root = folder.section('/', 0, -2);

                if (root.isEmpty())
                {
                    root = QString("/");
                }

                QString sub = folder.section('/', -1);
                qCDebug(LOG_IMPORTUI) << "Camera folder: '" << folder << "' (root='" << root << "', sub='" << sub << "')";
                d->folderView->addFolder(root, sub, it.value());
            }
        }
    }

    connect(d->folderView, SIGNAL(signalFolderChanged(CameraFolderItem*)),
            this, SLOT(slotFolderPathSelectionChanged(CameraFolderItem*)));

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    adjustSize();

    // make sure the ok button is properly set up
    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(d->folderView->currentItem() != 0);
}

CameraFolderDialog::~CameraFolderDialog()
{
    delete d;
}

QString CameraFolderDialog::selectedFolderPath() const
{
    QTreeWidgetItem* const item = d->folderView->currentItem();

    if (!item)
    {
        return QString();
    }

    CameraFolderItem* const folderItem = dynamic_cast<CameraFolderItem*>(item);

    if (!folderItem)
    {
        return QString();
    }

    if (folderItem->isVirtualFolder())
    {
        return QString(d->rootPath);
    }

    // Case of Gphoto2 cameras. No need to duplicate root '/'.
    if (d->rootPath == QString("/"))
    {
        return(folderItem->folderPath());
    }

    return (d->rootPath + folderItem->folderPath());
}

void CameraFolderDialog::slotFolderPathSelectionChanged(CameraFolderItem* item)
{
    if (item)
    {
        d->buttons->button(QDialogButtonBox::Ok)->setEnabled(true);
        qCDebug(LOG_IMPORTUI) << "Camera folder path: " << selectedFolderPath();
    }
    else
    {
        d->buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}

void CameraFolderDialog::slotHelp()
{
    KHelpClient::invokeHelp("camerainterface.anchor", "digikam");
}

}  // namespace Digikam
