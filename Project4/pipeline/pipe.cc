#include <fstream>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>
#include <unistd.h>
#include <boost/algorithm/string.hpp>
#include <tbb/tbb.h>

using namespace std;
using namespace cv;
using namespace tbb;

struct arg_t {
    int x = 1;
    int y = 1;
    int frameRate = 1;
    int cellSize = 20;
    int round = 20;
    string framePic = "1,2,3";
    string waterMark = "vancleecheng";
    string out_vid = ""; //The output video
    bool usage = false;   //Should we show the usage
};

//check a cell is alive or not, by checking the certain cell in int** buf
bool cellIsAlive(int** buf, Point cell) {
	if(buf[cell.x][cell.y]==1)
        return true;
    else
        return false;
}

//set color of one cell
void setCellColor(Mat &img, Point cell, int cell_length, int R, int G, int B) {
	// cout << cell << endl;
	for(int x=cell.x*cell_length; x<(cell.x+1)*cell_length; x++) {
        for(int y=cell.y*cell_length; y<(cell.y+1)*cell_length; y++) {
            img.at<Vec3b>(x,y)[0] = R;
            img.at<Vec3b>(x,y)[1] = G;
            img.at<Vec3b>(x,y)[2] = B;
        }
    }
}

//in case of reaching the border
bool vaildCell(Point cell, int rows, int cols){
    if(cell.x>=0 && cell.x<rows && cell.y>=0 && cell.y<cols){
        return true;
    }
    else{
        return false;
    }
}

//count the number of alive cells of one cell
int neighbourCount(int** buf, Point cell, int rows, int cols) {
    int lifeCount=0;
    Point loc = cell + Point(-1,-1);

    if(vaildCell(loc, rows, cols)&&cellIsAlive(buf, loc)) lifeCount++; loc = loc+Point(0,1);
    if(vaildCell(loc, rows, cols)&&cellIsAlive(buf, loc)) lifeCount++; loc = loc+Point(0,1);
    if(vaildCell(loc, rows, cols)&&cellIsAlive(buf, loc)) lifeCount++; loc = loc+Point(1,0);
    if(vaildCell(loc, rows, cols)&&cellIsAlive(buf, loc)) lifeCount++; loc = loc+Point(1,0);
    if(vaildCell(loc, rows, cols)&&cellIsAlive(buf, loc)) lifeCount++; loc = loc+Point(0,-1);
    if(vaildCell(loc, rows, cols)&&cellIsAlive(buf, loc)) lifeCount++; loc = loc+Point(0,-1);
    if(vaildCell(loc, rows, cols)&&cellIsAlive(buf, loc)) lifeCount++; loc = loc+Point(-1,0);
    if(vaildCell(loc, rows, cols)&&cellIsAlive(buf, loc)) lifeCount++;

    return lifeCount;
}

//fill int** buf with 0
void clearBuf(int** buf, int rows, int cols) {
    for(int i=0; i<rows; i++) {
        for(int j=0; j<cols; j++) {
            buf[i][j]=0;
        }
    }
}

//add watermark
void addWaterMark(Mat &img, string waterMark){
    putText(img, waterMark, Point(5, 25), FONT_HERSHEY_DUPLEX, 0.8, cvScalar(255, 0, 0), 1, CV_AA);
}

//show the state of the input file(matrix)
void showInit(VideoWriter &out, Mat &img, string waterMark){
    addWaterMark(img, waterMark);
    out << img;
}

//produce a new int** buf from the old one
int** Pmatrix(int** buf, int rows, int cols){
    int **temp = new int*[rows];
    for(int i=0; i<rows; i++){
        temp[i] = new int[cols];
    }
    clearBuf(temp, rows, cols);
    
    for(int i=0; i<rows; i++) {
        for(int j=0; j<cols; j++) {
            if(cellIsAlive(buf, Point(i,j))) {
                if(neighbourCount(buf, Point(i,j), rows, cols)<2) temp[i][j]=0;
                if(neighbourCount(buf, Point(i,j), rows, cols)>3) temp[i][j]=0;
                if(neighbourCount(buf, Point(i,j), rows, cols)==2) temp[i][j]=1;
                if(neighbourCount(buf, Point(i,j), rows, cols)==3) temp[i][j]=1;
            }
            else {
                if(neighbourCount(buf, Point(i,j), rows, cols)==3) temp[i][j]=1;
            }
        }
    }
    return temp;
}

