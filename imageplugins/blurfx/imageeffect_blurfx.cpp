/* ============================================================
 * File  : imageeffect_blurfx.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-09
 * Description : 
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * Original Blur algorithms copyrighted 2004 by 
 * Pieter Z. Voloshyn <pieter_voloshyn at ame.com.br>.
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

#define ANGLE_RATIO        0.017453292519943295769236907685   // Represents 1� 
 
// C++ include.

#include <cstring>
#include <cmath>
#include <cstdlib>
 
// Qt includes. 
 
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qslider.h>
#include <qimage.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qdatetime.h> 
#include <qtimer.h>

// KDE includes.

#include <klocale.h>
#include <kcursor.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kprogress.h>
#include <knuminput.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "imageeffect_blurfx.h"


namespace DigikamBlurFXImagesPlugin
{

ImageEffect_BlurFX::ImageEffect_BlurFX(QWidget* parent)
                  : KDialogBase(Plain, i18n("Blur Effects"),
                                Help|User1|Ok|Cancel, Ok,
                                parent, 0, true, true, i18n("&Reset Values")),
                    m_parent(parent)
{
    m_timer = 0;
    QString whatsThis;
    
    setButtonWhatsThis( User1, i18n("<p>Reset all parameters to the default values.") );
    m_cancel = false;
    m_dirty = false;
    
    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Blur Effects"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to apply blurring special effect to an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
                                       
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    about->addAuthor("Pieter Z. Voloshyn", I18N_NOOP("Blurring algorithms"), 
                     "pieter_voloshyn at ame.com.br"); 
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Blur Effects Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------
        
    QGridLayout* topLayout = new QGridLayout( plainPage(), 2, 2 , marginHint(), spacingHint());

    QFrame *headerFrame = new QFrame( plainPage() );
    headerFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( headerFrame );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );
    QLabel *pixmapLabelLeft = new QLabel( headerFrame, "pixmapLabelLeft" );
    pixmapLabelLeft->setScaledContents( false );
    layout->addWidget( pixmapLabelLeft );
    QLabel *labelTitle = new QLabel( i18n("Apply Blurring Special Effect to Image"), headerFrame, "labelTitle" );
    layout->addWidget( labelTitle );
    layout->setStretchFactor( labelTitle, 1 );
    topLayout->addMultiCellWidget(headerFrame, 0, 0, 0, 1);
    
    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") +
                                                                        "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left",
                                                 "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );

    // -------------------------------------------------------------
    
    QVGroupBox *gbox = new QVGroupBox(i18n("Preview"), plainPage());
    QFrame *frame = new QFrame(gbox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_previewWidget = new Digikam::ImageWidget(480, 320, frame);
    l->addWidget(m_previewWidget, 0, Qt::AlignCenter);
    QWhatsThis::add( m_previewWidget, i18n("<p>This is the preview of the blurring special effect.") );
    topLayout->addMultiCellWidget(gbox, 1, 1, 0, 0);
    
    // -------------------------------------------------------------
    
    QGroupBox *gbox2 = new QGroupBox(i18n("Settings"), plainPage());
    QGridLayout *gridBox2 = new QGridLayout( gbox2, 3, 2, marginHint(), spacingHint());
    
    m_effectTypeLabel = new QLabel(i18n("Type:"), gbox2);
    
    m_effectType = new QComboBox( false, gbox2 );
    m_effectType->insertItem( i18n("Zoom Blur") );
    m_effectType->insertItem( i18n("Radial Blur") );
    m_effectType->insertItem( i18n("Far Blur") );
    m_effectType->insertItem( i18n("Motion Blur") );
    m_effectType->insertItem( i18n("Softner Blur") );
    m_effectType->insertItem( i18n("Skake Blur") );
    m_effectType->insertItem( i18n("Focus Blur") );
    m_effectType->insertItem( i18n("Frost Glass") );
    QWhatsThis::add( m_effectType, i18n("<p>Select here the effect type to apply on image.<p>"
                                        "<b>Zoom Blur</b>:  blurs the image along radial lines starting from "
                                        "a specified center point. This simulates the blur of a zooming camera.<p>"
                                        "<b>Radial Blur</b>: blurs the image by rotating the pixels around "
                                        "the specified center point. This simulates the blur of a rotating camera.<p>"
                                        "<b>Far Blur</b>: blurs the image by using far pixels. This simulates the blur "
                                        "of an unfocalized camera lens.<p>"
                                        "<b>Motion Blur</b>: blurs the image by moving the pixels horizontally. "
                                        "This simulates the blur of a linear moving camera.<p>"
                                        "<b>Softner Blur</b>: blurs the image softly in dark tones and hardly in light "
                                        "tones. This gives images a dreamy and glossy soft focus effect. It's ideal "
                                        "for creating romantic portraits, glamour photographs, or giving images a warm "
                                        "and subtle glow.<p>"
                                        "<b>Skake Blur</b>: blurs the image by skaking randomly the pixels. "
                                        "This simulates the blur of a random moving camera.<p>"
                                        "<b>Focus Blur</b>: blurs the image corners to reproduce the astigmatism distortion "
                                        "of a lens.<p>"
                                        "<b>Frost Glass</b>: blurs the image by randomly disperse light coming through "
                                        "a frosted glass."));
    gridBox2->addMultiCellWidget(m_effectTypeLabel, 0, 0, 0, 0);
    gridBox2->addMultiCellWidget(m_effectType, 0, 0, 1, 2);
                                                  
    m_distanceLabel = new QLabel(i18n("Level:"), gbox2);
    m_distanceInput = new KIntNumInput(gbox2);
    m_distanceInput->setRange(0, 200, 1, true);
    QWhatsThis::add( m_distanceInput, i18n("<p>Set here the blur distance in pixels."));
    
    gridBox2->addMultiCellWidget(m_distanceLabel, 1, 1, 0, 0);
    gridBox2->addMultiCellWidget(m_distanceInput, 1, 1, 1, 1);
        
    m_angleLabel = new QLabel(i18n("Angle:"), gbox2);
    m_angleInput = new KIntNumInput(gbox2);
    m_angleInput->setRange(0, 360, 1, true);
    QWhatsThis::add( m_angleInput, i18n("<p>This value controls the angle to use for motion blur effect."));  
    
    gridBox2->addMultiCellWidget(m_angleLabel, 2, 2, 0, 0);
    gridBox2->addMultiCellWidget(m_angleInput, 2, 2, 1, 1);
    
    m_progressBar = new KProgress(100, gbox2, "progressbar");
    m_progressBar->setValue(0);
    QWhatsThis::add( m_progressBar, i18n("<p>This is the current percentage of the task completed.") );
    gridBox2->addMultiCellWidget(m_progressBar, 3, 3, 0, 1);

    topLayout->addMultiCellWidget(gbox2, 1, 1, 1, 1);

    // -------------------------------------------------------------
    
    adjustSize();
    disableResize();  
    QTimer::singleShot(0, this, SLOT(slotUser1()));     // Reset all parameters to the default values.
        
    // -------------------------------------------------------------
    
    connect(m_effectType, SIGNAL(activated(int)),
            this, SLOT(slotEffectTypeChanged(int)));
    
    connect(m_distanceInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));            
    
    connect(m_angleInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));            
}

ImageEffect_BlurFX::~ImageEffect_BlurFX()
{
    if (m_timer)
       delete m_timer;
}

void ImageEffect_BlurFX::slotHelp()
{
    KApplication::kApplication()->invokeHelp("blurfx",
                                             "digikamimageplugins");
}

void ImageEffect_BlurFX::closeEvent(QCloseEvent *e)
{
    m_cancel = true;
    e->accept();    
}

void ImageEffect_BlurFX::slotCancel()
{
    m_cancel = true;
    done(Cancel);
}

void ImageEffect_BlurFX::slotUser1()
{
    if (m_dirty)
       {
       m_cancel = true;
       }
    else
       {
       m_effectType->setCurrentItem(5); // Shake blur
       slotEffectTypeChanged(5);
       }
} 

void ImageEffect_BlurFX::slotTimer()
{
    if (m_timer)
       {
       m_timer->stop();
       delete m_timer;
       }
    
    m_timer = new QTimer( this );
    connect( m_timer, SIGNAL(timeout()),
             this, SLOT(slotEffect()) );
    m_timer->start(500, true);
}

void ImageEffect_BlurFX::slotEffectTypeChanged(int type)
{
    m_distanceInput->setEnabled(true);
    m_distanceLabel->setEnabled(true);
    
    m_distanceInput->blockSignals(true);
    m_angleInput->blockSignals(true);
    m_distanceInput->setRange(0, 200, 1, true);
    m_distanceInput->setValue(100);
    
    m_angleInput->setEnabled(false);
    m_angleLabel->setEnabled(false);
          
    switch (type)
       {
       case 0: // Zoom Blur.
          break;
       
       case 1: // Radial Blur.
       case 7: // Frost Glass.
          m_distanceInput->setRange(0, 10, 1, true);
          m_distanceInput->setValue(3);
          break;
          
       case 2: // Far Blur.
          m_distanceInput->setRange(0, 20, 1, true);
          m_distanceInput->setMaxValue(20);
          m_distanceInput->setValue(10);
          break;
       
       case 3: // Motion Blur.
          m_distanceInput->setRange(0, 100, 1, true);
          m_distanceInput->setValue(20);
          m_angleInput->setEnabled(true);
          m_angleLabel->setEnabled(true);
          break;

       case 4: // Softner Blur.
          m_distanceInput->setEnabled(false);
          m_distanceLabel->setEnabled(false);
          break;
          
       case 5: // Skake Blur    
       case 6: // Focus Blur.
          m_distanceInput->setRange(0, 100, 1, true);
          m_distanceInput->setValue(20);
          break;
       }

    m_distanceInput->blockSignals(false);
    m_angleInput->blockSignals(false);
       
    slotEffect();
}

void ImageEffect_BlurFX::slotEffect()
{
    m_dirty = true;
    m_parent->setCursor( KCursor::waitCursor() );
    setButtonText(User1, i18n("&Abort"));
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    enableButton(Ok, false);
        
    m_effectTypeLabel->setEnabled(false);
    m_effectType->setEnabled(false);
    m_distanceInput->setEnabled(false);
    m_distanceLabel->setEnabled(false);
    m_angleInput->setEnabled(false);
    m_angleLabel->setEnabled(false);

    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    // Preview image size.
    int wp      = iface->previewWidth();
    int hp      = iface->previewHeight();
    
    // All data from the image
    uint* data  = iface->getOriginalData();
    int w       = iface->originalWidth();
    int h       = iface->originalHeight();
    
    int d       = m_distanceInput->value();
    int a       = m_angleInput->value();

    m_progressBar->setValue(0); 

    switch (m_effectType->currentItem())
       {
       case 0: // Zoom Blur.
          zoomBlur(data, w, h, w/2, h/2, d);
          break;

       case 1: // Radial Blur.
          radialBlur(data, w, h, w/2, h/2, d);
          break;
       
       case 2: // Far Blur.
          farBlur(data, w, h, d);
          break;

       case 3: // Motion Blur.
          motionBlur(data, w, h, d, (double)a);
          break;

       case 4: // Softner Blur.
          softnerBlur(data, w, h);
          break;

       case 5: // Shake Blur.
          shakeBlur(data, w, h, d);
          break;
                              
       case 6: // Focus Blur.
          focusBlur(data, w, h, w/2, h/2, 10, d*10);
          break;
       
       case 7: // Frost Glass.
          frostGlass(data, w, h, d);
          break;
       }
    
    if ( !m_cancel ) 
       {
       QImage newImg;
       newImg.create( w, h, 32 );
       memcpy(newImg.bits(), data, newImg.numBytes());
       QImage destImg = newImg.smoothScale(wp, hp);
       iface->putPreviewData((uint*)destImg.bits());
       }
                  
    delete [] data;
    m_progressBar->setValue(0); 
    m_previewWidget->update();

    m_effectTypeLabel->setEnabled(true);
    m_effectType->setEnabled(true);
    m_distanceInput->setEnabled(true);
    m_distanceLabel->setEnabled(true);
    
    switch (m_effectType->currentItem())
       {
       case 0: // Zoom Blur.
       case 1: // Radial Blur.
       case 2: // Far Blur.
       case 5: // Shake Blur.
       case 6: // Focus Blur.
       case 7: // Frost Glass.
          break;

       case 3: // Motion Blur.
          m_angleInput->setEnabled(true);
          m_angleLabel->setEnabled(true);
          break;

       case 4: // Softner Blur.
          m_distanceInput->setEnabled(false);
          m_distanceLabel->setEnabled(false);
          break;
       }
    
    m_cancel = false;
    m_dirty = false;
    setButtonText(User1, i18n("&Reset Values"));
    setButtonWhatsThis( User1, i18n("<p>Reset all parameters to the default values.") );
    enableButton(Ok, true);
    m_parent->setCursor( KCursor::arrowCursor() );
}

void ImageEffect_BlurFX::slotOk()
{
    m_effectTypeLabel->setEnabled(false);
    m_effectType->setEnabled(false);
    m_distanceInput->setEnabled(false);
    m_distanceLabel->setEnabled(false);
    m_angleInput->setEnabled(false);
    m_angleLabel->setEnabled(false);
    
    enableButton(Ok, false);
    enableButton(User1, false);
    
    m_parent->setCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewWidget->imageIface();

    uint* data  = iface->getOriginalData();
    int w       = iface->originalWidth();
    int h       = iface->originalHeight();
    int d       = m_distanceInput->value();
    int a       = m_angleInput->value();

    m_progressBar->setValue(0); 
        
    if (data) 
       {
       switch (m_effectType->currentItem())
          {
          case 0: // Zoom Blur.
             zoomBlur(data, w, h, w/2, h/2, d);
             break;

          case 1: // Radial Blur.
             radialBlur(data, w, h, w/2, h/2, d);
             break;
       
          case 2: // Far Blur.
             farBlur(data, w, h, d);
             break;

          case 3: // Motion Blur.
             motionBlur(data, w, h, d, (double)a);
             break;

          case 4: // Softner Blur.
             softnerBlur(data, w, h);
             break;
          
          case 5: // Shake Blur.
             shakeBlur(data, w, h, d);
             break;
          
          case 6: // Focus Blur.
             focusBlur(data, w, h, w/2, h/2, 10, d*10);
             break;
             
          case 7: // Frost Glass
             frostGlass(data, w, h, d);
             break;
          }
       
       if ( !m_cancel ) iface->putOriginalData(data);
       }
    
    delete [] data;    
    m_parent->setCursor( KCursor::arrowCursor() );        
    accept();
}

/* Function to apply the ZoomBlur effect backported from ImageProcessing version 2                                           
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.  
 * X, Y             => Center of zoom in the image
 * Distance         => Distance value                                            
 *                                                                                  
 * Theory           => Here we have a effect similar to RadialBlur mode Zoom from  
 *                     Photoshop. The theory is very similar to RadialBlur, but has one
 *                     difference. Instead we use pixels with the same radius and      
 *                     near angles, we take pixels with the same angle but near radius 
 *                     This radius is always from the center to out of the image, we   
 *                     calc a proportional radius from the center.
 */
