/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-10
 * Description : tool tip widget for iconview, thumbbar, and folderview items
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "ditemtooltip.h"

// Qt includes

#include <QToolTip>
#include <QLabel>
#include <QPixmap>
#include <QDateTime>
#include <QPainter>
#include <QApplication>
#include <QVBoxLayout>

// KDE includes


#include <kglobalsettings.h>
#include <kglobal.h>
#include <kdeversion.h>

namespace Digikam
{

DToolTipStyleSheet::DToolTipStyleSheet(const QFont& font)
    : maxStringLength(30)
{
    unavailable = i18n("unavailable");

    tipHeader   = QString("<qt><table cellspacing=\"0\" cellpadding=\"0\" width=\"250\" border=\"0\">");
    tipFooter   = QString("</table></qt>");

    headBeg     = QString("<tr bgcolor=\"%1\"><td colspan=\"2\">"
                            "<nobr><font size=\"-1\" color=\"%2\" face=\"%3\"><center><b>")
                            .arg(ThemeEngine::instance()->baseColor().name())
                            .arg(ThemeEngine::instance()->textRegColor().name())
                            .arg(font.family());
    headEnd     = QString("</b></center></font></nobr></td></tr>");

    cellBeg     = QString("<tr><td><nobr><font size=\"-1\" color=\"%1\" face=\"%2\">")
                            .arg(ThemeEngine::instance()->textRegColor().name())
                            .arg(font.family());
    cellMid     = QString("</font></nobr></td><td><nobr><font size=\"-1\" color=\"%1\" face=\"%2\">")
                            .arg(ThemeEngine::instance()->textRegColor().name())
                            .arg(font.family());
    cellEnd     = QString("</font></nobr></td></tr>");

    cellSpecBeg = QString("<tr><td><nobr><font size=\"-1\" color=\"%1\" face=\"%2\">")
                            .arg(ThemeEngine::instance()->textRegColor().name())
                            .arg(font.family());
    cellSpecMid = QString("</font></nobr></td><td><nobr><font size=\"-1\" color=\"%1\" face=\"%2\"><i>")
                            .arg(ThemeEngine::instance()->textSpecialRegColor().name())
                            .arg(font.family());
    cellSpecEnd = QString("</i></font></nobr></td></tr>");
}

QString DToolTipStyleSheet::breakString(const QString& input)
{
    QString str = input.simplified();
    str         = Qt::escape(str);

    if (str.length() <= maxStringLength)
        return str;

    QString br;

    int i     = 0;
    int count = 0;

    while (i < str.length())
    {
        if (count >= maxStringLength && str[i].isSpace())
        {
            count = 0;
            br.append("<br/>");
        }
        else
        {
            br.append(str[i]);
        }

        ++i;
        ++count;
    }
    return br;
}

QString DToolTipStyleSheet::elidedText(const QString& str, Qt::TextElideMode elideMode)
{
    if (str.length() <= maxStringLength)
        return str;

    switch (elideMode)
    {
        case Qt::ElideLeft:
            return "..." + str.right(maxStringLength-3);
        case Qt::ElideRight:
            return str.left(maxStringLength-3) + "...";
        case Qt::ElideMiddle:
            return str.left(maxStringLength / 2 - 2) + "..." + str.right(maxStringLength / 2 - 1);
        case Qt::ElideNone:
            return str.left(maxStringLength);
        default:
            return str;
    }
}

class DItemToolTipPriv
{
public:

    DItemToolTipPriv() :
        tipBorder(5)
    {
        corner = 0;
        label  = 0;
    }

    const uint  tipBorder;

    int         corner;

    QLabel     *label;

