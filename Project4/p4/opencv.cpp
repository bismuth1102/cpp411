#include <iostream>
#include <opencv2/opencv.hpp>
#include <unistd.h>

/** Store the arguments for our program */
struct arg_t {
  /** The input image */
  std::string in_img = "";

  /** The output image */
  std::string out_img = "";

  /** The output video */
  std::string out_vid = "";

  /** Should we show the usage */
  bool usage = false;
};

/**
 * Parse the command-line arguments, and use them to populate the provided args
 * object.
 *
 * @param argc The number of command-line arguments passed to the program
 * @param argv The list of command-line arguments
 * @param args The struct into which the parsed args should go
 */
void parse_args(int argc, char **argv, arg_t &args) {
  using namespace std;
  long opt;
  while ((opt = getopt(argc, argv, "i:j:v:h")) != -1) {
    switch (opt) {
    case 'i':
      args.in_img = string(optarg);
      break;
    case 'j':
      args.out_img = string(optarg);
      break;
    case 'v':
      args.out_vid = string(optarg);
      break;
    case 'h':
      args.usage = true;
      break;
    }
  }
}

/**
 * Find the pixel with manhattan distance closest to (255,255,255), and draw a
 * 5-pixel radius red circle at that point.  Also write "Michael Spear" on the
 * image.
 *
 * @param mat The matrix to update
 */
void colormark(cv::Mat &mat) {
  using namespace cv;
  using namespace std;
  pair<int, pair<int, int>> best = {0, {0, 0}};
  for (int y = 0; y < mat.rows; ++y) {
    for (int x = 0; x < mat.cols; ++x) {
      auto pixel = mat.at<Vec3b>(y, x); // !!!
      int val = pixel[0] + pixel[1] + pixel[2];
      if (val > best.first)
        best = {val, {x, y}};
    }
  }
  circle(mat, Point(best.second.first, best.second.second), 5,
         Scalar(0, 0, 255), -1, 8);
  // NB: this is not the fastest approach, since we could make the text once and
  // then copy it onto the image each time, instead of re-rendering the text.
  putText(mat, "Michael Spear", Point(5, 25), FONT_HERSHEY_DUPLEX, 0.8,
          cvScalar(255, 0, 0), 1, CV_AA);
}

int main(int argc, char **argv) {
  using namespace std;
  using namespace cv;

  // Parse command line arguments, and if usage was requested, print it and exit
  arg_t args;
  parse_args(argc, argv, args);
  if (args.usage) {
    cout << "-i: in-image, -j: out-image, -v: out-video" << endl;
    exit(0);
  }

  // Load the image as a Mat
  Mat in = imread(args.in_img, CV_LOAD_IMAGE_COLOR);

  // Blur the image, save it as out_img
  Mat blurry;
  blur(in, blurry, Size(5, 5));
  imwrite(args.out_img, blurry);

  // Create a video consisting of 160 frames.  The first 16 and last 16 frames
  // are the unblurred image.  The other frames increase the blur from 1 to 64,
  // and then back down from 64 to 1.  The images should be half height and half
  // width, relative to the input image.  Set the frame rate to 16 fps, so that
  // we can really see the blurring and unblurring.  For each image, be sure to
  // mark the red dot and write my name.
  int w = in.cols, h = in.rows;
  Mat shrunk, res;
  cv::resize(in, shrunk, Size(w, h), 0, 0, INTER_LANCZOS4);
  VideoWriter out;
  out.open(args.out_vid, out.fourcc('a', 'v', 'c', '1'), 16, Size(w, h), true);
  // first second (16 frames): no blur
  //
  // NB: could colormark it once and reuse that...
  for (int i = 0; i < 16; ++i) {
    res = shrunk.clone();
    colormark(res);
    out << res;
  }
  // four seconds (64 frames) of increasing blurriness
  for (int i = 1; i <= 64; ++i) {
    blur(shrunk, res, Size(i, i));
    colormark(res);
    out << res;
  }
  // four seconds (64 frames) of decreasing blurriness
  // NB: I choose to re-generate the images, instead of using the memory to save
  // all the mats that we made in the previous loop
  for (int i = 64; i >= 1; --i) {
    blur(shrunk, res, Size(i, i));
    colormark(res);
    out << res;
  }
  // last second (16 frames): no blur
  //
  // NB: could colormark it once and reuse that...
  for (int i = 0; i < 16; ++i) {
    res = shrunk.clone();
    colormark(res);
    out << res;
  }
  out.release();
}