void ImageEffect_BlurFX::zoomBlur(uint *data, int Width, int Height, int X, int Y, int Distance)
{
    if (Distance <= 1) return;

    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int    BitCount = LineWidth * Height;
    uchar*    pBits = (uchar*)data;
    uchar* pResBits = new uchar[BitCount];

    register int h, w, nh, nw, i, j, r;
    int nStride = GetStride(Width);
    int sumR, sumG, sumB, nCount;
    double lfRadius, lfNewRadius, lfRadMax, lfAngle;
    
    lfRadMax = sqrt (Height * Height + Width * Width);

    // total of bits to be taken is given by this formula
    nCount = 0;

    // we have to initialize all loop and positions valiables
    i = j = sumR = sumG = sumB = 0;

    // we have reached the main loop
    for (h = 0; !m_cancel && (h < Height); h++, i += nStride)
        {
        for (w = 0; !m_cancel && (w < Width); w++, i += 4)
            {
            // ...we enter this loop to sum the bits
            nw = X - w;
            nh = Y - h;

            lfRadius = sqrt (nw * nw + nh * nh);
            lfAngle = atan2 (nh, nw);
            lfNewRadius = (lfRadius * Distance) / lfRadMax;

            for (r = 0; !m_cancel && (r <= lfNewRadius); r++)
                {
                // we need to calc the positions
                nw = (int)(X - (lfRadius - r) * cos (lfAngle));
                nh = (int)(Y - (lfRadius - r) * sin (lfAngle));

                if (IsInside(Width, Height, nw, nh))
                    {
                    // we adjust the positions
                    j = SetPosition(Width, nw, nh);
                    // finally we sum the bits
                    sumR += pBits[ j ];
                    sumG += pBits[j+1];
                    sumB += pBits[j+2];
                    nCount++;
                    }
                }
            
            if (nCount == 0) nCount = 1;                    

            // now, we have to calc the arithmetic average
            pResBits[ i ] = sumR / nCount;
            pResBits[i+1] = sumG / nCount;
            pResBits[i+2] = sumB / nCount;
            // we initialize the variables
            sumR = sumG = sumB = nCount = 0;
            }
        
        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (((double)h * 100.0) / Height));
        kapp->processEvents();             
        }

    if (!m_cancel) 
       memcpy (data, pResBits, BitCount);        
                
    delete [] pResBits;
}

