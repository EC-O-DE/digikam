/* ============================================================
 * File  : albumicongroupitem.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-04-25
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju

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

#include <qpixmap.h>
#include <qpainter.h>

#include <klocale.h>
#include <kglobal.h>

#include <kdeversion.h>
#if KDE_IS_VERSION(3,2,0)
#include <kcalendarsystem.h>
#endif


#include "albummanager.h"
#include "album.h"
#include "themeengine.h"
#include "albumsettings.h"
#include "albumiconview.h"
#include "albumicongroupitem.h"

AlbumIconGroupItem::AlbumIconGroupItem(AlbumIconView* view,
                                       int albumID)
    : IconGroupItem(view), m_albumID(albumID), m_view(view)
{
    
}

AlbumIconGroupItem::~AlbumIconGroupItem()
{
    
}

int AlbumIconGroupItem::compare(IconGroupItem* group)
{
    AlbumIconGroupItem* agroup = (AlbumIconGroupItem*)group;
    
    PAlbum* mine = AlbumManager::instance()->findPAlbum(m_albumID);
    PAlbum* his = AlbumManager::instance()->findPAlbum(agroup->m_albumID);

    const AlbumSettings *settings = m_view->settings();
    
    switch (settings->getImageSortOrder())
    {
    case(AlbumSettings::ByIName):
    case(AlbumSettings::ByISize):
    case(AlbumSettings::ByIPath):
    {
        return mine->getTitle().localeAwareCompare(his->getTitle());
    }
    case(AlbumSettings::ByIDate):
    {
        if (mine->getDate() < his->getDate())
            return -1;
        else if (mine->getDate() > his->getDate())
            return 1;
        else
            return 0;
    }
    }

    return 0;
}

void AlbumIconGroupItem::paintBanner()
{
    AlbumManager* man = AlbumManager::instance();
    PAlbum* album = man->findPAlbum(m_albumID);

    QDate  date  = album->getDate();

#if KDE_IS_VERSION(3,2,0)
    QString dateAndComments = i18n("%1 %2 - 1 Item", "%1 %2 - %n Items", count())
                              .arg(KGlobal::locale()->calendar()->monthName(date, false))
                              .arg(KGlobal::locale()->calendar()->year(date));
#else
    QString dateAndComments = i18n("%1 %2 - 1 Item", "%1 %2 - %n Items", count())
                              .arg(KGlobal::locale()->monthName(date.month()))
                              .arg(QString::number(date.year()));
#endif

    if (!album->getCaption().isEmpty())
        dateAndComments += " - " + album->getCaption();

    
    QRect r(0, 0, rect().width(), rect().height());

    QPixmap pix(m_view->bannerPixmap());
    
    QFont fn(m_view->font());
    fn.setBold(true);
    int fnSize = fn.pointSize();
    bool usePointSize;
    if (fnSize > 0) {
        fn.setPointSize(fnSize+2);
        usePointSize = true;
    }
    else {
        fnSize = fn.pixelSize();
        fn.setPixelSize(fnSize+2);
        usePointSize = false;
    }

    QPainter p(&pix);
    p.setPen(ThemeEngine::instance()->textSelColor());
    p.setFont(fn);

    QRect tr;
    p.drawText(5, 5, r.width(), r.height(),
               Qt::AlignLeft | Qt::AlignTop, album->getPrettyURL(),
               -1, &tr);

    r.setY(tr.height() + 2);

    if (usePointSize)
        fn.setPointSize(m_view->font().pointSize());
    else
        fn.setPixelSize(m_view->font().pixelSize());

    fn.setBold(false);
    p.setFont(fn);

    p.drawText(5, r.y(), r.width(), r.height(),
               Qt::AlignLeft | Qt::AlignVCenter, dateAndComments);
    
    p.end();

    r = rect();
    r = QRect(iconView()->contentsToViewport(QPoint(r.x(), r.y())),
              QSize(r.width(), r.height()));
    
    bitBlt(iconView()->viewport(), r.x(), r.y(), &pix,
           0, 0, r.width(), r.height());
}
