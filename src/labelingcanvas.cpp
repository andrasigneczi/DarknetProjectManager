#include "labelingcanvas.h"
#include <QMouseEvent>
#include <iostream>
#include "util.h"
#include <math.h>
#include <fstream>
#include <QDir>

LabelingCanvas::LabelingCanvas(QWidget *parent)
: QWidget(parent)
, mouseX(0)
, mouseY(0)
, mSelectedRect(-1)
, mFocusedRect(-1)
, mousePressed(false)
, mRectDragged(false)
, mousePressedX(0)
, mousePressedY(0)
, mHelperBox(NONE)
, mZoom(1.)
, mPixmapPos(0,0)
, mImageDragged(false)
, mTransferRateW(1.)
, mTransferRateH(1.)
, mZoomBox(0, 0, 27, 35)
, mGrayBoxBackground(true)
, mFocusedZoomOn(false)
, mBoxesHidden(false)
{
    setMouseTracking(true);
}

void LabelingCanvas::paintEvent(QPaintEvent* pe) {
    QPainter p(this);
    p.fillRect(0, 0, width(), height(), QBrush(Qt::GlobalColor::darkGray));
    
    if(!mOriginalImage.isNull()) {
        double sizeRate = (double)mOriginalImage.width() / mOriginalImage.height();
        QRectF desc;
        QRectF src;
        if(mFitToWidth) {
            desc = QRectF(0, 0, width(), width() / sizeRate);
            src = QRectF(mPixmapPos.x(), mPixmapPos.y(), mOriginalImage.width() * mZoom, mOriginalImage.height() * mZoom);

            // Let's try to extend to the height to the full canvas
            if(mZoom < 1. && desc.height() < height()) {
                double corrSizeRate = src.height() / desc.height();
                desc.setHeight(height());
                src.setHeight(height() * corrSizeRate);
            }
        } else {
            desc = QRectF(0, 0, height() * sizeRate, height());
            src = QRectF(mPixmapPos.x(), mPixmapPos.y(), mOriginalImage.width() * mZoom, mOriginalImage.height() * mZoom);

            // Let's try to extend the width to the full canvas
            if(mZoom < 1. && desc.width() < width()) {
                double corrSizeRate = src.width() / desc.width();
                desc.setWidth(width());
                src.setWidth(width() * corrSizeRate);
            }
        }
        p.drawPixmap(desc, mOriginalImage, src);
        drawRectsAndLabels(p);
    }
    //p.fillRect(0,0,150,50, QBrush(QColor("#0000ff")));
    //p.setFont(QFont("times",18));
    //p.setPen(QPen(QColor("#ff0000")));
    //p.drawText(QRect(0,0,500,30), Qt::AlignLeft, (std::to_string(mouseX) + " " + std::to_string(mouseY)).c_str());
    drawZoomIcons(p);
    QWidget::paintEvent(pe);
}

void LabelingCanvas::drawZoomIcons(QPainter& p) {
    p.setFont(QFont("times",24));
    p.setPen(QPen(QColor("#eeeeee")));
    p.fillRect(width()-33, 5, mZoomBox.width(), 135, QBrush(QColor("#000000")));
    p.drawText(QRect(width()-29, 5, mZoomBox.width(), mZoomBox.height()), Qt::AlignLeft, "+");
    p.drawText(QRect(width()-25, 35, mZoomBox.width(), mZoomBox.height()), Qt::AlignLeft, "-");
    p.setFont(QFont("times",16));
    p.drawText(QRect(width()-26, 75, mZoomBox.width(), mZoomBox.height()), Qt::AlignLeft, "R");
    if(mFocusedZoomOn) {
        p.setPen(QPen(Qt::GlobalColor::green));
    }
    p.drawText(QRect(width()-26, 110, mZoomBox.width(), mZoomBox.height()), Qt::AlignLeft, "F");
}

