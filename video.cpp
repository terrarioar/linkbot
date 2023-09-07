#include "video.h"
#include <iostream>
//有关摄像头的函数
Camera::Camera()
{

}


Camera::Camera(int cam)
{
	camera_num = cam;
}


void Camera::setcam(int cam)//读取设置的摄像头索引号
{
	camera_num = cam;
}


void Camera::open_camera()//打开摄像头
{
	if (!capture.open(camera_num))
		std::cout << "open camera defeat." << std::endl;
	//调整分辨率
	else {
		capture.set(CAP_PROP_FRAME_WIDTH, 1280);
		capture.set(CAP_PROP_FRAME_HEIGHT, 720);
		nFps = capture.get(CV_CAP_PROP_FPS);
	}
}

void Camera::close_camera()
{
	capture.release();
}

Mat& Camera::read_frame()//读取图像
{
	capture >> img;
	return img;
}


bool Camera::isopened()
{
	if (capture.isOpened())
		return true;
	else
		return false;
}
