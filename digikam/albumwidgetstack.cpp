/* ============================================================
 * Authors: Gilles Caulier 
 * Date   : 2006-06-13
 * Description : A widget stack to embedded album content view
 *               or the current image preview.
 *
 * Copyright 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qfileinfo.h>

// KDE includes.

#include <kurl.h>
#include <khtmlview.h>

// Local includes.

#include "albumsettings.h"
#include "albumiconview.h"
#include "imagepreviewview.h"
#include "welcomepageview.h"
#include "mediaplayerview.h"
#include "albumwidgetstack.h"
#include "albumwidgetstack.moc"

namespace Digikam
{

class AlbumWidgetStackPriv
{

public:

    AlbumWidgetStackPriv()
    {
        albumIconView    = 0;
        imagePreviewView = 0;
        welcomePageView  = 0;
        mediaPlayerView  = 0;
    }

    AlbumIconView    *albumIconView;

    ImagePreviewView *imagePreviewView;

    WelcomePageView  *welcomePageView;

    MediaPlayerView  *mediaPlayerView;
};

AlbumWidgetStack::AlbumWidgetStack(QWidget *parent)
                : QWidgetStack(parent, 0, Qt::WDestructiveClose)
{
    d = new AlbumWidgetStackPriv;

    d->albumIconView    = new AlbumIconView(this);
    d->imagePreviewView = new ImagePreviewView(this);
    d->welcomePageView  = new WelcomePageView(this);
    d->mediaPlayerView  = new MediaPlayerView(this);

    addWidget(d->albumIconView,           PreviewAlbumMode);
    addWidget(d->imagePreviewView,        PreviewImageMode);
    addWidget(d->welcomePageView->view(), WelcomePageMode);
    addWidget(d->mediaPlayerView,         MediaPlayerMode);

    setPreviewMode(PreviewAlbumMode);

    // -----------------------------------------------------------------

    connect(d->imagePreviewView, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->imagePreviewView, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    connect(d->imagePreviewView, SIGNAL(signalEditItem()),
            this, SIGNAL(signalEditItem()));

    connect(d->imagePreviewView, SIGNAL(signalDeleteItem()),
            this, SIGNAL(signalDeleteItem()));

    connect(d->imagePreviewView, SIGNAL(signalPreviewLoaded()),
            this, SLOT(slotPreviewLoaded()));

    connect(d->imagePreviewView, SIGNAL(signalBack2Album()),
            this, SIGNAL(signalBack2Album()));
}

AlbumWidgetStack::~AlbumWidgetStack()
{
    delete d;
}

void AlbumWidgetStack::slotEscapePreview()
{
    if (previewMode() == MediaPlayerMode)
        d->mediaPlayerView->escapePreview();
}

AlbumIconView* AlbumWidgetStack::albumIconView()
{
    return d->albumIconView;
}

ImagePreviewView* AlbumWidgetStack::imagePreviewView()
{
    return d->imagePreviewView;
}

void AlbumWidgetStack::setPreviewItem(ImageInfo* info, ImageInfo *previous, ImageInfo *next)
{
    if (!info)
    {
        if (previewMode() == MediaPlayerMode)
            d->mediaPlayerView->setMediaPlayerFromUrl(KURL());
        else if (previewMode() == PreviewImageMode)
        {
            d->imagePreviewView->setImageInfo();
        }
    }    
    else
    {
        AlbumSettings *settings      = AlbumSettings::instance();
        QString currentFileExtension = QFileInfo(info->kurl().path()).extension(false);
        QString mediaplayerfilter    = settings->getMovieFileFilter().lower() +
                                       settings->getMovieFileFilter().upper() +
                                       settings->getAudioFileFilter().lower() +
                                       settings->getAudioFileFilter().upper();
        if (mediaplayerfilter.contains(currentFileExtension) )
        {
            setPreviewMode(AlbumWidgetStack::MediaPlayerMode);
            d->mediaPlayerView->setMediaPlayerFromUrl(info->kurl());
        }
        else
        {
            // Stop media player if running...
            if (previewMode() == MediaPlayerMode)
                setPreviewItem();

            d->imagePreviewView->setImageInfo(info, previous, next);

            // NOTE: No need to toggle imediatly in PreviewImageMode here, 
            // because we will recieve a signal for that when the image preview will be loaded.
            // This will prevent a flicker effect with the old image preview loaded in stack.
        }
    }
}

int AlbumWidgetStack::previewMode(void)
{
    return id(visibleWidget());
}

void AlbumWidgetStack::setPreviewMode(int mode)
{
    if (mode != PreviewAlbumMode && mode != PreviewImageMode && 
        mode != WelcomePageMode  && mode != MediaPlayerMode)
        return;

    if (mode == PreviewAlbumMode || mode == WelcomePageMode)
    {
        d->albumIconView->setFocus();   
        setPreviewItem();
        emit signalToggledToPreviewMode(false);
    }
    else 
        emit signalToggledToPreviewMode(true);

    raiseWidget(mode);
}

void AlbumWidgetStack::slotPreviewLoaded()
{
    setPreviewMode(PreviewImageMode);
}

void AlbumWidgetStack::slotItemsUpdated(const KURL::List& list)
{
    // If item are updated from Icon View, and if we are in Preview Mode,
    // We will check if the current item preview need to be reloaded.

    if (previewMode() == AlbumWidgetStack::PreviewAlbumMode ||
        previewMode() == AlbumWidgetStack::WelcomePageMode  ||
        previewMode() == AlbumWidgetStack::MediaPlayerMode)    // What we can do with media player ?
        return;

    if (list.contains(imagePreviewView()->getImageInfo()->kurl()))
        d->imagePreviewView->reload();
}

}  // namespace Digikam