/* Function to apply the farBlur effect backported from ImageProcessing version 2                                            
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Distance         => Distance value                                            
 *                                                                                  
 * Theory           => This is an interesting effect, the blur is applied in that   
 *                     way: (the value "1" means pixel to be used in a blur calc, ok?)
 *                     e.g. With distance = 2 
 *                                            |1|1|1|1|1| 
 *                                            |1|0|0|0|1| 
 *                                            |1|0|C|0|1| 
 *                                            |1|0|0|0|1| 
 *                                            |1|1|1|1|1| 
 *                     We sum all the pixels with value = 1 and apply at the pixel with*
 *                     the position "C".
 */
void ImageEffect_BlurFX::farBlur(uint *data, int Width, int Height, int Distance)
{
    if (Distance < 1) return;

    // we need to create our kernel
    // e.g. distance = 3, so kernel={3 1 1 2 1 1 3}
    
    int *nKern = new int[Distance * 2 + 1];
    
    for (int i = 0; i < Distance * 2 + 1; i++)
        {
        // the first element is 3
        if (i == 0)
            nKern[i] = 2;
        // the center element is 2
        else if (i == Distance)
            nKern[i] = 3;
        // the last element is 3
        else if (i == Distance * 2)
            nKern[i] = 3;
        // all other elements will be 1
        else
            nKern[i] = 1;
        }

    // now, we apply a convolution with kernel
    MakeConvolution(data, Width, Height, Distance, nKern);

    // we must delete to free memory
    delete [] nKern;
}

