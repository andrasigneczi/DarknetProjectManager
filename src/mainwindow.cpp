#include "mainwindow.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <iostream>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QToolBar>
#include "labelingcanvas.h"
#include <QAction>
#include <QMessageBox>
#include "util.h"
#include <fstream>
#include <QDir>
#include <QKeyEvent>

namespace {
    
class MyLineEdit : public QLineEdit {
public:
    using QLineEdit::QLineEdit;
    MyLineEdit(QListWidget* w) : mListWidget(w) {}
    
    void keyPressEvent(QKeyEvent* event) override {
        QLineEdit::keyPressEvent(event);
    }
    
    void keyReleaseEvent(QKeyEvent* event) override {
        if(event->modifiers() == Qt::NoModifier) {
            if(event->key() == Qt::Key_Down) {
                if(mListWidget->currentRow() + 1 < mListWidget->count()) {
                    mListWidget->setCurrentRow(mListWidget->currentRow() + 1);
                    return;
                }
            } else if(event->key() == Qt::Key_Up) {
                if(mListWidget->currentRow() - 1 >= 0) {
                    mListWidget->setCurrentRow(mListWidget->currentRow() - 1);
                    return;
                }
            }
        } else if(event->modifiers() == Qt::ControlModifier) {
            if(event->key() == Qt::Key_Home) {
                mListWidget->setCurrentRow(0);
                return;
            }
            else if(event->key() == Qt::Key_End) {
                if(mListWidget->count() > 0) {
                    mListWidget->setCurrentRow(mListWidget->count() - 1);
                    return;
                }
            }
        }
        QLineEdit::keyReleaseEvent(event);
    }
private:
    QListWidget* mListWidget;
};

}

MainWindow::MainWindow(Project& project)
: mProject(project){
    setGeometry(10, 50, 1000, 600);
    
    auto fileToolBar = addToolBar(tr("Box"));
    fileToolBar->addWidget(new QLabel("Box:")); // icon?
    QAction* newAction = fileToolBar->addAction("New"); // icon?
    QAction* deleteAction = fileToolBar->addAction("Delete"); // icon?
    QAction* duplicateAction = fileToolBar->addAction("Duplicate"); // icon?
    
    QGridLayout* grid = new QGridLayout;
    
    mListWidget = new QListWidget(this);
    
    const vector<string>& trainFileList = mProject.getTrainAndTestFileList();
    size_t index = 0;
    for( const string& x : trainFileList) {
        QListWidgetItem* item = new QListWidgetItem(tr(x.c_str()), mListWidget);
        item->setData(Qt::UserRole, QVariant::fromValue(index++));
        //std::cout << x << " - " << item->text().toUtf8().constData() << "\n";
        //std::cout.flush();
    }
    
    mCanvas = new LabelingCanvas(this);
    //mCanvas->setFixedSize(800, 550);
    
    QPushButton* saveBtn = new QPushButton("Save All");
    mLabelEdit = new MyLineEdit(mListWidget);
    QVBoxLayout* toolLayout = new QVBoxLayout;

    QGridLayout* lineLayout = new QGridLayout;
    lineLayout->addWidget(new QLabel("Label:"), 0, 0);
    lineLayout->addWidget(mLabelEdit, 0, 1);
    lineLayout->addWidget(new QLabel("Position:"), 1, 0);
    lineLayout->addWidget(new QLabel("Size:"), 2, 0);
    lineLayout->addWidget(mPosition = new QLabel(""), 1, 1);
    lineLayout->addWidget(mSize = new QLabel(""), 2, 1);

    toolLayout->addLayout(lineLayout);
    toolLayout->addWidget(saveBtn);
    
    
    QGridLayout* underCanvasLayout = new QGridLayout;
    underCanvasLayout->addWidget(new QLabel("Label string:"), 0, 0);
    mLabelStrUnderMousePointer = new QLabel;
    underCanvasLayout->addWidget(mLabelStrUnderMousePointer, 0, 1, Qt::AlignLeft);
    
    grid->addWidget(mCanvas, 0, 0);
    grid->addLayout(underCanvasLayout, 1, 0);
    grid->addWidget(mListWidget, 0, 1);
    grid->addLayout(toolLayout, 1, 1);
    
    QWidget* widget = new QWidget;
    setCentralWidget(widget);
    widget->setLayout(grid);
    
    grid->setColumnStretch(0, 1000);
    underCanvasLayout->setColumnStretch(1, 1000);
    
    QObject::connect(mListWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
        this, SLOT(listCurrentItemChanged(QListWidgetItem*, QListWidgetItem*)));

    QObject::connect(mCanvas, SIGNAL(boundingboxSelection(string, string, string)),
        this, SLOT(boundingboxSelection(string, string, string)));

    QObject::connect(saveBtn, SIGNAL(pressed()),
        this, SLOT(savePressed()));
        
    QObject::connect(mLabelEdit, SIGNAL(textChanged(const QString &)),
        mCanvas, SLOT(labelChanged(const QString &)));

    QObject::connect(newAction, SIGNAL(triggered(bool)),
        this, SLOT(newButtonTriggered(bool)));

    QObject::connect(deleteAction, SIGNAL(triggered(bool)),
        this, SLOT(deleteButtonTriggered(bool)));

    QObject::connect(duplicateAction, SIGNAL(triggered(bool)),
        this, SLOT(duplicateButtonTriggered(bool)));

    QObject::connect(mCanvas, SIGNAL(updateUnderMouseLabels(string)),
        this, SLOT(updateUnderMouseLabels(string)));
}

