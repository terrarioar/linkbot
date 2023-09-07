#ifndef VIDEO_H
#define VIDEO_H


#include <opencv2/opencv.hpp>
using namespace cv;

class Camera//¿ØÖÆÉãÏñÍ·
{
private:

	int camera_num = 0;
	Mat img;
	VideoCapture capture;



public:
	int nFps;

	Camera();

	Camera(int cam);

	void setcam(int cam);
	
	void open_camera();

	void close_camera();

	Mat& read_frame();

	bool isopened();
};


#endif