    QPixmap     corners[4];
};

DItemToolTip::DItemToolTip()
            : QFrame(0), d(new DItemToolTipPriv)
{
    hide();

    setFrameStyle(QFrame::Plain | QFrame::Box);
    setLineWidth(1);
    setWindowFlags(Qt::ToolTip);

    QVBoxLayout *layout = new QVBoxLayout(this);

    d->label = new QLabel(this);
    d->label->setMargin(0);
    d->label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    layout->addWidget(d->label);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setMargin(d->tipBorder+1);
    layout->setSpacing(0);

    renderArrows();
}

DItemToolTip::~DItemToolTip()
{
    delete d;
}

void DItemToolTip::updateToolTip()
{
    renderArrows();
    d->label->setText(tipContents());
}

bool DItemToolTip::toolTipIsEmpty() const
{
    return(d->label->text().isEmpty());
}

void DItemToolTip::reposition()
{
    QRect rect = repositionRect();
    if (rect.isNull())
        return;

    QPoint pos = rect.center();
    // d->corner:
    // 0: upperleft
    // 1: upperright
    // 2: lowerleft
    // 3: lowerright

    d->corner = 0;
    // should the tooltip be shown to the left or to the right of the ivi ?

    QRect desk = KGlobalSettings::desktopGeometry(rect.center());

    if (rect.center().x() + width() > desk.right())
    {
        // to the left
        if (pos.x() - width() < 0)
        {
            pos.setX(0);
            d->corner = 4;
        }
        else
        {
            pos.setX( pos.x() - width() );
            d->corner = 1;
        }
    }

    // should the tooltip be shown above or below the ivi ?
    if (rect.bottom() + height() > desk.bottom())
    {
        // above
        pos.setY( rect.top() - height() - 5);
        d->corner += 2;
    }
    else
    {
        pos.setY( rect.bottom() + 5 );
    }

    move( pos );
}

void DItemToolTip::renderArrows()
{
    int w = d->tipBorder;

    // -- left top arrow -------------------------------------

    QPixmap& pix0 = d->corners[0];
    pix0          = QPixmap(w, w);
    pix0.fill(ThemeEngine::instance()->baseColor());

    QPainter p0(&pix0);
    p0.setPen(QPen(ThemeEngine::instance()->textRegColor(), 1));

    for (int j=0; j<w; ++j)
        p0.drawLine(0, j, w-j-1, j);

    p0.end();

    // -- right top arrow ------------------------------------

    QPixmap& pix1 = d->corners[1];
    pix1          = QPixmap(w, w);
    pix1.fill(ThemeEngine::instance()->baseColor());

    QPainter p1(&pix1);
    p1.setPen(QPen(ThemeEngine::instance()->textRegColor(), 1));

    for (int j=0; j<w; ++j)
        p1.drawLine(j, j, w-1, j);

    p1.end();

    // -- left bottom arrow ----------------------------------

    QPixmap& pix2 = d->corners[2];
    pix2          = QPixmap(w, w);
    pix2.fill(ThemeEngine::instance()->baseColor());

    QPainter p2(&pix2);
    p2.setPen(QPen(ThemeEngine::instance()->textRegColor(), 1));

    for (int j=0; j<w; ++j)
        p2.drawLine(0, j, j, j);

    p2.end();

    // -- right bottom arrow ---------------------------------

    QPixmap& pix3 = d->corners[3];
    pix3          = QPixmap(w, w);
    pix3.fill(ThemeEngine::instance()->baseColor());

    QPainter p3(&pix3);
    p3.setPen(QPen(ThemeEngine::instance()->textRegColor(), 1));

    for (int j=0; j<w; ++j)
        p3.drawLine(w-j-1, j, w-1, j);

    p3.end();
}

bool DItemToolTip::event(QEvent *e)
{
    switch ( e->type() )
    {
        case QEvent::Leave:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        case QEvent::Wheel:
            hide();
        default:
            break;
    }

    return QFrame::event(e);
}

void DItemToolTip::resizeEvent(QResizeEvent* e)
{
    QFrame::resizeEvent(e);
    reposition();
}

void DItemToolTip::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);

    if (d->corner >= 4)
        return;

    QPainter p(this);
    QPixmap &pix = d->corners[d->corner];

    switch (d->corner)
    {
        case 0:
            p.drawPixmap( 3, 3, pix );
            break;
        case 1:
            p.drawPixmap( width() - pix.width() - 3, 3, pix );
            break;
        case 2:
            p.drawPixmap( 3, height() - pix.height() - 3, pix );
            break;
        case 3:
            p.drawPixmap( width() - pix.width() - 3, height() - pix.height() - 3, pix );
            break;
    }
}

}  // namespace Digikam
