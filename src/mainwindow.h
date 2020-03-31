#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <QMainWindow>
#include <QListWidgetItem>
#include "project.h"

class LabelingCanvas;
class QLabel;
class QPushButton;

class Q_DECL_EXPORT MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(Project& project);
    
public slots:
    // current item changed in the train file list
    void listCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);

    // boundingboxSelection triggered by the canvas, this slot updates the label
    // in the QLineEdit
    void boundingboxSelection(string label, string position, string size);
    void savePressed();
    void grayPressed();
    void exportPressed();

    // toolbar button slots
    void newButtonTriggered(bool checked = false);
    void deleteButtonTriggered(bool checked = false);
    void duplicateButtonTriggered(bool checked = false);

    void resizeEvent(QResizeEvent *event) override;
    
    void updateUnderMouseLabels(string label);
    void mousePositionChange(int x, int y);
    
    void hideBoxesToggled(bool checked);
    
private:
    void saveRecAndLabelData(size_t index);

    Project& mProject;
    LabelingCanvas* mCanvas;
    QLineEdit* mLabelEdit;
    QLineEdit* mFilterEdit;
    QListWidget* mListWidget;
    QLabel* mPosition;
    QLabel* mSize;
    QLabel* mLabelStrUnderMousePointer;
    QLabel* mMousePosition;
    QPushButton* mHideBoxesBtn;
};

#endif // __MAINWINDOW_H__
