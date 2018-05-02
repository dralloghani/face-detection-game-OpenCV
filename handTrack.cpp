#include "opencv2/opencv.hpp"
#include "windows.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "time.h"

using namespace cv;
using namespace std;

class HandTracker
{
	public:
		HandTracker(void);
		bool init(Mat frame, Rect &trackBox);
		void detectPalm(Mat img, Rect &box);
		bool isHand(const Mat frame);
		void getSkinModel(const Mat img, Rect rect);
		void frameDiff(const Mat image, Mat &diff);
		void calSkinPro(Mat frame);
		bool processFrame(Mat frame, Rect &trackBox);
		bool detectFist(Mat frame, Rect palmBox);


	private:
		Mat backProject;
		MatND hist;
		CascadeClassifier palmCascade;
		CascadeClassifier fistCascade;
		Mat preGray;
		int successiveDetect;
};

HandTracker::HandTracker()
{
	successiveDetect = 0;
	// loading pre-trained cascade classifier model
	const char *palmCascadeName = "..\\src/palm.dat";
	const char *fistCascadeName = "..\\src/fist.dat";

	if (!palmCascade.load(palmCascadeName) || !fistCascade.load(fistCascadeName))
	{
		cout << "Can not load cascade!" << endl;
	}
}

// init function: detect hand region and init meanshift
bool HandTracker::init(Mat frame, Rect &trackBox)
{
	trackBox = Rect(0, 0, 0, 0);

	// detect hand
	detectPalm(frame, trackBox);

	// The detected box should large enough and not near the boundary of image
	if (trackBox.area() > 900 && 0.3 * frame.cols < trackBox.x + 0.5 * trackBox.width
	    && trackBox.x + 0.5 * trackBox.width < 0.7 * frame.cols
	    && 0.3 * frame.rows < trackBox.y + 0.5 * trackBox.height
	    && trackBox.y + 0.5 * trackBox.height < 0.7 * frame.rows)
	{
		// Check skin area of the detected box to make sure it is a hand
		if (isHand(frame(trackBox)))
		{
			// To avoid detecting error, need to successive detect twice successfully
			successiveDetect++;
			if (successiveDetect > 2)
			{
				// Calculate skin probability model for meanshift
				getSkinModel(frame, trackBox);
				successiveDetect = 0;
				return true;
			}
		}
	}
	return false;
}

// detect hands and return the biggest hand
void HandTracker::detectPalm(Mat img, Rect &box)
{
	double scale = 1.3;
	Mat small_img, gray;
	vector<Rect> boxs;
	gray.create(img.rows, img.cols, CV_8UC1);
	// resize
	small_img.create(cvRound(gray.rows / scale), cvRound(gray.cols / scale), CV_8UC1);
	// convert to gray scale
	cvtColor(img, gray, CV_BGR2GRAY);
	resize(gray, small_img, small_img.size(), 0, 0, INTER_LINEAR);
	// histogramed, convert light images into darker ones/enhence contrast and illumination
	equalizeHist(small_img, small_img);
	// feature detection
	palmCascade.detectMultiScale(small_img, boxs, 1.1, 2, CV_HAAR_SCALE_IMAGE, Size(30, 30));
	//Get the bigest hand
	Rect maxBox(0, 0, 0, 0);

	for (vector<Rect>::const_iterator r = boxs.begin(); r != boxs.end(); r++)
	{
		if (r->area() > maxBox.area())
			maxBox = *r;
	}

	if (boxs.size() > 0)
	{
		box.x = cvRound(maxBox.x * scale);
		box.y = cvRound(maxBox.y * scale);
		box.width = cvRound(maxBox.width * scale);
		box.height = cvRound(maxBox.height * scale);
	}
}

// check skin area of our tracking box to make sure it is a hand
bool HandTracker::isHand(const Mat frame)
{
	Mat YCbCr;
	vector<Mat> planes;
	int count = 0;
	// convert to gray image
	cvtColor(frame, YCbCr, CV_RGB2YCrCb);
	// split other color layers
	split(YCbCr, planes);

	MatIterator_<uchar> it_Cb = planes[1].begin<uchar>(), it_Cb_end = planes[1].end<uchar>();
	MatIterator_<uchar> it_Cr = planes[2].begin<uchar>();

	// skin satisfy: 138 <= Cr <= 170 and 100 <= Cb <= 127 (empirical value)
	for (; it_Cb != it_Cb_end; ++it_Cr, ++it_Cb)
	{
		if (138 <= *it_Cr &&  *it_Cr <= 170 && 100 <= *it_Cb &&  *it_Cb <= 127)
			count++;
	}

	// It is a hand when contains large enough skin area
	return (count > 0.4 * frame.cols * frame.rows);
}

// Calculate skin probability model (histogram) for meanshift
void HandTracker::getSkinModel(const Mat img, Rect rect)
{
	int hue_Bins = 50;
	float hue_Ranges[] = { 0, 180 };
	const float *ranges = hue_Ranges;

	Mat HSV, hue, mask;
	cvtColor(img, HSV, CV_RGB2HSV);
	inRange(HSV, Scalar(0, 30, 10), Scalar(180, 256, 256), mask);
	vector<Mat> planes;
	split(HSV, planes);
	hue = planes[0];

	Mat roi(hue, rect), maskroi(mask, rect);
	calcHist(&roi, 1, 0, maskroi, hist, 1, &hue_Bins, &ranges);
	normalize(hist, hist, 0, 255, CV_MINMAX);
}