/* Function to apply the radialBlur effect backported from ImageProcessing version 2                                           
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.     
 * X, Y             => Center of radial in the image                       
 * Distance         => Distance value                                            
 *                                                                                  
 * Theory           => Similar to RadialBlur from Photoshop, its an amazing effect    
 *                     Very easy to understand but a little hard to implement.           
 *                     We have all the image and find the center pixel. Now, we analize
 *                     all the pixels and calc the radius from the center and find the 
 *                     angle. After this, we sum this pixel with others with the same  
 *                     radius, but different angles. Here I'm using degrees angles.
 */
void ImageEffect_BlurFX::radialBlur(uint *data, int Width, int Height, int X, int Y, int Distance)
{
    if (Distance <= 1) return;

    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int    BitCount = LineWidth * Height;
    uchar*    pBits = (uchar*)data;
    uchar* pResBits = new uchar[BitCount];

    int nStride = GetStride(Width);
    register int sumR, sumG, sumB, i, j, nw, nh;
    double Radius, Angle, AngleRad;
    
    double *nMultArray = new double[Distance * 2 + 1];
    
    for (i = -Distance; i <= Distance; i++)
        nMultArray[i + Distance] = i * ANGLE_RATIO;

    // total of bits to be taken is given by this formula
    int nCount = 0;

    // we have to initialize all loop and positions valiables
    i = j = sumR = sumG = sumB = 0;

    // we have reached the main loop
    
    for (int h = 0; !m_cancel && (h < Height); h++, i += nStride)
        {
        for (int w = 0; !m_cancel && (w < Width); w++, i += 4)
            {
            // ...we enter this loop to sum the bits
            nw = X - w;
            nh = Y - h;

            Radius = sqrt (nw * nw + nh * nh);
            AngleRad = atan2 (nh, nw);
            
            for (int a = -Distance; !m_cancel && (a <= Distance); a++)
                {
                Angle = AngleRad + nMultArray[a + Distance];
                // we need to calc the positions
                nw = (int)(X - Radius * cos (Angle));
                nh = (int)(Y - Radius * sin (Angle));

                if (IsInside(Width, Height, nw, nh))
                    {
                    // we adjust the positions
                    j = SetPosition (Width, nw, nh);
                    // finally we sum the bits
                    sumR += pBits[ j ];
                    sumG += pBits[j+1];
                    sumB += pBits[j+2];
                    nCount++;
                    }
                }

            if (nCount == 0) nCount = 1;                    
                
            // now, we have to calc the arithmetic average
            pResBits[ i ] = sumR / nCount;
            pResBits[i+1] = sumG / nCount;
            pResBits[i+2] = sumB / nCount;
            // we initialize the variables
            sumR = sumG = sumB = nCount = 0;
            }

        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (((double)h * 100.0) / Height));
        kapp->processEvents();             
        }

    if (!m_cancel) 
       memcpy (data, pResBits, BitCount);        
                
    delete [] pResBits;
    delete [] nMultArray;
}

