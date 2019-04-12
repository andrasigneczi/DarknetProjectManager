#include "project.h"
#include <fstream>
#include <iostream>
#include "util.h"
#include <QPixmap>
#include <sstream>
#include <iomanip>
#include <math.h>

using namespace util;

void Project::loadDataFile(string path) {
    mDataFilePath = path;
    ifstream namesFile(path, ios_base::in);
    string line;
    while(!namesFile.eof()) {
        getline(namesFile, line);
        //cout << line << "\n";
        parseDataFileLine(line);
    }
}

void Project::parseDataFileLine(string line) {
    // classes= 20
    // train  = /home/pjreddie/data/voc/train.txt
    // valid  = /home/pjreddie/data/voc/2007_test.txt
    // names = data/voc.names
    // backup = backup
    // labels = data/imagenet.labels.list
    // top=5
    // leaves = data/imagenet1k.labels
    // map = data/inet9k.map
    // eval = imagenet
    // results = results
    
    // removing the comments
    size_t comment = line.find('#');
    if(comment != string::npos) {
        line.resize(comment);
    }
    line = trim(line);

    size_t equals = line.find('=');
    if(equals != string::npos) {
        string key = trim(line.substr(0, equals));
        string value = trim(line.substr(equals + 1));
        //cout << "key: '" << key << "'; value: '" << value << "'" << endl;
        mDataMap.emplace(key, value);
    }
}

void Project::loadNamesFile() {
    auto it = mDataMap.find("names");
    if(it == mDataMap.end() ||it->second.length() == 0) {
        throw string("'names' is missing from the data file");
    }
    
    ifstream file(it->second, ios_base::in);
    if(!file.is_open()) {
        throw it->second + " does not exist";
    }
    string name;
    while(!file.eof()) {
        getline(file, name);
        name = trim(name);
        if(name.length() > 0) {
            mNames.push_back(name);
            //cout << "name: '" << name << "'" << endl;
            name = "";
        }
    }
}

void Project::loadTrainAndTestTxt(ifstream& file) {
    string line;
    while(!file.eof()) {
        getline(file, line);
        line = trim(line);
        if(line.length() > 0) {
            mTrainAndTestFiles.push_back(line);
            //cout << "line: '" << line << "'" << endl;
            line = "";
        }
    }
    std::sort(mTrainAndTestFiles.begin(), mTrainAndTestFiles.end());
}

void Project::loadTrainAndTestTxt() {
    auto it = mDataMap.find("train");
    if(it == mDataMap.end() || it->second.length() == 0) {
        throw string("'train' is missing from the data file");
    }
    
    ifstream file(it->second, ios_base::in);
    if(!file.is_open()) {
        throw it->second + " does not exist";
    }
    loadTrainAndTestTxt(file);
    
    it = mDataMap.find("valid");
    if(it == mDataMap.end() || it->second.length() == 0) {
        return;
    }
    
    file.close();
    file.open(it->second, ios_base::in);
    if(!file.is_open()) {
        throw it->second + " does not exist";
    }
    loadTrainAndTestTxt(file);
}

void Project::loadBoundingRecFiles() {
    for(const string& imgPath : mTrainAndTestFiles) {
        string txtPath = getTxtPath(imgPath);

        //cerr << "dbg1: " << txtPath << "\n";
        ifstream txt(txtPath);
        if(!txt.is_open()) {
            cerr << string("Warning: can't open ") + txtPath + "\n";
            mRectsAndLabels.push_back(vector<RectAndLabel>());
            continue;
        }
        
        //cerr << "dbg: " << imgPath.c_str() << "\n";
        //QPixmap img(imgPath.c_str());
        ////cerr << "dbg22\n";
        //if(img.isNull()) {
        //    throw string("Can't open ") + imgPath;
        //}
        
        cout << "Loading file " << txtPath << "\n";
        string content;
        getline(txt, content, (char) txt.eof());
        content = trim(content);
        //cout << txtPath << " content:\n" << content << endl;
        //cout << "image width: " << img.width() << "; image height: " << img.height() << "\n";
        //mBoundingRectDescriptions.push_back(content);
        parseLabelFileString(content);
        //parseLabelFileString(content, 1920, 1080);
        //cerr << "dbg4\n";
    }
}

