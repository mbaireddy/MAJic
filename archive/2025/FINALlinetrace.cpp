
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/signal.h>
#include <time.h>
#include <stdbool.h>
#include <errno.h>

#include <chrono>
#include <thread>

#include "opencv2/opencv.hpp"
#include <opencv2/imgproc.hpp>
#include <lccv.hpp>
#include <unistd.h>
#include <iostream>

using namespace std::chrono_literals;
using namespace std;

using namespace cv;

#define BAUDRATE B115200
#define MODEMDEVICE "/dev/ttyAMA0"

int uart0_filestream = -1;

int errors, spdChange, motor2green;
int flight, blight, fright, bright;
int result = 0;

int lastflights[10] = { 0 };
int lastfrights[10] = { 0 };
//test camera on terminal
//tighten motors
//clear time of flight paths
//recalibrate camera color values

bool stopMotors = false;
bool initgr = false;
bool onball = false;
int movement = 0;

bool panic = false;

float base_kp = 1.0;
float kp = 0.3;
int targetValue = 160;
int targetSpeed = 75;

int rx_length = 0;
//ensure camera cable is fully in
//test camera on terminal
//tighten motors
//clear time of flight paths
//recalibrate camera color values

lccv::PiCamera cam;

bool contour_cmp(vector<Point> &a, vector<Point> &b) {
	return contourArea(a) < contourArea(b);
}

void loop() {
	char tx_buffer[20];

	if (stopMotors == 1) {
		flight = 0;
		blight = 0;
		fright = 0;
		bright = 0;
	}

	sprintf(tx_buffer, "[%d %d %d]", flight, fright, result);

	if (uart0_filestream != -1) {
		int count = 0;
		count = write(uart0_filestream, &tx_buffer[0], strlen(tx_buffer));  //Filestream, bytes to write, number of bytes to write
		if (count < 0) {
			printf("uart: %d\n", uart0_filestream);
			printf("UART TX error\n");

		} else {
			//printf("my name is maya");
		}
	}
}

void PID_track(Mat image, int x, int y) {
	circle(image, Point(x, y), 6, Scalar(255, 100, 255), -1);
	
	if (y > 150) {
		onball = true;
	}
	
	if (onball) {
		flight = 60;
		fright = 60;
	}
	else {
	errors = targetValue - x;
	spdChange = errors * kp;

	flight = targetSpeed + spdChange;
	fright = targetSpeed - spdChange;
	printf("spdChange: %d\t flight: %d\t fright: %d\t errors: %d", spdChange, flight, fright, errors);
	}
}

