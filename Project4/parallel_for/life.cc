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

class Panel{
public:
	int **buf; //store the state of matrix
	int rows;  //number of rows of the input matrix
	int cols;  //number of cols of the input matrix
	int cell_length;   //number of pixels one edge of per cell
	Mat img;

	Panel(Mat &image, int length);
	bool cellIsAlive(Point cell);
	void setCellColor(Point cell, int R, int G, int B);
	int neighbourCount(Point cell);
	bool vaildCell(Point cell);
    void addWaterMark(string waterMark);
    void showInit(VideoWriter &out, string waterMark);
    void show(VideoWriter &out, string waterMark);
	void clearBuf();
};

Panel::Panel(Mat &image, int length) {
	img = image;
    cell_length = length;
    rows = (img.rows)/cell_length;
    cols = (img.cols)/cell_length;
    buf = new int*[rows];   //dynamicly construct a 2d array
    for(int i=0; i<rows; i++){
    	buf[i] = new int[cols];
    }
    clearBuf();
}

//check a cell is alive or not, by checking one of the pixels of that cell
bool Panel::cellIsAlive(Point cell) {
	int left_up_x = cell.x*cell_length;
	int left_up_y = cell.y*cell_length;
	if(img.at<Vec3b>(left_up_x, left_up_y)[0]==0&&
		(img.at<Vec3b>(left_up_x, left_up_y)[1]==0)&&
		(img.at<Vec3b>(left_up_x, left_up_y)[2]==0)){
			return true;
	}
	else{
		return false;
	}
}

//set color of one cell
void Panel::setCellColor(Point cell, int R, int G, int B) {
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
bool Panel::vaildCell(Point cell){
    if(cell.x>=0&&cell.x<rows&&cell.y>=0&&cell.y<cols){
        return true;
    }
    else{
        return false;
    }
}

//count the number of alive cells of one cell
int Panel::neighbourCount(Point cell) {
    int lifeCount=0;
    Point loc = cell + Point(-1,-1);

    if(vaildCell(loc)&&cellIsAlive(loc)) lifeCount++; loc = loc+Point(0,1);
    if(vaildCell(loc)&&cellIsAlive(loc)) lifeCount++; loc = loc+Point(0,1);
    if(vaildCell(loc)&&cellIsAlive(loc)) lifeCount++; loc = loc+Point(1,0);
    if(vaildCell(loc)&&cellIsAlive(loc)) lifeCount++; loc = loc+Point(1,0);
    if(vaildCell(loc)&&cellIsAlive(loc)) lifeCount++; loc = loc+Point(0,-1);
    if(vaildCell(loc)&&cellIsAlive(loc)) lifeCount++; loc = loc+Point(0,-1);
    if(vaildCell(loc)&&cellIsAlive(loc)) lifeCount++; loc = loc+Point(-1,0);
    if(vaildCell(loc)&&cellIsAlive(loc)) lifeCount++;

    return lifeCount;
}

//fill buf with 0
void Panel::clearBuf() {
    for(int i=0; i<rows; i++) {
        for(int j=0; j<cols; j++) {
            buf[i][j]=0;
        }
    }
}

//add watermark
void Panel::addWaterMark(string waterMark){
    putText(img, waterMark, Point(5, 25), FONT_HERSHEY_DUPLEX, 0.8, cvScalar(255, 0, 0), 1, CV_AA);
}

//show the state of the input file(matrix)
void Panel::showInit(VideoWriter &out, string waterMark){
    addWaterMark(waterMark);
    out << img;
}

//show every matrix, use two parallel_for here. One in updating int **buf, one in updating Mat img
void Panel::show(VideoWriter &out, string waterMark){

    for(int i=0; i<rows; i++) {
        parallel_for(tbb::blocked_range<int>(0, cols, 4),
            [&](const tbb::blocked_range<int>& r){
                for (auto j=r.begin(); j!=r.end(); ++j) {
                    if(cellIsAlive(Point(i,j))) {
                    if(neighbourCount(Point(i,j))<2) buf[i][j]=0;
                    if(neighbourCount(Point(i,j))>3) buf[i][j]=0;
                    if(neighbourCount(Point(i,j))==2) buf[i][j]=1;
                    if(neighbourCount(Point(i,j))==3) buf[i][j]=1;
                    }
                    else {
                        if(neighbourCount(Point(i,j))==3) buf[i][j]=1;
                    }
                }
            });
    }

    for(int i=0; i<rows; i++) {
        parallel_for(tbb::blocked_range<int>(0, cols, 4),
            [&](const tbb::blocked_range<int>& r){
                for (auto j=r.begin(); j!=r.end(); ++j) {
                    Point cell(i,j);
                    if(buf[i][j]==1) {
                        setCellColor(cell, 0, 0, 0);
                    }
                    else{
                        setCellColor(cell, 255, 255, 255);
                    }
                }
            });
    }

    addWaterMark(waterMark);
    out << img;

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

    //init panel
	Mat maze(rows*cell_length, cols*cell_length, CV_8UC3, Scalar(0));
	Panel panel(maze, args.cellSize);
	for(int i=0; i<panel.rows; i++) {
        for(int j=0; j<panel.cols; j++) {
        	Point cell(i,j);
            panel.setCellColor(cell, 255, 255, 255);
        }
    }
	
	char c;
	int size = rows*cols;
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
    		panel.setCellColor(cell, 0, 0, 0);
    	}
    }
    delete [] data;

    VideoWriter out;
    out.open(args.out_vid, out.fourcc('a', 'v', 'c', '1'), args.frameRate, Size(panel.img.cols, panel.img.rows), true);

    vector<string> vStr;
    boost::algorithm::split(vStr, args.framePic, boost::is_any_of( "," ));

    panel.showInit(out, args.waterMark);
    for(int r=0; r<args.round; r++){
        if(find(vStr.begin(), vStr.end(), to_string(r)) != vStr.end()){
            imwrite(to_string(r)+".jpg", panel.img);
        }
        panel.show(out, args.waterMark);
    }
    
    out.release();

    for(int i=0; i<panel.rows; i++) {
        delete [] panel.buf[i];
    }
    delete [] panel.buf;  
}