void LabelingCanvas::drawRectsAndLabels(QPainter& p) {
    //p.setCompositionMode(QPainter::CompositionMode_Difference);
    p.setFont(QFont("times",12, QFont::Bold));
    QColor orange;
    orange.setNamedColor("orange");
    QBrush brYellow(Qt::GlobalColor::yellow, Qt::SolidPattern);
    QBrush brOrange(orange, Qt::SolidPattern);
    QBrush brGreen(Qt::GlobalColor::green, Qt::SolidPattern);
    QBrush brRed(Qt::GlobalColor::red, Qt::SolidPattern);
    QBrush brBlue(Qt::GlobalColor::blue, Qt::SolidPattern);
    QBrush brWhite(Qt::GlobalColor::white, Qt::SolidPattern);
    QBrush brBlack(Qt::GlobalColor::black, Qt::SolidPattern);
    const int tickness = 2;
    
    for(size_t i = 0; i < mRectAndLabelData.size(); ++i) {
        if(rectOrLabelStatus(i) == DELETED) continue;
        if((int)i == mSelectedRect) {
            p.setPen(QPen(brGreen, tickness, Qt::SolidLine));
        }
        
        if((int)i == mFocusedRect) {
            p.setPen(QPen(brRed, tickness, Qt::SolidLine));
        }
        
        if((int)i != mFocusedRect && (int)i != mSelectedRect) {
            if(mGrayBoxBackground) {
                p.setPen(QPen(brWhite, tickness, Qt::SolidLine));
            } else {
                p.setPen(QPen(brBlack, tickness, Qt::SolidLine));
            }
        }
        
        QRectF r = convertedRect(i);
        // the mOriginalImage or part of that is scaled to the canvas, that rate must be calcualated first
        r = transfer(r);
        
        if(!mBoxesHidden || (int)i == mFocusedRect || (int)i == mSelectedRect) {
            p.setOpacity(0.3);
            if(mGrayBoxBackground) {
                p.fillRect(r, Qt::GlobalColor::black);
            } else {
                p.fillRect(r, Qt::GlobalColor::white);
            }
            p.setOpacity(1.);
            p.drawRect(r);
            p.drawText(r.left() + 10, r.top(), 500, 40, Qt::AlignLeft, tr(originalRectAndLabel(i).mLabel.c_str()));
        }
        
        if((int)i == mSelectedRect) {
            // resizing helper boxes
            if(mRectDragged) {
                p.setPen(QPen(brBlue, tickness, Qt::DotLine));
                drawHelperBoxes(p, transfer(mMovingRect));
            } else {
                drawHelperBoxes(p, r);
            }
        }
    }
    
    if(mRectDragged) {
        p.setPen(QPen(brBlue, tickness, Qt::DotLine));
        p.drawRect(transfer(mMovingRect));
    }
}