void MainWindow::saveRecAndLabelData(size_t index) {
    vector<LabelingCanvas::RectAndLabelData>& rectAndLabelData = mCanvas->getRectAndLabelData();
    vector<Project::RectAndLabel> updatedLabelsAndRects;
    for(LabelingCanvas::RectAndLabelData& data: rectAndLabelData) {
        if(data.mRectOrLabelStatus == LabelingCanvas::DELETED) continue;
        updatedLabelsAndRects.push_back(data.mOriginalRectAndLabel);
        if(data.mRectOrLabelStatus != LabelingCanvas::UNTAUCHED) {
            
            Project::NormRect normRect = LabelingCanvas::calcNormalizedRec(data.mConvertedRect.x(), data.mConvertedRect.y(),
                    data.mConvertedRect.width(), data.mConvertedRect.height(),
                    mCanvas->pixmapWidth(), mCanvas->pixmapHeight());

            data.mOriginalRectAndLabel.mNormRect = normRect;
            updatedLabelsAndRects.back().mNormRect = normRect;
        }
    }
    mProject.updateLabelAndRect(index, updatedLabelsAndRects);
    mCanvas->saved();
}

void MainWindow::listCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous) {
    if(mCanvas->isModified()) {
        QMessageBox msgBox;
        msgBox.setText("The canvas was modified!");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
        if(ret == QMessageBox::Yes) {
            size_t index = previous->data(Qt::UserRole).value<size_t>();
            saveRecAndLabelData(index);
        }
    }
    // update pixmap
    mCanvas->loadPixmap(current->text());
    
    size_t index = current->data(Qt::UserRole).value<size_t>();
    // update bounding rect list
    mCanvas->setLabelsAndRects(mProject.getLabelsAndRects(index));
    boundingboxSelection("", "", "");
}

void MainWindow::boundingboxSelection(string label, string position, string size) {
    QObject::disconnect(mLabelEdit, SIGNAL(textChanged(const QString &)),
        mCanvas, SLOT(labelChanged(const QString &)));

    mLabelEdit->setText(label.c_str());
    mPosition->setText(position.c_str());
    mSize->setText(size.c_str());
    mLabelEdit->setFocus();
    
    QObject::connect(mLabelEdit, SIGNAL(textChanged(const QString &)),
        mCanvas, SLOT(labelChanged(const QString &)));
}

void MainWindow::savePressed() {
    auto* current = mListWidget->currentItem();
    if(current && mCanvas->isModified()) {
        size_t index = current->data(Qt::UserRole).value<size_t>();
        // update bounding rect list
        saveRecAndLabelData(index);
    }
}

void MainWindow::newButtonTriggered(bool checked /*= false*/) {
    UNUSED(checked);
    mCanvas->addNewActiveBox();
}

void MainWindow::deleteButtonTriggered(bool checked /*= false*/) {
    UNUSED(checked);
    QMessageBox msgBox;
    msgBox.setText("The active box will be deleted. Are you sure?");
    //msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Yes) {
        mCanvas->deleteActiveBox();
    }
}

void MainWindow::duplicateButtonTriggered(bool checked /*= false*/) {
    UNUSED(checked);
    mCanvas->duplicateActiveBox();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    cout << "MainWindow::resizeEvent\n";
    QWidget::resizeEvent(event);
    auto* current = mListWidget->currentItem();
    if(current) {
        size_t index = current->data(Qt::UserRole).value<size_t>();
        // update bounding rect list
        mCanvas->setLabelsAndRects(mProject.getLabelsAndRects(index));
        boundingboxSelection("", "", "");
    }
}

void MainWindow::updateUnderMouseLabels(string label) {
    if(mLabelStrUnderMousePointer->text() != label.c_str()) {
        mLabelStrUnderMousePointer->setText(label.c_str());
    }
}
