#ifndef __LABELINGCANVAS_H__
#define __LABELINGCANVAS_H__

#include <QWidget>
#include <QPixmap>
#include <QPainter>
#include "project.h"

class Q_DECL_EXPORT LabelingCanvas : public QWidget {
    Q_OBJECT
public:
    enum BoxStatus {
        UNTAUCHED,
        MODIFIED,
        DELETED,
        NEW
    };

    enum ActivatedResizingHelperBox {
        NONE,
        TOP_LEFT,
        TOP,
        TOP_RIGHT,
        RIGHT,
        BOTTOM_RIGHT,
        BOTTOM,
        BOTTOM_LEFT,
        LEFT
    };

    struct RectAndLabelData {
        RectAndLabelData(){ mRectOrLabelStatus = UNTAUCHED; }

        Project::RectAndLabel mOriginalRectAndLabel;
        QRectF    mConvertedRect;
        BoxStatus mRectOrLabelStatus;
    };

    explicit LabelingCanvas(QWidget *parent = 0);

    void loadPixmap(QString str);
    // it's called when another image was clicked on the mainwindow
    void setLabelsAndRects(const vector<Project::RectAndLabel>& rectsAndLabels);

    void paintEvent(QPaintEvent* pe) override;
    void mouseMoveEvent(QMouseEvent*me) override;
    void mousePressEvent(QMouseEvent*me) override;
    void mouseReleaseEvent(QMouseEvent*me) override;
    void wheelEvent(QWheelEvent *event) override;

    void deleteActiveBox();
    void addNewActiveBox();
    void duplicateActiveBox();
    bool isModified();
    vector<RectAndLabelData>& getRectAndLabelData() { return mRectAndLabelData; }
    void saved();
    
    double pixmapWidth();
    double pixmapHeight();
    int imageWidth() { return mOriginalImage.width(); }
    int imageHeight() { return mOriginalImage.height(); }
    void switchGrayBoxBackground();
    
    // rect calculation from or to normrect
    static Project::NormRect calcNormalizedRec(double boundingLeft, double boundingTop, double boundingWidth, double boundingHeight, double imageWidth, double imageHeight);
    static QRectF calcBoundingRec(double centerX, double centerY, double normWidth, double normHeight, double imageWidth, double imageHeight);
    static QRectF calcBoundingRec(Project::NormRect normRect, double imageWidth, double imageHeight);
    
signals:
    void boundingboxSelection(string label, string position, string size);
    void updateUnderMouseLabels(string label);

public slots:
    void labelChanged(const QString &text);

private:
    void drawRectsAndLabels(QPainter& p);
    void drawHelperBoxes(QPainter& p, QRectF rect);
    void updateMovingRectDuringResize();
    void updateBoundingBoxData(int index, string label);
    void nextSelectedRectUnderMouse();
    
    // helper function for mRectAndLabelData
    Project::RectAndLabel& originalRectAndLabel(size_t index) { return mRectAndLabelData[index].mOriginalRectAndLabel; }
    QRectF& convertedRect(size_t index) { return mRectAndLabelData[index].mConvertedRect; }
    BoxStatus& rectOrLabelStatus(size_t index) { return mRectAndLabelData[index].mRectOrLabelStatus; }

    void drawZoomIcons(QPainter& p);
    void calcBoxZoomRate();
    void centeredZoom(double newZoom);
    
    // convert the rect from the original image coordinate system to the
    // screen/canvas coordinate system
    QRectF transfer(QRectF r);
    
    // convert the rect from screen/canvas coordinate system to the
    // the original image coordinate system
    QRectF transferBack(QRectF r);
    
    //QPixmap mImage;
    QPixmap mOriginalImage;
    int mouseX; // mouseMoveEvent set it
    int mouseY; // mouseMoveEvent set it
    int mSelectedRect; // index in the vectors, the mouse is above this rect
    int mFocusedRect; // index in the vectors, this rect was clicked
    bool mousePressed;
    bool mRectDragged;
    int mousePressedX;
    int mousePressedY;
    QRectF mMovingRect;
    ActivatedResizingHelperBox mHelperBox;

    vector<RectAndLabelData> mRectAndLabelData;
    double mZoom;
    
    // top-right corner of the sub-image which is copied to the canvas 0,0 position
    QPoint mPixmapPos;
    
    bool mFitToWidth;
    bool mImageDragged;
    QPoint mPixmapDraggedPos;
    // for converting the zoomed image system to the canvas system
    double mTransferRateW;
    double mTransferRateH;
    string mLastLabel;
    QRectF mLastMovingRect;
    const QRect mZoomBox;
    bool mGrayBoxBackground;
    bool mFocusedZoomOn;
};

#endif // __LABELINGCANVAS_H__
