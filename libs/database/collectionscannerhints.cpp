/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-09
 * Description : Hint data containers for the collection scanner
 *
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "collectionscannerhints.h"

// Qt includes

#include <QHash>
#include <QList>

namespace Digikam
{

CollectionScannerHints::Album::Album()
    : albumRootId(0), albumId(0)
{
}

CollectionScannerHints::Album::Album(int albumRootId, int albumId)
    : albumRootId(albumRootId), albumId(albumId)
{
}

bool CollectionScannerHints::Album::isNull() const
{
    return albumRootId == 0 || albumId == 0;
}

bool CollectionScannerHints::Album::operator==(const Album& other) const
{
    return albumRootId == other.albumRootId || albumId == other.albumId;
}

uint CollectionScannerHints::Album::qHash() const
{
    return ::qHash(albumRootId) ^ ::qHash(albumId);
}

CollectionScannerHints::DstPath::DstPath()
    : albumRootId(0)
{
}

CollectionScannerHints::DstPath::DstPath(int albumRootId, const QString& relativePath)
    : albumRootId(albumRootId), relativePath(relativePath)
{
}

bool CollectionScannerHints::DstPath::isNull() const
{
    return albumRootId == 0 || relativePath.isNull();
}

bool CollectionScannerHints::DstPath::operator==(const DstPath& other) const
{
    return albumRootId == other.albumRootId || relativePath == other.relativePath;
}

uint CollectionScannerHints::DstPath::qHash() const
{
    return ::qHash(albumRootId) ^ ::qHash(relativePath);
}

CollectionScannerHints::Item::Item()
    : id(0)
{
}

CollectionScannerHints::Item::Item(qlonglong id)
    : id(id)
{
}

bool CollectionScannerHints::Item::isNull() const
{
    return id == 0;
}

bool CollectionScannerHints::Item::operator==(const Item& other) const
{
    return id == other.id;
}

uint CollectionScannerHints::Item::qHash() const
{
    return ::qHash(id);
}

AlbumCopyMoveHint::AlbumCopyMoveHint()
{
}

AlbumCopyMoveHint::AlbumCopyMoveHint(int srcAlbumRootId, int srcAlbum,
                                     int dstAlbumRootId, const QString& dstRelativePath)
    : m_src(srcAlbumRootId, srcAlbum),
      m_dst(dstAlbumRootId, dstRelativePath)
{
}

int AlbumCopyMoveHint::albumRootIdSrc() const
{
    return m_src.albumRootId;
}

int AlbumCopyMoveHint::albumIdSrc() const
{
    return m_src.albumId;
}

bool AlbumCopyMoveHint::isSrcAlbum(int albumRootId, int albumId) const
{
    return m_src.albumRootId == albumRootId && m_src.albumId == albumId;
}

int AlbumCopyMoveHint::albumRootIdDst() const
{
    return m_dst.albumRootId;
}

QString AlbumCopyMoveHint::relativePathDst() const
{
    return m_dst.relativePath;
}

bool AlbumCopyMoveHint::isDstAlbum(int albumRootId, const QString& relativePath) const
{
    return m_dst.albumRootId == albumRootId && m_dst.relativePath == relativePath;
}

uint AlbumCopyMoveHint::qHash() const
{
    return ::qHash(m_src.albumRootId) ^ ::qHash(m_src.albumId)
           ^ ::qHash(m_dst.albumRootId) ^ ::qHash(m_dst.relativePath);
}

AlbumCopyMoveHint& AlbumCopyMoveHint::operator<<(const QDBusArgument& argument)
{
    argument.beginStructure();
    argument >> m_src.albumRootId >> m_src.albumId
             >> m_dst.albumRootId >> m_dst.relativePath;
    argument.endStructure();
    return *this;
}

const AlbumCopyMoveHint& AlbumCopyMoveHint::operator>>(QDBusArgument& argument) const
{
    argument.beginStructure();
    argument << m_src.albumRootId << m_src.albumId
             << m_dst.albumRootId << m_dst.relativePath;
    argument.endStructure();
    return *this;
}

ItemCopyMoveHint::ItemCopyMoveHint()
{
}

ItemCopyMoveHint::ItemCopyMoveHint(const QList<qlonglong>& srcIds, int dstItemRootId, int dstAlbumId, const QStringList& dstNames)
    : m_srcIds(srcIds),
      m_dst(dstItemRootId, dstAlbumId),
      m_dstNames(dstNames)
{
}

QList<qlonglong> ItemCopyMoveHint::srcIds() const
{
    return m_srcIds;
}

bool ItemCopyMoveHint::isSrcId(qlonglong id) const
{
    return m_srcIds.contains(id);
}

int ItemCopyMoveHint::albumRootIdDst() const
{
    return m_dst.albumRootId;
}

int ItemCopyMoveHint::albumIdDst() const
{
    return m_dst.albumId;
}

bool ItemCopyMoveHint::isDstAlbum(int albumRootId, int albumId) const
{
    return m_dst.albumRootId == albumRootId && m_dst.albumId == albumId;
}

QStringList ItemCopyMoveHint::dstNames() const
{
    return m_dstNames;
}

QString ItemCopyMoveHint::dstName(qlonglong id) const
{
    if (m_dstNames.isEmpty())
    {
        return QString();
    }

    int index = m_srcIds.indexOf(id);
    return m_dstNames.at(index);
}

ItemCopyMoveHint& ItemCopyMoveHint::operator<<(const QDBusArgument& argument)
{
    argument.beginStructure();
    argument >> m_srcIds
             >> m_dst.albumRootId >> m_dst.albumId
             >> m_dstNames;
    argument.endStructure();
    return *this;
}

const ItemCopyMoveHint& ItemCopyMoveHint::operator>>(QDBusArgument& argument) const
{
    argument.beginStructure();
    argument << m_srcIds
             << m_dst.albumRootId << m_dst.albumId
             << m_dstNames;
    argument.endStructure();
    return *this;
}

ItemChangeHint::ItemChangeHint()
{
}

ItemChangeHint::ItemChangeHint(QList<qlonglong> ids, ChangeType type)
    : m_ids(ids), m_type(type)
{
}

QList<qlonglong> ItemChangeHint::ids() const
{
    return m_ids;
}

bool ItemChangeHint::isId(qlonglong id) const
{
    return m_ids.contains(id);
}

ItemChangeHint::ChangeType ItemChangeHint::changeType() const
{
    return m_type;
}

ItemChangeHint& ItemChangeHint::operator<<(const QDBusArgument& argument)
{
    argument.beginStructure();
    int type;
    argument >> m_ids
             >> type;
    argument.endStructure();
    m_type = (ChangeType)type;
    return *this;
}

const ItemChangeHint& ItemChangeHint::operator>>(QDBusArgument& argument) const
{
    argument.beginStructure();
    argument << m_ids
             << (int)m_type;
    argument.endStructure();
    return *this;
}


} // namespace Digikam