/* Function to apply the motionBlur effect backported from ImageProcessing version 2                                              
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Distance         => Distance value 
 * Angle            => Angle direction (degrees)                                               
 *                                                                                  
 * Theory           => Similar to MotionBlur from Photoshop, the engine is very       
 *                     simple to undertand, we take a pixel (duh!), with the angle we   
 *                     will taking near pixels. After this we blur (add and do a       
 *                     division).
 */
void ImageEffect_BlurFX::motionBlur(uint *data, int Width, int Height, int Distance, double Angle)
{
    if (Distance == 0) return;

    // we try to avoid division by 0 (zero)
    if (Angle == 0.0) Angle = 360.0;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int nStride = GetStride(Width);
    register int sumR, sumG, sumB, nCount, i, j;
    double nAngX, nAngY, nw, nh;
    
    int    BitCount = LineWidth * Height;
    uchar*    pBits = (uchar*)data;
    uchar* pResBits = new uchar[BitCount];

    // we initialize cos and sin for a best performance
    nAngX = cos ((2.0 * M_PI) / (360.0 / Angle));
    nAngY = sin ((2.0 * M_PI) / (360.0 / Angle));
    
    // total of bits to be taken is given by this formula
    nCount = Distance * 2 + 1;

    // we will alloc size and calc the possible results
    double *lpXArray = new double[nCount];
    double *lpYArray = new double[nCount];
    
    for (i = 0; i < nCount; i++)
        {
        lpXArray[i] = (i - Distance) * nAngX;
        lpYArray[i] = (i - Distance) * nAngY;
        }

    // we have to initialize all loop and positions valiables
    i = j = sumR = sumG = sumB = 0;

    // we have reached the main loop
    
    for (int h = 0; !m_cancel && (h < Height); h++, i += nStride)
        {
        for (int w = 0; !m_cancel && (w < Width); w++, i += 4)
            {
            // ...we enter this loop to sum the bits
            for (int a = -Distance; !m_cancel && (a <= Distance); a++)
                {
                // we need to calc the positions
                nw = ((double)w + lpXArray[a + Distance]);
                nh = ((double)h + lpYArray[a + Distance]);
                    
                // we adjust the positions
                j = SetPositionAdjusted(Width, Height, (int)nw, (int)nh);
                // finally we sum the bits
                sumR += pBits[ j ];
                sumG += pBits[j+1];
                sumB += pBits[j+2];
                }
            
            if (nCount == 0) nCount = 1;                    

            // now, we have to calc the arithmetic average
            pResBits[ i ] = sumR / nCount;
            pResBits[i+1] = sumG / nCount;
            pResBits[i+2] = sumB / nCount;
            // we initialize the variables
            sumR = sumG = sumB = 0;
            
            pResBits[i+3] = pBits[i+3];         // Alpha channel.
            }
        
        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (((double)h * 100.0) / Height));
        kapp->processEvents();             
        }

    if (!m_cancel) 
       memcpy (data, pResBits, BitCount);        
                
    delete [] pResBits;
    delete [] lpXArray;
    delete [] lpYArray;
}

/* Function to apply the focusBlur effect backported from ImageProcessing version 2                                              
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * BlurRadius       => Raduis of blured image. 
 * Radius           => Focus distance.
 * bInversed        => If true, invert focus effect.
 *                                                                                 
 */
void ImageEffect_BlurFX::focusBlur (uint *data, int Width, int Height, int X, int Y, int BlurRadius, int Radius, bool bInversed)
{
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    register int h, w, nh, nw, i=0;
    int nStride = GetStride (Width);
    int nBlendFactor;
    double lfRadius;
    
    int    BitCount = LineWidth * Height;
    uchar*    pBits = (uchar*)data;
    uchar* pResBits = new uchar[BitCount];
            
    // Gaussian blur using the BlurRadius parameter.
    memcpy (pResBits, data, BitCount);        
    Digikam::ImageFilters::gaussianBlurImage((uint *)pResBits, Width, Height, BlurRadius);
    
    // Blending result  
    for (h = 0; !m_cancel && (h < Height); h++, i += nStride)
        {
        nh = Y - h;

        for (w = 0; !m_cancel && (w < Width); w++)
            {
            nw = X - w;

            lfRadius = sqrt (nh * nh + nw * nw);

            nBlendFactor = LimitValues ((int)(255.0 * lfRadius / (double)Radius));

            if (bInversed)
                {
                pResBits[i++] = (pResBits[i] * (255 - nBlendFactor) + pBits[i] * nBlendFactor) >> 8;    // Blue.
                pResBits[i++] = (pResBits[i] * (255 - nBlendFactor) + pBits[i] * nBlendFactor) >> 8;    // Green.
                pResBits[i++] = (pResBits[i] * (255 - nBlendFactor) + pBits[i] * nBlendFactor) >> 8;    // Red.
                }
            else
                {
                pResBits[i++] = (pBits[i] * (255 - nBlendFactor) + pResBits[i] * nBlendFactor) >> 8;    // Blue.
                pResBits[i++] = (pBits[i] * (255 - nBlendFactor) + pResBits[i] * nBlendFactor) >> 8;    // Green.
                pResBits[i++] = (pBits[i] * (255 - nBlendFactor) + pResBits[i] * nBlendFactor) >> 8;    // Red.
                }
                
            pResBits[i++] = pBits[i];       // Alpha channel.
            }
        
        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (((double)h * 100.0) / Height));
        kapp->processEvents();             
       }
    
    if (!m_cancel) 
       memcpy (data, pResBits, BitCount);        
                
    delete [] pResBits;
}