void LabelingCanvas::drawHelperBoxes(QPainter& p, QRectF rect) {
    int tickness = 20;
    mHelperBox = NONE;
    
    // Left helper
    if(rect.left() < mouseX && rect.left() + tickness > mouseX &&
    rect.top() + tickness < mouseY && rect.bottom() - tickness > mouseY) {
        p.drawRect(rect.left(), rect.top() + tickness, tickness, 
        rect.height() - 2 * tickness);
        mHelperBox = LEFT;
    }
    
    // Right helper
    if(rect.right() > mouseX && rect.right() - tickness < mouseX &&
    rect.top() + tickness < mouseY && rect.bottom() - tickness > mouseY) {
        p.drawRect(rect.right() - tickness, rect.top() + tickness, tickness, 
        rect.height() - 2 * tickness);
        mHelperBox = RIGHT;
    }
    
    // Top helper
    if(rect.left() + tickness < mouseX && rect.right() - tickness > mouseX &&
    rect.top() < mouseY && rect.top() + tickness > mouseY) {
        p.drawRect(rect.left() + tickness, rect.top(), rect.width() - 2 * tickness, 
        tickness);
        mHelperBox = TOP;
    }

    // Bottom helper
    if(rect.left() + tickness < mouseX && rect.right() - tickness > mouseX &&
    rect.bottom() > mouseY && rect.bottom() - tickness < mouseY) {
        p.drawRect(rect.left() + tickness, rect.bottom() - tickness, 
        rect.width() - 2 * tickness, 
        tickness);
        mHelperBox = BOTTOM;
    }
    
    // top left corner
    if(rect.left() < mouseX && rect.left() + tickness > mouseX &&
    rect.top() + tickness > mouseY && rect.top() < mouseY) {
        p.drawRect(rect.left(), rect.top(), tickness, tickness);
        mHelperBox = TOP_LEFT;
    }
    // top right corner
    if(rect.right() > mouseX && rect.right() - tickness < mouseX &&
    rect.top() + tickness > mouseY && rect.top() < mouseY) {
        p.drawRect(rect.right() - tickness, rect.top(), tickness, tickness);
        mHelperBox = TOP_RIGHT;
    }
    // bottom left corner
    if(rect.left() < mouseX && rect.left() + tickness > mouseX &&
    rect.bottom() - tickness < mouseY && rect.bottom() > mouseY) {
        p.drawRect(rect.left(), rect.bottom() - tickness, tickness, tickness);
        mHelperBox = BOTTOM_LEFT;
    }
    // bottom right corner
    if(rect.right() - tickness < mouseX && rect.right() > mouseX &&
    rect.bottom() - tickness < mouseY && rect.bottom() > mouseY) {
        p.drawRect(rect.right() - tickness, rect.bottom() - tickness, tickness, tickness);
        mHelperBox = BOTTOM_RIGHT;
    }
}

void LabelingCanvas::updateMovingRectDuringResize() {
    double horizontalDiff = (double)(mouseX - mousePressedX) * mZoom;
    double verticalDiff   = (double)(mouseY - mousePressedY) * mZoom;
    const QRectF& cR = convertedRect(mSelectedRect);
    switch(mHelperBox) {
        case NONE:
        break;
        case TOP:
            mMovingRect = QRectF(cR.left(),
            cR.top() + verticalDiff,
            cR.width(),
            cR.height() - verticalDiff);
        break;
        case LEFT:
            mMovingRect = QRectF(cR.left() + horizontalDiff,
            cR.top(),
            cR.width() - horizontalDiff,
            cR.height());
        break;
        case RIGHT: 
            mMovingRect = QRectF(cR.left(),
            cR.top(),
            cR.width() + horizontalDiff,
            cR.height());
        break;
        case BOTTOM: 
            mMovingRect = QRectF(cR.left(),
            cR.top(),
            cR.width(),
            cR.height() + verticalDiff);
        break;
        case TOP_LEFT: 
            mMovingRect = QRectF(cR.left() + horizontalDiff,
            cR.top() + verticalDiff,
            cR.width() - horizontalDiff,
            cR.height() - verticalDiff);
        break;
        case TOP_RIGHT: 
            mMovingRect = QRectF(cR.left(),
            cR.top() + verticalDiff,
            cR.width() + horizontalDiff,
            cR.height() - verticalDiff);
        break;
        case BOTTOM_LEFT:
            mMovingRect = QRectF(cR.left() + horizontalDiff,
            cR.top(),
            cR.width() - horizontalDiff,
            cR.height() + verticalDiff);
        break;
        case BOTTOM_RIGHT: 
            mMovingRect = QRectF(cR.left(),
            cR.top(),
            cR.width() + horizontalDiff,
            cR.height() + verticalDiff);
        break;
    }
    
    if(mMovingRect.width() < 4 ) mMovingRect.setWidth(4);
    if(mMovingRect.height() < 4 ) mMovingRect.setHeight(4);
}