// Calculate skin probability image (back project map) for meanshift
void HandTracker::calSkinPro(Mat frame)
{
	Mat mask, hue, HSV;
	cvtColor(frame, HSV, CV_RGB2HSV);
	inRange(HSV, Scalar(0, 30, 10), Scalar(180, 256, 256), mask);
	vector<Mat> planes;
	split(HSV, planes);
	hue = planes[0];

	// hue varies from 0 to 179, see cvtColor
	float hue_Ranges[] = { 0, 180 };
	const float *ranges = hue_Ranges;

	calcBackProject(&hue, 1, 0, hist, backProject, &ranges, 1.0, true);
	backProject &= mask;
}

//  Detect motion using frame differece
void HandTracker::frameDiff(const Mat image, Mat &diff)
{
	int thresValue = 20;
	Mat curGray;
	cvtColor(image, curGray, CV_RGB2GRAY);
	if (preGray.size != curGray.size)
		curGray.copyTo(preGray);

	absdiff(preGray, curGray, diff);
	threshold(diff, diff, thresValue, 255, CV_THRESH_BINARY);
	erode(diff, diff, Mat(3, 3, CV_8UC1), Point(-1, -1));
	dilate(diff, diff, Mat(3, 3, CV_8UC1), Point(-1, -1));
	curGray.copyTo(preGray);
}

// Tracking hand using meanshift
bool HandTracker::processFrame(Mat frame, Rect &trackBox)
{
	float rate = 0.9;
	Mat diff;

	// tracking hand
	calSkinPro(frame);        // skin information
	frameDiff(frame, diff);   // motion information

	// fusing skin and motion information using a weighted rate
	Mat handProMap = backProject * rate + (1 - rate) * diff;
	meanShift(handProMap, trackBox, TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1));

	// ensure the tracking result is a hand
	Mat skin = backProject(trackBox) > 100;
	return countNonZero(skin) > 0.4 * trackBox.area();
}

// Detect fist for the command: click the mouse
bool HandTracker::detectFist(Mat frame, Rect palmBox)
{
	Rect detectFistBox;
	detectFistBox.x = (palmBox.x - 40) > 0 ? (palmBox.x - 40) : 0;
	detectFistBox.y = (palmBox.y - 20) > 0 ? (palmBox.y - 20) : 0;
	detectFistBox.width = palmBox.width + 80;
	detectFistBox.height = palmBox.height + 40;
	detectFistBox &= Rect(0, 0, frame.cols, frame.rows);

	Mat gray;
	cvtColor(frame, gray, CV_BGR2GRAY);
	Mat tmp = gray(detectFistBox);
	vector<Rect> fists;
	fistCascade.detectMultiScale(tmp, fists, 1.1, 2, CV_HAAR_SCALE_IMAGE, Size(30, 30));

	return fists.size();
}

// main loop for tracking and recording data
int main(int argc, char* argv[])
{

	// image frame
	Mat frame;
	// tracking box
	Rect trackBox;
	Point prePoint, curPoint;
	// camera object
	VideoCapture capture;
	bool gotTrackBox = false;
	int interCount = 0;
	// count for fist gesture
	int continue_fist = 0;
	// turn on the camera
	capture.open(0);
	if (!capture.isOpened())
	{
		cout << "Camera fail!\n" << endl;
		return -1;
	}
	// hand tracking object
	HandTracker hand_tracker;
	int fist_or_palm = -1;

	while(true)
	{
		// search for hand that needs to be tracked
		while (!gotTrackBox)
		{
			capture >> frame;
			if (frame.empty())
				return -1;

			if (hand_tracker.init(frame, trackBox))
			{
				gotTrackBox = true;
				prePoint = Point(trackBox.x + 0.5 * trackBox.width, trackBox.y + 0.5 * trackBox.height);
			}
			fist_or_palm = 0;
			imshow("handTracker", frame);

			if (waitKey(2) == 27)
				return -1;
		}

		capture >> frame;
		double t = (double)cvGetTickCount();

		// tracking hand
		if (!hand_tracker.processFrame(frame, trackBox))
			gotTrackBox = false;

		curPoint = Point(trackBox.x + 0.5 * trackBox.width, trackBox.y + 0.5 * trackBox.height);
		prePoint = curPoint;

		// When making a fist, to avoid successive active within short time,
		// next 10 frames will be ignored
		interCount++;
		if (interCount > 10 && hand_tracker.detectFist(frame, trackBox))
		{
			// To avoid detecting error, need to successive detect three times successfully
			continue_fist++;
			if (continue_fist > 3)
			{
				interCount = 0;
				fist_or_palm = 1;
			}
		}
		else
			continue_fist = 0;

		// show image
		rectangle(frame, trackBox, Scalar(0, 0, 255), 3);
		imshow("handTracker", frame);

		t = (double)cvGetTickCount() - t;
		cout << "cost time: " << t / ((double)cvGetTickFrequency()*1000.) << endl;

		if (cvWaitKey(3) == 27)
			break;
			fist_or_palm = -1;

		printf("Now gesture %d is detected.", fist_or_palm);
	}
	return 0;
}