/* Function to apply the softnerBlur effect                                            
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 *                                                                                  
 * Theory           => An interesting blur-like function. In dark tones we apply a   
 *                     blur with 3x3 dimentions, in light tones, we apply a blur with   
 *                     5x5 dimentions. Easy, hun?
 */
void ImageEffect_BlurFX::softnerBlur(uint *data, int Width, int Height)
{
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    uchar* Bits = (uchar*)data;

    int SomaR = 0, SomaG = 0, SomaB = 0;
    int i, j, Gray;
        
    for (int h = 0; !m_cancel && (h < Height); h++)
        {
        for (int w = 0; !m_cancel && (w < Width); w++)
            {
            i = h * LineWidth + 4 * w;
            Gray = (Bits[i+2] + Bits[i+1] + Bits[i]) / 3;
            
            if (Gray > 127)
                {
                for (int a = -3; !m_cancel && (a <= 3); a++)
                    {
                    for (int b = -3; !m_cancel && (b <= 3); b++)
                        {
                        j = (h + Lim_Max (h, a, Height)) * LineWidth + 4 * (w + Lim_Max (w, b, Width));  
                        
                        if ((h + a < 0) || (w + b < 0))
                           j = i;
                        
                        SomaR += Bits[j+2];
                        SomaG += Bits[j+1];
                        SomaB += Bits[ j ];
                        }
                    } 
                   
                Bits[i+2] = SomaR / 49;
                Bits[i+1] = SomaG / 49;
                Bits[ i ] = SomaB / 49;
                SomaR = SomaG = SomaB = 0;
                }
            else
                {
                for (int a = -1; !m_cancel && (a <= 1); a++)
                    {
                    for (int b = -1; !m_cancel && (b <= 1); b++)
                        {
                        j = (h + Lim_Max (h, a, Height)) * LineWidth + 4 * (w + Lim_Max (w, b, Width));
                        
                        if ((h + a < 0) || (w + b < 0))
                           j = i;
                        
                        SomaR += Bits[j+2];
                        SomaG += Bits[j+1];
                        SomaB += Bits[ j ];
                        }
                    }
                                
                Bits[i+2] = SomaR / 9;
                Bits[i+1] = SomaG / 9;
                Bits[ i ] = SomaB / 9;
                SomaR = SomaG = SomaB = 0;
                }
            }

        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (((double)h * 100.0) / Height));
        kapp->processEvents();             
        }
}

/* Function to apply the shake blur effect                                            
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Distance         => Distance between layers (from origin)                        
 *                                                                                  
 * Theory           => Similar to Fragment effect from Photoshop. We create 4 layers
 *                    each one has the same distance from the origin, but have       
 *                    different positions (top, botton, left and right), with these 4 
 *                    layers, we join all the pixels.                 
 */
void ImageEffect_BlurFX::shakeBlur(uint *data, int Width, int Height, int Distance)
{
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int BitCount = LineWidth * Height;
    uchar*   Bits = (uchar*)data;
    uchar* Layer1 = new uchar[BitCount];
    uchar* Layer2 = new uchar[BitCount];
    uchar* Layer3 = new uchar[BitCount];
    uchar* Layer4 = new uchar[BitCount];
    
    register int i = 0, j = 0, h, w, nw, nh;
        
    for (h = 0; !m_cancel && (h < Height); h++)
        {
        for (w = 0; !m_cancel && (w < Width); w++)
            {
            i = h * LineWidth + 4 * w;

            nh = (h + Distance >= Height) ? Height - 1 : h + Distance;
            j = nh * LineWidth + 4 * w;
            Layer1[i+2] = Bits[j+2];
            Layer1[i+1] = Bits[j+1];
            Layer1[ i ] = Bits[ j ];

            nh = (h - Distance < 0) ? 0 : h - Distance;
            j = nh * LineWidth + 4 * w;
            Layer2[i+2] = Bits[j+2];
            Layer2[i+1] = Bits[j+1];
            Layer2[ i ] = Bits[ j ];

            nw = (w + Distance >= Width) ? Width - 1 : w + Distance;
            j = h * LineWidth + 4 * nw;
            Layer3[i+2] = Bits[j+2];
            Layer3[i+1] = Bits[j+1];
            Layer3[ i ] = Bits[ j ];

            nw = (w - Distance < 0) ? 0 : w - Distance;
            j = h * LineWidth + 4 * nw;
            Layer4[i+2] = Bits[j+2];
            Layer4[i+1] = Bits[j+1];
            Layer4[ i ] = Bits[ j ];
            }
        
        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (((double)h * 50.0) / Height));
        kapp->processEvents();         
        }
        
    for (int h = 0; !m_cancel && (h < Height); h++)
        {
        for (int w = 0; !m_cancel && (w < Width); w++)
            {
            i = h * LineWidth + 4 * w;
            Bits[i+2] = (Layer1[i+2] + Layer2[i+2] + Layer3[i+2] + Layer4[i+2]) / 4;
            Bits[i+1] = (Layer1[i+1] + Layer2[i+1] + Layer3[i+1] + Layer4[i+1]) / 4;
            Bits[ i ] = (Layer1[ i ] + Layer2[ i ] + Layer3[ i ] + Layer4[ i ]) / 4;
            }
        
        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (50.0 + ((double)h * 50.0) / Height));
        kapp->processEvents();             
        }
    
    delete [] Layer1;
    delete [] Layer2;
    delete [] Layer3;
    delete [] Layer4;
}