void Project::parseLabelFileString(string content) {
    // 0 0.1 0.11 0.2 0.21
    // 1 0.1 0.11 0.2 0.21
    
    // id: 0
    // center-x: 0.1
    // center-y: 0.11
    // width: 0.2
    // height: 0.21

    // id: 1
    // center-x: 0.1
    // center-y: 0.11
    // width: 0.2
    // height: 0.21
    
    // id checking
    // read the image size and calculate the non-normalized values
    stringstream strStream(content);
    string line;
    vector<RectAndLabel> rectsAndLabels;
    while(content.length() > 0 && !strStream.eof()) {
        //getline(strStream, line);
        //cout << "parsed line: " << line << "\n";
        int id;
        double centerX, centerY, width, height;
        strStream.precision(25);
        strStream >> id >> centerX >> centerY >> width >> height;
        
        if(centerX < 1e-6 || centerY < 1e-6 || width < 1e-6 || height < 1e-6) {
            throw std::string("Invalid value in labels txts!\n");
        }
        
        // NormRect normrec = calcNormalizedRec(155, 260, 98, 24, imageWidth, imageHeight);
        // cout << setprecision(10) << "calculated norm rec: " << id << " " << normrec.mCenterX
        // << " " << normrec.mCenterY << " " << normrec.mWidth << " " << normrec.mHeight << endl;
        
        // inverse calculation for drawing
        //QRect boundingRec = calcBoundingRec(centerX, centerY, width, height, imageWidth, imageHeight);
        //cout << "calculated bounding values: " << id << " " << boundingRec.left()
        //<< " " << boundingRec.top() << " " << boundingRec.width() << " " << boundingRec.height() << endl;
        
        if(id < (int)mNames.size()) {
            rectsAndLabels.push_back({mNames[id], {centerX, centerY, width, height}});
        } else {
            std::cerr << "ERROR: Label is missing for id '" << id << "'!!!\n";
        }
        //cout << "Filling rec and labels done\n";
        
    }
    mRectsAndLabels.push_back(rectsAndLabels);
}

void Project::openProject(string path) {
    loadDataFile(path);
    loadNamesFile();
    loadTrainAndTestTxt();
    loadBoundingRecFiles();
}

void Project::saveProject(string path) {
    UNUSED(path);
}

void Project::updateLabelAndRect(size_t labelId, const vector<Project::RectAndLabel>& updatedLabelsAndRects) {
    string txtPath = getTxtPath(mTrainAndTestFiles[labelId]);
    ofstream txt(txtPath, ios_base::out | ios_base::trunc);
    if(!txt.is_open()) {
        cerr << string("Warning: can't open ") + txtPath + "\n";
        return;
    }
    
    // preparing a name map
    map<string, int> nameMap;
    int nameId = 0;
    for(const string& name : mNames) {
        nameMap.emplace(name, nameId++);
    }
    bool newNames = false;
    auto& selected = mRectsAndLabels[labelId];
    selected.clear();
    // label id, center-x, center-y, width, height
    for(size_t i = 0; i < updatedLabelsAndRects.size(); ++i) {
        selected.push_back(updatedLabelsAndRects[i]); //updatedLabelsAndRects
        if(i != 0) {
            txt << endl;
        }
        auto it = nameMap.find(selected.back().mLabel);
        if(it == nameMap.end()) {
            cerr << "Warning: label '" << selected.back().mLabel << "' not found, label file will be updated\n";
            txt << nameId << " ";
            nameMap.emplace(selected.back().mLabel, nameId++);
            newNames = true;
            mNames.push_back(selected.back().mLabel);
        } else {
            txt << it->second << " ";
        }
        txt.precision(25);
        txt << selected.back().mNormRect.mCenterX << " " << selected.back().mNormRect.mCenterY << " "
            << selected.back().mNormRect.mWidth << " " << selected.back().mNormRect.mHeight;
    }
    if(newNames) {
        saveNames();
    }
    txt.flush();
    txt.close();
}

string Project::getTxtPath(string imgPath) {
    string& txtPath = imgPath;
    replace(txtPath, "images", "labels");
    replace(txtPath, "JPEGImages", "labels");
    replace(txtPath, ".jpg", ".txt");
    replace(txtPath, ".png", ".txt");
    replace(txtPath, ".PNG", ".txt");
    replace(txtPath, ".JPG", ".txt");
    replace(txtPath, ".JPEG", ".txt");
    return txtPath;
}

void Project::saveNames() {
    auto it = mDataMap.find("names");
    ofstream names(it->second, ios_base::out | ios_base::trunc);
    if(!names.is_open()) {
        cerr << "Cannot open names file for writing\n";
        return;
    }
    
    bool first = true;
    for(const string& name : mNames) {
        if(first) first = false; else names << endl;
        names << name;
    }
    names.flush();
}