void LabelingCanvas::mouseMoveEvent(QMouseEvent*me) {
    mouseX = me->x();
    mouseY = me->y();

    // calculating the pointed pixel position on the image
    emit mousePositionChange(round(mouseX / mTransferRateW * mZoom), round(mouseY / mTransferRateH * mZoom));

    if(mousePressed) {
        setCursor(Qt::ClosedHandCursor);
        if(mSelectedRect == -1) {
            // drag and move the image, like google map
            if(!mImageDragged) {
                mPixmapDraggedPos = mPixmapPos;
            }
            mImageDragged = true;
            mPixmapPos.setX(round(mPixmapDraggedPos.x() - (mouseX - mousePressedX) / mTransferRateW * mZoom));
            mPixmapPos.setY(round(mPixmapDraggedPos.y() - (mouseY - mousePressedY) / mTransferRateH * mZoom));
        } else {
            // updating the position of the rect
            if(mHelperBox == NONE || me->modifiers() == Qt::ControlModifier) {
                mMovingRect = QRectF(convertedRect(mSelectedRect).left() + (mouseX - mousePressedX) * mZoom,
                convertedRect(mSelectedRect).top() + (mouseY - mousePressedY) * mZoom,
                convertedRect(mSelectedRect).width(),
                convertedRect(mSelectedRect).height());
            } else {
                updateMovingRectDuringResize();
            }
            mRectDragged = true;
        }
    } else {
        if(mSelectedRect != -1) {
            QRectF r = convertedRect(mSelectedRect);
            r = transfer(r);
            
            if(!r.contains(mouseX, mouseY)) {
                mSelectedRect = -1;
            }
        }
        
        if(mSelectedRect == -1) {
            for(size_t i = 0; i < mRectAndLabelData.size(); ++i) {
                if(rectOrLabelStatus(i) == DELETED) continue;
                
                QRectF r = convertedRect(i);
                r = transfer(r);
                
                if(r.contains(mouseX, mouseY)) {
                    mSelectedRect = i;
                    break;
                }
            }
        }
        if(mSelectedRect != -1) {
            emit updateUnderMouseLabels(originalRectAndLabel(mSelectedRect).mLabel);
        }
    }
    repaint();
    QWidget::mouseMoveEvent(me);
}

void LabelingCanvas::mousePressEvent(QMouseEvent*me) {
    if(me->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(me);
        return;
    }
    
    mousePressed = true;
    mousePressedX = me->x();
    mousePressedY = me->y();
    setCursor(Qt::OpenHandCursor);

    if(QRect(width()-30, 5, mZoomBox.width(), mZoomBox.height()).contains(mousePressedX, mousePressedY) && mZoom > 0.1 + 1e-6) {
        mFocusedRect = -1;
        emit boundingboxSelection("", "", "");
        centeredZoom(mZoom - 0.1);
    } else if(QRect(width()-25, 35, mZoomBox.width(), mZoomBox.height()).contains(mousePressedX, mousePressedY)) {
        mFocusedRect = -1;
        emit boundingboxSelection("", "", "");
        centeredZoom(mZoom + 0.1);
    } else if(QRect(width()-26, 75, mZoomBox.width(), mZoomBox.height()).contains(mousePressedX, mousePressedY)) {
        mFocusedRect = -1;
        emit boundingboxSelection("", "", "");
        mPixmapPos.setX(0);
        mPixmapPos.setY(0);
        mZoom = 1.;
    } else if(QRect(width()-26, 105, mZoomBox.width(), mZoomBox.height()).contains(mousePressedX, mousePressedY)) {
        mFocusedZoomOn = !mFocusedZoomOn;
    } else if(mFocusedZoomOn) {
        mFocusedZoomOn = false;
        mFocusedRect = -1;
        emit boundingboxSelection("", "", "");
        
        // calculate the clicked position on the bitmap
        double pixPosX = mPixmapPos.x() + (double)mousePressedX / mTransferRateW * mZoom;
        double pixPosY = mPixmapPos.y() + (double)mousePressedY / mTransferRateH * mZoom;
        
        // calculate the center of the canvas
        double pixPosXC = (double)width() / 2. / mTransferRateW * mZoom;
        double pixPosYC = (double)height() / 2. / mTransferRateH * mZoom;
        
        // move the clicked position to the middle of the canvas
        mPixmapPos.setX(pixPosX - pixPosXC);
        mPixmapPos.setY(pixPosY - pixPosYC);
        
        // zoom the image and correct the position
        const double newZoom = 0.2;
        if(mZoom > newZoom) {
            centeredZoom(newZoom);
        }
    } else if(mSelectedRect != -1) {
        updateBoundingBoxData(mSelectedRect, "");
        mFocusedRect = mSelectedRect;
    }
    repaint();
    QWidget::mousePressEvent(me);
}

