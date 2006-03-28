/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-23
 * Description : image metadata interface
 *
 * Copyright 2006 by Gilles Caulier
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

#ifndef DMETADATA_H
#define DMETADATA_H

// QT includes.

#include <qcstring.h>
#include <qimage.h>

// Local includes.

#include "dimg.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DMetadata
{

public:

    enum ImageOrientation
    {
        ORIENTATION_UNSPECIFIED  = 0, 
        ORIENTATION_NORMAL       = 1, 
        ORIENTATION_HFLIP        = 2, 
        ORIENTATION_ROT_180      = 3, 
        ORIENTATION_VFLIP        = 4, 
        ORIENTATION_ROT_90_HFLIP = 5, 
        ORIENTATION_ROT_90       = 6, 
        ORIENTATION_ROT_90_VFLIP = 7, 
        ORIENTATION_ROT_270      = 8
    };

public:

    DMetadata()  {};
    ~DMetadata() {};
    
    /** Load Metadata from image file */
    DMetadata(const QString& filePath, DImg::FORMAT ff=DImg::NONE);

    bool load(const QString& filePath, DImg::FORMAT ff=DImg::NONE);
    bool save(const QString& filePath, const QString& format);

    /** Metadata manipulation methods */
    QByteArray       getExif() const;
    QByteArray       getIptc() const;
    QImage           getExifThumbnail(bool fixOrientation) const;
    ImageOrientation getExifImageOrientation();

    void setExif(const QByteArray& data);
    void setIptc(const QByteArray& data);

private:

    DImg::FORMAT fileFormat(const QString& filePath);

private:

    QByteArray m_exifMetadata;
    QByteArray m_iptcMetadata;

    friend class DMetaLoader;
};

}  // NameSpace Digikam

#endif /* DMETADATA_H */