/* Function to apply the frostGlass effect                                            
 *                                                                                  
 * data             => The image data in RGBA mode.                            
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Frost            => Frost value 
 *                                                                                  
 * Theory           => Similar to Diffuse effect, but the random byte is defined   
 *                     in a matrix. Diffuse uses a random diagonal byte.
 */
void ImageEffect_BlurFX::frostGlass(uint *data, int Width, int Height, int Frost)
{
    Frost = (Frost < 1) ? 1 : (Frost > 10) ? 10 : Frost;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int BitCount   = LineWidth * Height;
    uchar*    Bits = (uchar*)data;
    uchar* NewBits = new uchar[BitCount];

    register int i = 0, h, w; 
    QRgb color;
        
    for (h = 0; !m_cancel && (h < Height); h++)
        {
        for (w = 0; !m_cancel && (w < Width); w++)
            {
            i = h * LineWidth + 4 * w;
            color = RandomColor (Bits, Width, Height, w, h, Frost);
            NewBits[ i ] = qRed(color);
            NewBits[i+1] = qGreen(color);
            NewBits[i+2] = qBlue(color);
            }
            
        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (((double)h * 100.0) / Height));
        kapp->processEvents();             
        }

    if (!m_cancel) 
       memcpy (data, NewBits, BitCount);        
                
    delete [] NewBits;
}

/* Function to get a color in a matriz with a determined size                       
 *                                                                                  
 * Bits              => Bits array                                                   
 * Width             => Image width                                                  
 * Height            => Image height                                                
 * X                 => Position horizontal                                          
 * Y                 => Position vertical                                            
 * Radius            => The radius of the matrix to be created                     
 *                                                                                 
 * Theory            => This function takes from a distinct matrix a random color  
 */
QRgb ImageEffect_BlurFX::RandomColor(uchar *Bits, int Width, int Height, int X, int Y, int Radius)
{
    int w, h, counter = 0;
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    uchar I;
    const uchar MAXINTENSITY = 255;

    uchar IntensityCount[MAXINTENSITY + 1];
    uint AverageColorR[MAXINTENSITY + 1];
    uint AverageColorG[MAXINTENSITY + 1];
    uint AverageColorB[MAXINTENSITY + 1];

    memset(IntensityCount, 0, sizeof(IntensityCount) ); 
    memset(AverageColorR,  0, sizeof(AverageColorR) ); 
    memset(AverageColorG,  0, sizeof(AverageColorG) ); 
    memset(AverageColorB,  0, sizeof(AverageColorB) ); 
    
    for (w = X - Radius; !m_cancel && (w <= X + Radius); w++)
        {
        for (h = Y - Radius; !m_cancel && (h <= Y + Radius); h++)
            {
            if ((w >= 0) && (w < Width) && (h >= 0) && (h < Height))
                {
                int i = h * LineWidth + 4 * w;
                I = GetIntensity (Bits[i], Bits[i+1], Bits[i+2]);
                IntensityCount[I]++;
                counter++;

                if (IntensityCount[I] == 1)
                    {
                    AverageColorR[I] = Bits[i];
                    AverageColorG[I] = Bits[i+1];
                    AverageColorB[I] = Bits[i+2];
                    }
                else
                    {
                    AverageColorR[I] += Bits[i];
                    AverageColorG[I] += Bits[i+1];
                    AverageColorB[I] += Bits[i+2];
                    }
                }
            }
        }

    int RandNumber, count, Index, ErrorCount = 0;
    uchar J;
    
    do
        {
        RandNumber = abs( (int)((rand() + 1) * ((double)counter / (RAND_MAX + 1))) );
        count = 0;
        Index = 0;
        
        do
            {
            count += IntensityCount[Index];
            Index++;
            }
        while (count < RandNumber && !m_cancel);

        J = Index - 1;
        ErrorCount++;
        }
    while ((IntensityCount[J] == 0) && (ErrorCount <= counter)  && !m_cancel);

    int R, G, B;

    if (ErrorCount >= counter)
        {
        R = AverageColorR[J] / counter;
        G = AverageColorG[J] / counter;
        B = AverageColorB[J] / counter;
        }
    else
        {
        R = AverageColorR[J] / IntensityCount[J];
        G = AverageColorG[J] / IntensityCount[J];
        B = AverageColorB[J] / IntensityCount[J];
        }
        
    return ( qRgb(R, G, B) );
}