void LabelingCanvas::centeredZoom(double newZoom) {
    double s1 = width() / 2. / mTransferRateW * mZoom;
    double s2 = width() / 2. / mTransferRateW * newZoom;
    mPixmapPos.setX(mPixmapPos.x() - (s2 - s1));
    s1 = height() / 2. / mTransferRateH * mZoom;
    s2 = height() / 2. / mTransferRateH * newZoom;
    mPixmapPos.setY(mPixmapPos.y() - (s2 - s1));
    mZoom = newZoom;
}

void LabelingCanvas::mouseReleaseEvent(QMouseEvent*me) {
    if(me->button() == Qt::RightButton) {
        nextSelectedRectUnderMouse();
        QWidget::mouseReleaseEvent(me);
        repaint();
        return;
    }
    setCursor(Qt::ArrowCursor);
    mousePressed = false;
    if(mSelectedRect != -1 && mRectDragged) {
        convertedRect(mSelectedRect) = mMovingRect;
        mLastMovingRect = mMovingRect;
        rectOrLabelStatus(mSelectedRect) = MODIFIED;
        mRectDragged = false;
        repaint();
    }
    mImageDragged = false;
    QWidget::mouseReleaseEvent(me);
}

void LabelingCanvas::wheelEvent(QWheelEvent *event) {
    // Let's change the selected rect in case of overlapping
    std::cout << "LabelingCanvas::wheelEvent\n";
    nextSelectedRectUnderMouse();
    repaint();
    QWidget::wheelEvent(event);
}

void LabelingCanvas::nextSelectedRectUnderMouse() {
    // Let's change the selected rect in case of overlapping
    for(size_t i = 0; i < mRectAndLabelData.size(); ++i) {
        size_t pos = i;
        if(mSelectedRect != -1) {
            pos = (pos + mSelectedRect + 1) % mRectAndLabelData.size();
        }
        if(rectOrLabelStatus(pos) == DELETED) continue;
        
        QRectF r = convertedRect(pos);
        r = transfer(r);
        
        if(r.contains(mouseX, mouseY)) {
            mSelectedRect = pos;
            break;
        }
    }
    if(mSelectedRect != -1) {
        emit updateUnderMouseLabels(originalRectAndLabel(mSelectedRect).mLabel);
    }
}

// it's called when another image was clicked on the mainwindow
void LabelingCanvas::setLabelsAndRects(const vector<Project::RectAndLabel>& rectsAndLabels) {
    mFocusedRect = -1;
    mSelectedRect = -1;
    mousePressed = false;
    mRectDragged = false;
    mRectAndLabelData = vector<RectAndLabelData>(rectsAndLabels.size());
    // convert rects
    calcBoxZoomRate();
    for(size_t i = 0; i < rectsAndLabels.size(); ++i) {
        originalRectAndLabel(i) = rectsAndLabels[i];
        const Project::NormRect& nr = rectsAndLabels[i].mNormRect;
        QRectF r = calcBoundingRec(nr.mCenterX, nr.mCenterY, nr.mWidth, nr.mHeight, imageWidth(), imageHeight());
        r = QRectF(r.left() * mTransferRateW, r.top() * mTransferRateH, r.width() * mTransferRateW, r.height() * mTransferRateH);
        convertedRect(i) = r;
    }
    
    repaint();
}