int greenSquare(Mat img) {
	Mat hsvimg = img.clone();
	Mat saveimg = img.clone();
	
	cvtColor(img, img, COLOR_BGR2GRAY); 
		
	GaussianBlur(img, img, Size(5,5), 0, 0);
		
	threshold(img, img, 130, 255, THRESH_BINARY_INV);	
		
	Mat thresh = img.clone();
	imshow("greensquarethresh", thresh);
		
	vector<vector<Point>> contours;
	vector<vector<Point>> greenSquares;
	findContours(thresh, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	
	cvtColor(hsvimg, hsvimg, COLOR_BGR2HSV);

	inRange(hsvimg, Scalar (50, 130, 60), Scalar (110, 255, 255), hsvimg);
	imshow("hsv", hsvimg);
	
	findContours(hsvimg, greenSquares, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	drawContours(saveimg, greenSquares, -1, Scalar (100, 255, 100), 3);	
	imshow("w/ GS contours", saveimg);
	
	int leftcount = 0;
	int rightcount = 0;
	int func = 0;
	
	for (int i = 0; i < (int)greenSquares.size(); i++) {
		func = 0;
		if (contourArea(greenSquares[i]) < 500) {
			continue;
		}
		
		Rect box = boundingRect(greenSquares[i]);
		
		Point center(box.x+(box.width/2), box.y+(box.height/2));
		Point above(box.x+(box.width/2), box.y-10);
		Point right(box.x+(box.width) +10, box.y+(box.height/2));
		Point below(box.x+(box.width/2), box.y+(box.height)+10);
		Point left(box.x-10, box.y+(box.height/2));
		
		int abovecolor = thresh.at<unsigned char>(above);
		int rightcolor = thresh.at<unsigned char>(right);
		int belowcolor = thresh.at<unsigned char>(below);
		int leftcolor = thresh.at<unsigned char>(left);

		circle(saveimg, center, 3, Scalar(255,0,255), -1); //center - pink
		circle(saveimg, above, 3, Scalar(0,0,255), -1); // above - red
		circle(saveimg, right, 3, Scalar(0,255,255), -1); //right - yellow
		circle(saveimg, below, 3, Scalar(255,0,0), -1); //below - blue
		circle(saveimg, left, 3, Scalar(255,255, 0), -1); //left - teal
		
		imshow("w/ GS contours", saveimg);
		
		if (belowcolor == 255) {
			continue;
			func = 0;
		}
		else if(abovecolor == 255 && rightcolor == 255) {
			leftcount++;
		}
		else if(abovecolor == 255 && leftcolor == 255) {
			rightcount++;
		}
	}
		
	if (leftcount && rightcount) {
		printf("doublegreen\n");
		func = 3;
	}
	else if(leftcount == 1) {
		printf("left\n");
		func = 2;
	}
	else if(rightcount == 1) {
		printf("right\n");
		func = 1;
	}
	else {
		func = 0;
	}
	
	return func;
}

bool silverDetection(Mat img) {
	Mat saveimg = img.clone();
	GaussianBlur(img, img, Size(5,5), 0, 0);
	imshow("blurredv2", img);
	cvtColor(img, img, COLOR_BGR2HSV);
	inRange(img, Scalar (50, 130, 60), Scalar (110, 255, 255), img);
	imshow("hsv", img);
	
	Mat sliced = img(Rect(0, 0, 320, 200));
	imshow("sliced", sliced);
	
    vector<vector<Point>> silvercont;
    findContours(sliced, silvercont, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    
    /*if (silvercont.size() > 3) {
		return true;
	}
	else {
		return false;
	}*/
	return false;
}

void linetrace(Mat img) {       
	
	//img width = 280
	//img height = 220
	
	imshow("base image", img);
	Mat base = img.clone();
	Mat saveimg = img.clone();
	Mat hsvimg = img.clone();
	
	cvtColor(img, img, COLOR_BGR2HSV); 
		
	medianBlur(img, img, 5);
		
	//threshold(img, img, 30, 255, THRESH_BINARY_INV);	
	inRange(img, Scalar (0, 0, 0), Scalar (180, 128, 120), img); //43 = 55
	
	dilate(img, img, getStructuringElement(MORPH_RECT, Size (7, 7), Point (3, 3)));
	erode(img, img, getStructuringElement(MORPH_RECT, Size (7, 7), Point (3, 3)));
	 
	Mat thresh = img.clone();
	imshow("black and white", thresh);
	
	vector<vector<Point>> contours;
	vector<vector<Point>> contourLeft;
	vector<vector<Point>> contourRight;
	findContours(thresh, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	drawContours(saveimg, contours, -1, Scalar (100, 255, 100), 3);	
	
	imshow("savedimg", saveimg);
	
	cout << (int)contours.size() << endl;
	
	
	//float base_kp = 1.5; //0.3 mostlu works
	float base_kp = 2.1;
	float kp = base_kp;
	int targetValue = 160;
	int targetSpeed = 65;
	
	if (silverDetection(base)) {
		Mat silvingimg = base.clone();
		Mat borimg = base.clone();
		cvtColor(silvingimg, silvingimg, COLOR_BGR2HSV);

		inRange(silvingimg, Scalar (50, 130, 60), Scalar (110, 255, 255), silvingimg);
		imshow("hsv", silvingimg);
		
		Mat sliced = silvingimg(Rect(0, 0, 320, 200));
		imshow("sliced", sliced);
		
		vector<vector<Point>> silvercont;
		findContours(sliced, silvercont, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		
		int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;

		for (size_t i = 0; i < silvercont.size(); i++) {
			Rect rect = boundingRect(silvercont[i]);
			minX = min(minX, rect.x);
			minY = min(minY, rect.y);
			maxX = max(maxX, rect.x + rect.width);
			maxY = max(maxY, rect.y + rect.height);
		}
		
		Rect combinedRect(minX, minY, maxX - minX, maxY - minY);

		rectangle(borimg, combinedRect, Scalar(0, 0, 255), 2);
		imshow("Contours with Combined Bounding Rectangle", borimg);
		
		float rectCenter = combinedRect.x+(combinedRect.width/2);
		float rectYCenter = combinedRect.y+(combinedRect.height/2);
		
		targetSpeed = 0;
		PID_track(borimg, rectCenter, rectYCenter);
		loop();
		targetSpeed = 75;
		
		if (rectCenter > 210 && rectCenter < 230) {
			flight = 50;
			fright = 50;
			loop();
		}
	}
	else if ((int)contours.size() > 0) {
		//sort(contours.begin(), contours.end(), contour_cmp);
		vector<Point> line = *max_element(contours.begin(), contours.end(), contour_cmp);
		
		if (contourArea(contours[0]) > 900) {
		
		cout << "countoursize: " << contourArea(contours[0]) << endl;

		//Moments m = moments(contours[0]);
		Moments m = moments(line);
		int cx = m.m10 / m.m00;
		int cy = m.m01 / m.m00;
		
		circle(saveimg, Point (cx, cy), 6, Scalar (255, 0, 255), -1);
		imshow("savedimg", saveimg);
		
		if(cy > 160) {
			//printf("%d", cy);
			kp = 6.3;
			panic = true;
		}
		else {
			kp = base_kp;
		}
		
		errors = targetValue - cx;
		cout << "errors: " << errors << endl;
		spdChange = errors * kp;
		//printf("%d", spdChange);
		cout << spdChange << endl;
		flight = targetSpeed + spdChange;
		//blight = targetSpeed + spdChange;
		fright = targetSpeed - spdChange;
		//bright = targetSpeed - spdChange;
		
		Mat sliceLeft = img(Rect(0, 0, 50, 240));
		findContours(sliceLeft, contourLeft, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		Mat sliceRight = img(Rect(270, 0, 50, 240));
		findContours(sliceRight, contourRight, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		
		cout << "left speed: " << flight;
		cout << "   right speed: " << fright << endl;
		
		result = greenSquare(hsvimg);
		if (result != 0) {
			int savedflight = flight;
			int savedfright = fright;
			result = 4;
			stopMotors = true;
			loop();
			
			this_thread::sleep_for(1200ms);
			
			flight = savedflight;
			fright = savedfright;
			
			stopMotors = false;
			
			Mat GRimg;
			if(!cam.getVideoFrame(GRimg,99999999)) {
				printf("!ERROR!\n");
			}
			
			imshow("GRimg", GRimg);
			
			result = greenSquare(GRimg);
		}
		
		
		for (int i = 9; i > 0; i--) {
			lastflights[i] = lastflights[i-1];
			lastfrights[i] = lastfrights[i-1];
		}
		
		lastflights[0] = flight;
		lastfrights[0] = fright;
		
		/*if (cy > 210 && (int)contourLeft.size() > 0 && (int)contourRight.size() == 0) {
			stopMotors = true;
			loop();
			this_thread::sleep_for(1.5s);
			stopMotors = false;
			flight = -128;
			fright = 128;
			printf("LEFTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT");
			loop();
			this_thread::sleep_for(1000ms);
			
		}
		
		if (cy > 210 && (int)contourLeft.size() == 0 && (int)contourRight.size() > 0) {
			stopMotors = true;
			loop();
			this_thread::sleep_for(1.5s);
			stopMotors = false;
			flight = 128;
			fright = -128;
			printf("RIGHTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT");
			loop();
			this_thread::sleep_for(1000ms);
		}*/
		
		if (panic) {
			if (flight > fright) {
				flight = 50;
				fright = -120;
			}
			else {
				fright = 50;
				flight = -120;
			}
			loop();
			this_thread::sleep_for(230ms);
			panic = false;
		}
		else {
			loop();
		}
			
	}
	}
		
	else {
		//flight = 100;
		//fright = 240;
		flight = lastflights[5];
		fright = lastfrights[5];
		loop();
		this_thread::sleep_for(150ms);
	}
}

/*
void followBall(Mat img) {
	imshow("base image", img);
	Mat hsvimg = img.clone();
	Mat saveimg = img.clone();
	
	medianBlur(img, img, 9);
	
	cvtColor(hsvimg, hsvimg, COLOR_BGR2HSV);

	inRange(hsvimg, Scalar (0, 0, 0), Scalar (160, 248, 84), hsvimg);
	imshow("hsv", hsvimg);
	
	vector<vector<Point>> contours;
	findContours(hsvimg, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	
	drawContours(saveimg, contours, -1, Scalar (100, 255, 100), 3);	
	imshow("saveimg", saveimg);
	
	Point2f center;
	float radius;
	
	float kp = 0.1;
	int targetValue = 360;
	int targetSpeed = 70;
	
	sort(contours.begin(), contours.end(), contour_cmp);
	
	for (int i = 0; i < (int)contours.size(); i++) {
		if (contourArea(contours[i]) < 500) {
			continue;
		}
		
		minEnclosingCircle(contours[i], center, radius);
		circle(saveimg, center, radius, Scalar (255, 0, 255), 3);
		imshow("saveimg", saveimg);
		
		errors = targetValue - center.x;
		
		spdChange = errors * kp;
		//printf("%d", spdChange);
		cout << spdChange << endl;
		leftSpd = targetSpeed + spdChange;
		rightSpd = targetSpeed - spdChange;
		
		loop();
		
	}
}
*/
void evacroom(Mat img) {

	medianBlur(img, img, 7);

	Mat hsvimg = img.clone();
	//Mat silverimg = img.clone();
	Mat blackimg = img.clone();
	Mat saveimg = img.clone();

	//vector<vector<Point>> silver_contour;
	vector<vector<Point>> black_contour;
	vector<vector<Point>> red_contour;
	vector<vector<Point>> green_contour;

	cvtColor(hsvimg, hsvimg, COLOR_BGR2HSV);

	//Mat silver = hsvimg.clone();
	Mat black = hsvimg.clone();
	Mat red = hsvimg.clone();
	Mat green = hsvimg.clone();

	imshow("base image", img);

	//inRange(silver, Scalar(85,0,156), Scalar(180, 255, 255), silver);
	//imshow("silver", silver);
	//imshow("silver_blur", silver_blur);
	//findContours(silver, silver_contour, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	//destroyWindow("windowname");

	inRange(black, Scalar(90, 40, 0), Scalar(126, 155, 145), black);
	imshow("black", black);
	findContours(black, black_contour, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	inRange(red, Scalar(0, 67, 0), Scalar(1, 255, 255), red);
	//imshow("red", red);
	findContours(red, red_contour, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	inRange(green, Scalar(71, 102, 0), Scalar(90, 255, 255), green);
	//imshow("green", green);
	findContours(green, green_contour, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	
	//drawContours(silver, silver_contour, -1, Scalar(255, 100, 100), 3);
	drawContours(black, black_contour, -1, Scalar(255, 100, 100), 3);
	drawContours(red, red_contour, -1, Scalar(255, 100, 100), 3);
	drawContours(green, green_contour, -1, Scalar(255, 100, 100), 3);

	Point2f center;
	float radius;

	int ball = 1;  //for when to track black vs silver ball. 0 means has not finished silver yet

	//we were getting a segmentation fault because we didn't check for what to do when there is no silver contour
	//then max_element would return null, and we cant find the moments of the ball
	//re get threshholds in the daylight?
	auto b = max_element(black_contour.begin(), black_contour.end(), contour_cmp);
	//auto s = max_element(silver_contour.begin(), silver_contour.end(), contour_cmp);
	//auto g = max_element(green_contour.begin(), green_contour.end(), contour_cmp);
	auto r = max_element(red_contour.begin(), red_contour.end(), contour_cmp);

	unsigned char rx_buffer[256];
	int rx_length = 0;
	rx_length = read(uart0_filestream, (void *)rx_buffer, 255);
	printf("rxlength: %d\n", rx_length);

	/*if (ball == 0 && rx_length < 0 && s != silver_contour.end()) {  //tracking silver, nothing in serial, CAN find contour
		minEnclosingCircle(silver_contour[0], center, radius);
		circle(silverimg, center, radius, Scalar(255, 0, 255), 3);

		vector<Point> ballc = *s;
		Moments m = moments(ballc);
		int cx = m.m10 / m.m00;
		int cy = m.m01 / m.m00;

		PID_track(silverimg, cx, cy);
		imshow("colored silver w/ contour", silverimg);
	}*/

	if (ball == 1 && rx_length < 0 && b != black_contour.end()) {  //tracking black, nothing in serial, CAN find contour
		minEnclosingCircle(black_contour[0], center, radius);
		circle(blackimg, center, radius, Scalar(255, 0, 255), 3);

		vector<Point> ballc = *b;
		Moments m = moments(ballc);
		int cx = m.m10 / m.m00;
		int cy = m.m01 / m.m00;

		PID_track(blackimg, cx, cy);
		imshow("colored black w/ contour", blackimg);
	}
	
	else if (rx_length == 1 || rx_length == 2 || rx_length == 3) {
		onball = false;
		//printf("rxbuffer[0]: %c\n", rx_buffer[0]);
		//printf("rxbuffer[1]: %c\n", rx_buffer[1]);
		//start tracking green to deposit ball
		
		printf("ballllllllllllllllllllllllllllllllllllllllllllll");
		
		result = 0;
		stopMotors = true;
		loop();
		this_thread::sleep_for(4000ms);
		stopMotors = false;

		if (rx_length == 2) {
			ball = 2;  //2 is green tracing
		}

		if (rx_length == 3) {
			ball = 3;
		}
	}
	
	else if (rx_length == 4) {
		//printf("rxbuffer[0]: %c\n", rx_buffer[0]);
		//printf("rxbuffer[1]: %c\n", rx_buffer[1]);
		//start tracking green to deposit ball

		result = 0;
		stopMotors = true;
		loop();
		this_thread::sleep_for(6800ms);
		stopMotors = false;
		
		/*if (ball == 2) { //just finished green
			ball = 1; //start tracing black
		}*/
		
		if (ball == 3) { //just finished red
			ball = 5; //do nothing
		}
	}

	/*else if (ball == 2 && rx_length < 0 && g != green_contour.end()) {  //green tracing, nothing in serial, CAN find contour
		vector<Point> ballc = *g;
		Moments m = moments(ballc);
		int cx = m.m10 / m.m00;
		int cy = m.m01 / m.m00;

		PID_track(silverimg, cx, cy);
	}*/

	else if (ball == 3 && rx_length < 0 && r != red_contour.end()) {  //red tracing, nothing in serial, CAN find contour
		vector<Point> ballc = *r;
		Moments m = moments(ballc);
		int cx = m.m10 / m.m00;
		int cy = m.m01 / m.m00;

		PID_track(blackimg, cx, cy);

		ball = 4;
	}
	else if (ball == 5) {
		rx_length = 4;
	}
	
	else {  //keep spinning if cant find contour
		flight = -128;
		fright = 128;
		result = 6;
		//result = 0;
	}

	//imshow("w/ silver contours", silver);
	imshow("w/ black contours", black);
	loop();
	result = 0;
}

void init() {
	uart0_filestream = open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_NDELAY);  //Open in non blocking read/write mode
	if (uart0_filestream == -1) {
		//ERROR - CAN'T OPEN SERIAL PORT
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
		exit(-1);
	}

	struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;  //<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);
}

int main(void) {
	init();

	printf("gp20 found");

	Mat img;

	cam.options->camera = 0;
	cam.options->video_width = 320;
	cam.options->video_height = 240;
	cam.options->framerate = 30;
	cam.options->verbose = true;

	cam.startVideo();

	namedWindow("hi", WINDOW_NORMAL);
	
	/*
	while(1) {
		loop();
		this_thread::sleep_for(5ms);
	}*/
	
	unsigned char rx_buffer[256];
	
	while (rx_length <= 0) {
		rx_length = read(uart0_filestream, (void *)rx_buffer, 255);
	}
	
	rx_buffer[rx_length] = '\0';
	printf("Start data recieved: %s\n", rx_buffer);
	
	if (rx_length < 0) {
		//An error occured (will occur if there are no bytes)
	} else if (rx_length == 0) {
		//No data waiting - if we are non-blocking
	} else{
		//Bytes received
		rx_buffer[rx_length] = '\0';
		printf("%i bytes read : %s", rx_length, rx_buffer);
	}
	
	while (rx_length != 10) {
		if(!cam.getVideoFrame(img,99999999)) {
			printf("!ERROR!\n");
			break;
		}
		
		if (rx_length > 0) { //change back to 6
			linetrace(img);
		}
		else if (rx_length == 2) {
			//evacroom(img);
			//stopMotors = true;
			//flight = 0;
			//fright = 0;
			//loop();
			this_thread::sleep_for(1800ms);
		}
		else if (rx_length == 4) {
			linetrace(img);
		}
		
		char key = (char) waitKey(1);	
		cout << "waitKey value: " << key << endl;
		
		if (key == 'q') {
			break;
		} else if (key == ' ') {
			stopMotors = true;
		} else if (key == 'g') {
			stopMotors = false;
		}
		
		int test = read(uart0_filestream, (void*)rx_buffer, 255);
		
		if (test > 0 ||test==2) {
			rx_length = test;
		}
		
		printf("rxlength: %d", rx_length);
	}
	
	cam.stopVideo();
	destroyAllWindows();

	return 0;
}
