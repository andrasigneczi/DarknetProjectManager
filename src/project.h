#ifndef __PROJECT_H__
#define __PROJECT_H__

#include <string>
#include <vector>
#include <map>
#include <QRect>

using namespace std;

class Project {
public:
    struct NormRect {
        double mCenterX;
        double mCenterY;
        double mWidth;
        double mHeight;
    };
    
    struct RectAndLabel {
        string   mLabel;
        NormRect mNormRect;
        //int      mLabelId;
        //QRect    mRect;
        //int      mImageWidth;
        //int      mImageHeight;
    };

    // load the datafile and the others listed  in the data 
    void openProject(string path);
    void saveProject(string path);

    // returns with the imagelist of the training files
    const vector<string>& getTrainAndTestFileList() const { return mTrainAndTestFiles; }
    // returns with the bounding rect vector of the selected image
    const vector<RectAndLabel>& getLabelsAndRects(size_t labelId) const { return mRectsAndLabels[labelId]; };

    // The MainWindow calls this function if anything was modified and saving necessary
    void updateLabelAndRect(size_t labelId, const vector<RectAndLabel>& updatedLabelsAndRects);

    const vector<string>& getNames() { return mNames; }
    
private:
    // main/project file loading
    void loadDataFile(string path);
    // Names file contains the labels
    void loadNamesFile();
    // train.txt and test.txt file contain a list of image paths
    void loadTrainAndTestTxt();
    void loadTrainAndTestTxt(ifstream& file);
    // load all the txt file which contains one or more label id and bounding rect
    // description
    void loadBoundingRecFiles();
    // parse a line of the data file
    void parseDataFileLine(string line);
    // the program loads the whole content of a label file
    // this function parses this string line by line and fills the mRectsAndLabels
    void parseLabelFileString(string content);
    // Txt path contains the bounding rects' descriptions and the label id
    string getTxtPath(string imgPath);
    // Saving the labels
    void saveNames();

    // Labels file path
    string mNamesFilePath;
    // Data dir file path
    string mDataFilePath;
    // Every relative path compared to this folder
    string mProjectFolder;
    // .data file's content
    map<string, string> mDataMap;
    // Labels, the index is the id
    vector<string> mNames;
    // File names from the training and test file list
    vector<string> mTrainAndTestFiles;
    // The txt files' content in structures
    vector<vector<RectAndLabel>> mRectsAndLabels;
};

#endif // __PROJECT_H__