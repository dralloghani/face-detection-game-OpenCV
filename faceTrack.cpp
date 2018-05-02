#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>  // Gaussian Blur
#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat, Scalar)
#include <opencv2/highgui/highgui.hpp>  // OpenCV window I/O
#include "opencv2/objdetect/objdetect.hpp"//API to face detection

using namespace cv; //necessary for using opencv apis
using namespace std;

//loading pre-trained model for face classification
string face_cascade_name = "..\\src/haarcascade_frontalface_alt.xml";
CascadeClassifier face_cascade;
string window_name = "Face Tracking";
int pos_x = -1;

void detectAndDisplay( Mat frame )
{
	std::vector<Rect> faces;
	Mat frame_gray;
	// convert to gray image
	cvtColor( frame, frame_gray, CV_BGR2GRAY );
	// standardize
	equalizeHist( frame_gray, frame_gray );
	// detect specifications
	face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );
    // draw bounding box
	for( int i = 0; i < faces.size(); i++ )
	{
		pos_x = faces[i].x;
		Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
//		printf("\n\n");
//		printf("%d, %d", faces[i].x, faces[i].y);
//		printf("%d, %d", center.x, center.y);
//		printf("\n\n");
		ellipse( frame, center, Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );
	}
	// launch the window/frame
	imshow(window_name, frame);
}

//int main()
//{
//	// open the default camera
//	VideoCapture cap(0);
//
//	// check if we succeeded
//	if (!cap.isOpened())
//	{
//		printf("\"Camera fail\"\n");
//	}
//
//	Mat edges;
//
//	//check if the cascade classification file loaded
//	if (!face_cascade.load(face_cascade_name))
//	{
//		printf("Cascade classification model not loaded\n");
//	}
//
//	// time count
//	//int nTick = 0;
//
//	while(running)
//	{
//		// wait for the camera
//		if (!cap.isOpened())
//		{
//			continue;
//		}
//		Mat frame;
//		// nTick = getTickCount();
//		// get a new frame from camera
//		cap >> frame;
//		// wait for the frame to be captured
//		if (frame.data == NULL)
//		{
//			continue;
//		}
//		cvtColor(frame, edges, CV_BGR2BGRA);
//
//		detectAndDisplay(edges);
//
//		if (waitKey(30) >= 0) break;
//	}
//}