void LabelingCanvas::calcBoxZoomRate() {
    double sizeRate = (double)mOriginalImage.width() / mOriginalImage.height();
    double lWidth, lHeight;
    if(mFitToWidth) {
        lWidth = width();
        lHeight = width() / sizeRate;
    } else {
        lWidth = height() * sizeRate;
        lHeight = height();
    }

    // let's convert the rect, because it is aligned to other image size
    mTransferRateW = lWidth / imageWidth();
    mTransferRateH = lHeight / imageHeight();
}

double LabelingCanvas::pixmapWidth() {
    double sizeRate = (double)mOriginalImage.width() / mOriginalImage.height();
    double lWidth;
    if(mFitToWidth) {
        lWidth = width();
    } else {
        lWidth = height() * sizeRate;
    }
    return lWidth;
}

double LabelingCanvas::pixmapHeight() {
    double sizeRate = (double)mOriginalImage.width() / mOriginalImage.height();
    double lHeight;
    if(mFitToWidth) {
        lHeight = width() / sizeRate;
    } else {
        lHeight = height();
    }
    return lHeight;
}

// The main window send this text, when the content of the linetext was changed
void LabelingCanvas::labelChanged(QString const& text) {
    if(mFocusedRect != -1) {
        mLastLabel = text.toUtf8().constData();
        originalRectAndLabel(mFocusedRect).mLabel = text.toUtf8().constData();
        rectOrLabelStatus(mFocusedRect) = MODIFIED;;
        repaint();
    }
}

void LabelingCanvas::deleteActiveBox() {
    if(mFocusedRect != -1) {
        rectOrLabelStatus(mFocusedRect) = DELETED;
        mFocusedRect = -1;
        repaint();
    }
}

void LabelingCanvas::addNewActiveBox() {
    RectAndLabelData newItem;
    newItem.mOriginalRectAndLabel.mLabel = mLastLabel;
    newItem.mConvertedRect = transferBack(QRect(10, 10, 200, 200));
    if(!mLastMovingRect.isNull()) {
        newItem.mConvertedRect.setWidth(mLastMovingRect.width());
        newItem.mConvertedRect.setHeight(mLastMovingRect.height());
    }
    newItem.mRectOrLabelStatus = NEW;
    mRectAndLabelData.push_back(newItem);

    mFocusedRect = mRectAndLabelData.size() - 1;
    updateBoundingBoxData(mFocusedRect, newItem.mOriginalRectAndLabel.mLabel);
    repaint();
}

void LabelingCanvas::duplicateActiveBox() {
    RectAndLabelData newItem;
    newItem.mOriginalRectAndLabel.mLabel = mLastLabel;
    newItem.mConvertedRect = transferBack(QRect(10, 10, 200, 200));
    if(mFocusedRect != -1) {
        newItem.mConvertedRect.setWidth(convertedRect(mFocusedRect).width());
        newItem.mConvertedRect.setHeight(convertedRect(mFocusedRect).height());
        newItem.mOriginalRectAndLabel.mLabel = originalRectAndLabel(mFocusedRect).mLabel;
    }
    newItem.mRectOrLabelStatus = NEW;
    mRectAndLabelData.push_back(newItem);

    mFocusedRect = mRectAndLabelData.size() - 1;
    updateBoundingBoxData(mFocusedRect, newItem.mOriginalRectAndLabel.mLabel);
    repaint();
}

bool LabelingCanvas::isModified() {
    for(size_t i = 0; i < mRectAndLabelData.size(); ++i) {
        if(rectOrLabelStatus(i) != UNTAUCHED)
            return true;
    }
    return false;
}

