#include "video.h"
#include <iostream>
//�й�����ͷ�ĺ���
Camera::Camera()
{

}


Camera::Camera(int cam)
{
	camera_num = cam;
}


void Camera::setcam(int cam)//��ȡ���õ�����ͷ������
{
	camera_num = cam;
}


void Camera::open_camera()//������ͷ
{
	if (!capture.open(camera_num))
		std::cout << "open camera defeat." << std::endl;
	//�����ֱ���
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

Mat& Camera::read_frame()//��ȡͼ��
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