/* Function to simple convolve a unique pixel with a determined radius                
 *                                                                                    
 * data             => The image data in RGBA mode.  
 * Width            => Width of image.                          
 * Height           => Height of image.                            
 * Radius           => kernel radius, e.g. rad=1, so array will be 3X3               
 * Kernel           => kernel array to apply.
 *                                                                                    
 * Theory           => I've worked hard here, but I think this is a very smart       
 *                     way to convolve an array, its very hard to explain how I reach    
 *                     this, but the trick here its to store the sum used by the       
 *                     previous pixel, so we sum with the other pixels that wasn't get 
 */
void ImageEffect_BlurFX::MakeConvolution (uint *data, int Width, int Height, int Radius, int Kernel[])
{
    if (Radius <= 0) return;
    
    register int i, j, n, h, w;
    
    int nSumR, nSumG, nSumB, nCount;
    int nKernelWidth = Radius * 2 + 1;
    int nStride = GetStride(Width);
    
    int LineWidth = Width * 4;                     
    if (LineWidth % 4) LineWidth += (4 - LineWidth % 4);
    
    int    BitCount = LineWidth * Height;
    uchar*  pInBits = (uchar*)data;
    uchar* pOutBits = new uchar[BitCount];
    uchar* pBlur    = new uchar[BitCount];
    
    // We need to copy our bits to blur bits
    
    memcpy (pBlur, pInBits, BitCount);     

    // We need to alloc a 2d array to help us to store the values
    
    int** arrMult = Alloc2DArray (nKernelWidth, 256);
    
    for (i = 0; i < nKernelWidth; i++)
        for (j = 0; j < 256; j++)
            arrMult[i][j] = j * Kernel[i];

    // We need to initialize all the loop and iterator variables
    
    nSumR = nSumG = nSumB = nCount = i = j = 0;

    // Now, we enter in the main loop
    
    for (h = 0; !m_cancel && (h < Height); h++, i += nStride)
        {
        for (w = 0; !m_cancel && (w < Width); w++, i += 4)
            {
            // first of all, we need to blur the horizontal lines
                
            for (n = -Radius; !m_cancel && (n <= Radius); n++)
               {
               // if is inside...
               if (IsInside (Width, Height, w + n, h))
                    {
                    // we points to the pixel
                    j = i + n * 4;
                    
                    // finally, we sum the pixels using a method similar to assigntables
                    nSumR += arrMult[n + Radius][pInBits[ j ]];
                    nSumG += arrMult[n + Radius][pInBits[j+1]];
                    nSumB += arrMult[n + Radius][pInBits[j+2]];
                    
                    // we need to add to the counter, the kernel value
                    nCount += Kernel[n + Radius];
                    }
                }
                
            if (nCount == 0) nCount = 1;                    
                
            // now, we return to blur bits the horizontal blur values
            pBlur[ i ] = LimitValues (nSumR / nCount);
            pBlur[i+1] = LimitValues (nSumG / nCount);
            pBlur[i+2] = LimitValues (nSumB / nCount);
            // ok, now we reinitialize the variables
            nSumR = nSumG = nSumB = nCount = 0;
            }
            
        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (((double)h * 50.0) / Height));
        kapp->processEvents();             
        }

    // getting the blur bits, we initialize position variables
    i = j = 0;

    // We enter in the second main loop
    for (w = 0; !m_cancel && (w < Width); w++, i = w * 4)
        {
        for (h = 0; !m_cancel && (h < Height); h++, i += LineWidth)
            {
            // first of all, we need to blur the vertical lines
            for (n = -Radius; !m_cancel && (n <= Radius); n++)
                {
                // if is inside...
                if (IsInside(Width, Height, w, h + n))
                    {
                    // we points to the pixel
                    j = i + n * LineWidth;
                      
                    // finally, we sum the pixels using a method similar to assigntables
                    nSumR += arrMult[n + Radius][pBlur[ j ]];
                    nSumG += arrMult[n + Radius][pBlur[j+1]];
                    nSumB += arrMult[n + Radius][pBlur[j+2]];
                    
                    // we need to add to the counter, the kernel value
                    nCount += Kernel[n + Radius];
                    }
                }
                
            if (nCount == 0) nCount = 1;                    
                
            // now, we return to bits the vertical blur values
            pOutBits[ i ] = LimitValues (nSumR / nCount);
            pOutBits[i+1] = LimitValues (nSumG / nCount);
            pOutBits[i+2] = LimitValues (nSumB / nCount);
                
            // ok, now we reinitialize the variables
            nSumR = nSumG = nSumB = nCount = 0;
            }
        
        // Update the progress bar in dialog.
        m_progressBar->setValue((int) (50.0 + ((double)w * 50.0) / Width));            
        kapp->processEvents();             
        }

    if (!m_cancel) 
       memcpy (data, pOutBits, BitCount);   
       
    // now, we must free memory
    Free2DArray (arrMult, nKernelWidth);
    delete [] pBlur;
    delete [] pOutBits;
}

}  // NameSpace DigikamBlurFXImagesPlugin

#include "imageeffect_blurfx.moc"