//produce a mat based on int** buf
Mat Pmat(Mat &img, int** buf, string waterMark, int rows, int cols, int cell_length){
    for(int i=0; i<rows; i++) {
        for(int j=0; j<cols; j++) {
            Point cell(i,j);
            if(buf[i][j]==1) {
                setCellColor(img, cell, cell_length, 0, 0, 0);
            }
            else{
                setCellColor(img, cell, cell_length, 255, 255, 255);
            }
        }
    }
    addWaterMark(img, waterMark);
    return img;
}

void parse_args(int argc, char **argv, arg_t &args) {
    using namespace std;
    long opt;
    while ((opt = getopt(argc, argv, "x:y:f:c:r:v:p:w:h")) != -1) {
        switch (opt) {
            case 'x':
                args.x = stoi(string(optarg));
                break;
            case 'y':
                args.y = stoi(string(optarg));
                break;
            case 'f':
                args.frameRate = stoi(string(optarg));
                break;          
            case 'c':
                args.cellSize = stoi(string(optarg));
                break;
            case 'r':
                args.round = stoi(string(optarg));
                break;
            case 'v':
                args.out_vid = string(optarg);
                break;
            case 'p':
                args.framePic = string(optarg);
                break;
            case 'w':
                args.waterMark = string(optarg);
                break;
            case 'h':
                args.usage = true;
                break;
        }
    }
}

int main(int argc, char **argv){

    arg_t args;
    parse_args(argc, argv, args);
    if (args.usage) {
        cout << "x - rows of input matrix" << endl;
        cout << "y - cols of input matrix" << endl;
        cout << "f - frame rate" << endl;
        cout << "c - cell length" << endl;
        cout << "r - rounds" << endl;
        cout << "v - name of output video" << endl;
        cout << "p - which frame you want to output as a picture. Such as \"1,2,4\" means output frame 1,2 and 4 as pictures." << endl;
        cout << "w - watermark" << endl;
    }

    int rows = args.x;
    int cols = args.y;
    int cell_length = args.cellSize;

    //get Mat
	Mat maze(rows*cell_length, cols*cell_length, CV_8UC3, Scalar(0));
    Mat img = maze.clone();

    //get int** buf
    int** buf = new int*[rows];
    for(int i=0; i<rows; i++){
        buf[i] = new int[cols];
    }
    clearBuf(buf, rows, cols);  //!!! IMPORTANT

    //init img
	for(int i=0; i<rows; i++) {
        for(int j=0; j<cols; j++) {
        	Point cell(i,j);
            setCellColor(img, cell, cell_length, 255, 255, 255);
        }
    }
	
	char c;
	int size = rows * cols;
	int *data = new int[size];

    //read file, set 1 in data[] when encounter 1 in the input file
	ifstream infile; 
   	infile.open("file.txt");
   	int k = 0;
   	while(!infile.eof()&&k<size) {
        infile >> c;
		data[k] = atoi(&c);
		k++;
    }

    //fill the cell with black if certain location is 1
    for(int i=0; i<size; i++){
    	int x = i/cols;
    	int y = i%cols;
    	Point cell(x,y);
    	if(data[i]==1){
            buf[x][y] = 1;
    		setCellColor(img, cell, cell_length, 0, 0, 0);
    	}
    }
    delete [] data;

    VideoWriter out;
    out.open(args.out_vid, out.fourcc('a', 'v', 'c', '1'), args.frameRate, Size(img.cols, img.rows), true);

    vector<string> vStr;
    boost::algorithm::split(vStr, args.framePic, boost::is_any_of( "," ));

    showInit(out, img, args.waterMark);

    //pipeline
    int index = -1;
    parallel_pipeline(3,
            make_filter<void, int**>(
                filter::serial_in_order,
                [&](flow_control &fc)->int**{
                    if(index<args.round){
                        index++;
                        buf = Pmatrix(buf, rows, cols);
                        return buf;
                    }
                    else{
                        fc.stop();
                        return NULL;
                    }
                }    
            ) &
            make_filter<int**, Mat>(
                filter::serial_in_order, 
                [&](int** buf)->Mat{
                    img = Pmat(img, buf, args.waterMark, rows, cols, cell_length);
                    //output certain pictures
                    if(find(vStr.begin(), vStr.end(), to_string(index)) != vStr.end()){
                        imwrite(to_string(index)+".jpg", img);
                    }
                    return img;
                }
            ) &
            make_filter<Mat, void>(
                filter::serial_in_order,
                [&](Mat img){
                    out << img;
            })
        );

    out.release();

    for(int i=0; i<rows; i++) {
        delete [] buf[i];
    }
    delete [] buf;
}