void LabelingCanvas::saved() {
    for(size_t i = 0; i < mRectAndLabelData.size(); ++i) {
        if(rectOrLabelStatus(i) == DELETED) {
            mRectAndLabelData.erase(mRectAndLabelData.begin() + i);
        } else {
            rectOrLabelStatus(i) = UNTAUCHED;
        }
    }
}

void LabelingCanvas::loadPixmap(QString str) {
    mOriginalImage = QPixmap(str);
    if(mOriginalImage.isNull()) {
        throw str + "image does not exist";
    }
    cout << "Pixmap " << str.toUtf8().constData() << " has been loaded, size: " << mOriginalImage.width() << "x" << mOriginalImage.height() << endl;
    if((double)mOriginalImage.width() / mOriginalImage.height() > (double)width() / height()) {
        mFitToWidth = true;
    } else {
        mFitToWidth = false;
    }
    repaint();
}

QRectF LabelingCanvas::transfer(QRectF r) {
    return QRectF(r.x() / mZoom - mPixmapPos.x() * mTransferRateW / mZoom,
                  r.y() / mZoom - mPixmapPos.y() * mTransferRateH / mZoom,
                  r.width() / mZoom, r.height() / mZoom);
}

QRectF LabelingCanvas::transferBack(QRectF r) {
    return QRectF((r.x() + mPixmapPos.x() * mTransferRateW / mZoom)* mZoom,
                  (r.y() + mPixmapPos.y() * mTransferRateH / mZoom) * mZoom,
                  r.width() * mZoom, r.height() * mZoom);
}

Project::NormRect LabelingCanvas::calcNormalizedRec(double boundingLeft, double boundingTop, double boundingWidth, double boundingHeight,
                                          double imageWidth, double imageHeight) {
    Project::NormRect normRect;
    normRect.mCenterX = (boundingLeft + boundingWidth / 2.) / imageWidth;
    normRect.mCenterY = (boundingTop + boundingHeight / 2.) / imageHeight;
    normRect.mWidth   = boundingWidth / imageWidth;
    normRect.mHeight  = boundingHeight / imageHeight;
    return normRect;
}

QRectF LabelingCanvas::calcBoundingRec(double centerX, double centerY, double normWidth, double normHeight, 
                                     double imageWidth, double imageHeight) {
    double boundingWidth  = normWidth * imageWidth;
    double boundingHeight = normHeight * imageHeight;
    double boundingLeft   = centerX * imageWidth - boundingWidth / 2.;
    double boundingTop    = centerY * imageHeight - boundingHeight / 2.;
    return {boundingLeft, boundingTop, boundingWidth, boundingHeight};
}

QRectF LabelingCanvas::calcBoundingRec(Project::NormRect normRect, double imageWidth, double imageHeight) {
    return calcBoundingRec(normRect.mCenterX, normRect.mCenterY, normRect.mWidth, normRect.mHeight,
            imageWidth, imageHeight);
}

void LabelingCanvas::updateBoundingBoxData(int index, string label) {
    QRectF r = convertedRect(index);
    std::string position = std::to_string((int)round(r.x() / mTransferRateW)) + ";" + std::to_string((int)round(r.y() / mTransferRateH));
    std::string size = std::to_string((int)round(r.width() / mTransferRateW)) + "x" + std::to_string((int)round(r.height() / mTransferRateH));
    if(label.length() > 0) {
        emit boundingboxSelection(label, position, size);
    } else {
        emit boundingboxSelection(originalRectAndLabel(index).mLabel, position, size);
    }
}

void LabelingCanvas::switchGrayBoxBackground() {
    mGrayBoxBackground = !mGrayBoxBackground;
    repaint();
}

void LabelingCanvas::hideBoxes(bool hide) {
    mBoxesHidden = hide;
    repaint();
}
