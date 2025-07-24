
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
#include <fstream>

#include <opencv2/dnn.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/ocl.hpp>

std::string camTitle(int num){
    std::string title("Video");
    title += std::to_string(num);
    return title;
}

cv::Size2f imageShape(cv::Size(320,320));

using namespace std::chrono_literals;
using namespace std;

using namespace cv;

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyAMA0"

cv::dnn::Net net = cv::dnn::readNet("/home/pi/Arusha/yolo/majic_modelv1.onnx");

int uart0_filestream = -1;

int errors, spdChange, motor2green;
int flight, blight, fright, bright = 0;
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

bool revacturn = false;
bool levacturn = false;

bool is_cuda = false;

float base_kp = 1.0;
float kp = 0.3;
int targetValue = 160;
int targetSpeed = 75;

const float SCORE_THRESHOLD = 0.35;
const float NMS_THRESHOLD = 0.35;

bool silverdeposit = false;
bool blackdeposit = false;

int pickup_count = 0;

std::vector<std::string> class_list{"alive_victim", "az", "dead_victim", "dz"};

int rx_length = 0;
//ensure camera cable is fully in
//test camera on terminal
//tighten motors
//clear time of flight paths
//recalibrate camera color values

lccv::PiCamera cam;

//class ID:
//0 == av;
//1 == az;
//2 == dv;
//3 == dz;

//object structure array:
//0 and 1 == av;
//2 == az;
//3 == dv;
//4 == dz;


struct Obj{
	bool found = false;
	float dist;
	int x, y;
	int width;
	int height;
};

Obj obj[5];

bool contour_cmp(vector<Point> &a, vector<Point> &b) {
	return contourArea(a) < contourArea(b);
}

float findDist(int pWidth, float fVal, float objWidth){
	return (objWidth * fVal)/pWidth;
}

void setZero(){
	for(int i = 0; i < 5; i++){
		obj[i].found = false;
		obj[i].x = 0;
		obj[i].y = 0;
		obj[i].dist = 0.0;
		obj[i].height = 0;
		obj[i].width = 0;
	}
}

void loop() {
	char tx_buffer[20];
	
	sprintf(tx_buffer, "[%d %d %d]", flight, fright, result);
	printf("\nflight: %d, fright: %d, result: %d\n", flight, fright, result);

	if (uart0_filestream != -1) {
		int count = 0;
		count = write(uart0_filestream, &tx_buffer[0], strlen(tx_buffer));  //Filestream, bytes to write, number of bytes to write
		if (count < 0) {
			printf("\ncount: %d\n", count);
			printf("uart: %d\n", uart0_filestream);
			printf("UART TX error\n");
			printf("Error opening file: %s\n", strerror(errno));

		} else {
			//printf("my name is maya");
		}
	}
}

void PID_track(Mat image, int x, int y) {
	printf("PID TRACK ENTERED");
	circle(image, Point(x, y), 6, Scalar(255, 100, 255), -1);
	
	errors = targetValue - x;
	spdChange = errors * kp;

	flight = targetSpeed + spdChange;
	fright = targetSpeed - spdChange;
	printf("spdChange: %d\t flight: %d\t fright: %d\t errors: %d", spdChange, flight, fright, errors);
	
}

void getInfo(Mat& img) {
		Mat blob;
		
		// does necessary scaling and stuff for NN usage
		dnn::blobFromImage(img, blob, 1.0/255, imageShape, Scalar(), true, false);
		//dnn::blobFromImage(img, blob, 1.0, imageShape);//, Scalar(), true, false);
		//dnn::blobFromImage(img, blob, 1.0/255, imageShape, Scalar(0,0,0), false, false, CV_32F);
		
		net.setInput(blob);
		
		vector<Mat> outputs;
		net.forward(outputs, net.getUnconnectedOutLayersNames());
		
		// some info about the output
		//int rows = outputs[0].size[2];
		//int dimensions = outputs[0].size[1];
		int rows = outputs[0].size[1];
		int dimensions = outputs[0].size[2];
		/*if(dimensions > rows){
			printf("\n\nYOLO 8\n\n");
		}
		else{
			printf("\n\nYOLO 5\n\n");
		}*/
		//printf("Rows: %5d\tDimensions: %5d\n", rows, dimensions);     // 2100 rows and 8 dimensions
		
		// swap for yolov8 
		rows = outputs[0].size[2];
		dimensions = outputs[0].size[1];
		
		// changing around dimensions (it changed for no reason in yolov8 so we need to switch it back)
		outputs[0] = outputs[0].reshape(1, dimensions);
		cv::transpose(outputs[0], outputs[0]);
		
		float *data = (float *)outputs[0].data;
		
		vector<int> class_ids;
		vector<float> confidences;
		vector<Rect> boxes;
		 
		
		// loop through all the rows in the results
		
		for(int i = 0; i < rows; i++){
			float *classes_scores = data+4;
			
			Mat scores(1, class_list.size(), CV_32FC1, classes_scores);
			Point class_id;
			double maxClassScore;
			
			minMaxLoc(scores, 0, &maxClassScore, 0, &class_id);
			
			if(maxClassScore > 0.5){//SCORE_THRESHOLD){
				//printf("i = %d\tscore: %.2f\n", i, maxClassScore);
				
				confidences.push_back(maxClassScore);
				class_ids.push_back(class_id.x);
				
				//printf("class id: %d\n", class_id.x);
				
				float x = data[0];
				float y = data[1];
				float w = data[2];
				float h = data[3];
				
				//printf("x,y => %.2f, %.2f\n", x, y);
				
				int left = x * img.cols - w * img.cols / 2;
				int top = y * img.rows - h * img.rows / 2;
				
				int width = int(w * img.cols);
				int height = int(h * img.rows);
				
				//int left = int((x - 0.5 * w) * x_factor);
				//int top = int((y - 0.5 * h) * y_factor);
				
				//int width = int(w * x_factor);
                //int height = int(h * y_factor);
				boxes.push_back(Rect(left, top, width, height));
				//rectangle(img, Rect(left, top, width, height), Scalar(0,255,0), 1);
			}
			
			data += dimensions;
		}
		
		//printf("does it make it this far?????");
		
		// all done, time to draw
		vector<int> nms_result;
		dnn::NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, nms_result);
		
		for(unsigned long i = 0; i < nms_result.size(); i++){
			int idx = nms_result[i];
			rectangle(img, boxes[idx], Scalar(0,255,0), 1);
			putText(img, to_string( int(confidences[idx] * 100)) + "% " + class_list[class_ids[idx]], Point(boxes[idx].x, boxes[idx].y), 1, 3, Scalar(0,255,0), 2);
			/*boxes[idx][0] left coordinate
			boxes[idx][1] right coordinate
			to get to center, add half the width and height*/
			
			if(obj[0].found == false && class_ids[idx] == 0){
				obj[0].x = (int)boxes[idx].x + (0.5*(boxes[idx].width));
				obj[0].y = (int)boxes[idx].y + (0.5*(boxes[idx].height));
				obj[0].dist = findDist(boxes[idx].width, 308.88, 4.5);
				obj[0].height = boxes[idx].height;
				obj[0].width = boxes[idx].width;
				
				if (obj[0].x > 40 && obj[0].x < 280) {
					obj[0].found = true;
					printf("\nsilver ball found\n");
				}
			}
			else if(obj[0].found == true && obj[1].found == false && class_ids[idx] == 0){
				
				obj[1].x = boxes[idx].x + (0.5*(boxes[idx].width));
				obj[1].y = boxes[idx].y + (0.5*(boxes[idx].height));
				obj[1].dist = findDist(boxes[idx].width, 308.88, 4.5);
				obj[1].height = boxes[idx].height;
				obj[1].width = boxes[idx].width;
				if (obj[1].x > 40 && obj[1].x < 280) {
					obj[1].found = true;
					printf("\nsecond silver ball found\n");
				}

			}
			else if(class_ids[idx] == 1){ //alive zone
				obj[2].found = true;
				obj[2].x = boxes[idx].x + (0.5*(boxes[idx].width));
				obj[2].y = boxes[idx].y + (0.5*(boxes[idx].height));
				obj[2].dist = findDist(boxes[idx].height, 415, 6);
				obj[2].height = boxes[idx].height;
				obj[2].width = boxes[idx].width;
				printf("\nalive zone found\n");

			}
			else if(class_ids[idx] == 2){
				obj[3].x = boxes[idx].x + (0.5*(boxes[idx].width));
				obj[3].y = boxes[idx].y + (0.5*(boxes[idx].height));
				obj[3].dist = findDist(boxes[idx].width, 308.88, 4.5);
				obj[3].height = boxes[idx].height;
				obj[3].width = boxes[idx].width;
				printf("\nblack ball found\n");
				if (obj[3].x > 40 && obj[3].x < 280) {
					obj[3].found = true;
					printf("\nsecond silver ball found\n");
				}
			}
			else if(class_ids[idx] == 3){
				obj[4].found = true;
				obj[4].x = boxes[idx].x + (0.5*(boxes[idx].width));
				obj[4].y = boxes[idx].y + (0.5*(boxes[idx].height));
				obj[4].dist = findDist(boxes[idx].height, 415, 6);
				obj[4].height = boxes[idx].height;
				obj[4].width = boxes[idx].width;
				printf("\ndead zone found\n");

			}
		}
		
		imshow("main", img);
}

void evacroom(Mat img) {
	bool fsilver = false;
	bool ssilver = false;
	bool black = false;
	bool grarea = false;
	bool rarea = false;

	getInfo(img);
	if(obj[0].found == true){
		fsilver = true;
	}
	if(obj[1].found == true){
		ssilver = true;
	}
	if(obj[2].found == true){
		grarea = true;
	}
	if(obj[3].found == true){
		black = true;
	}
	if(obj[4].found == true){
		rarea = true;
	}
	printf("\nfirst silver: %d\n", fsilver);
	printf("\nsecond silver: %d\n", ssilver);
	
	if (fsilver == true && pickup_count < 2) {
		if (ssilver == true) {
			if(obj[0].dist < obj[1].dist){
				
				if (obj[0].dist > 15) {
					PID_track(img, obj[0].x, obj[0].y);
					result = 0;
					loop();
				}
				else {
					result = 1; //pickup
					loop();
					this_thread::sleep_for(1s); // sleep for one second
					pickup_count++;  // increase pickup count
				}
			}
			else {
				if (obj[1].dist > 15) {
					PID_track(img, obj[1].x, obj[1].y);
					result = 0;
					loop();
				}
				else {
					result = 1; //pickup
					loop();
					this_thread::sleep_for(1s);
					pickup_count++;
				}
			}
		}
		else {
			if(obj[0].dist > 15) {
			//while(obj[0].dist > 7){
				PID_track(img, obj[0].x, obj[0].y);
				result = 0;
				loop();
				/*cam.getVideoFrame(img,99999999);
				imshow("newimg", img);
				getInfo(img);
				char key = (char) waitKey(1);	
				cout << "waitKey value: " << key << endl;
			
				if (key == 'q') {
					break;
				}
				printf("DIST: %d\n", (int)obj[0].dist);
			}*/
			printf("DIST: %d\n", (int)obj[0].dist);
			}
			
			// else pickup
			
			
			// not really needed???
			cam.getVideoFrame(img,99999999);
			imshow("newimg", img);
			getInfo(img);
			waitKey(1);
			
			if (obj[0].dist < 15) {
				result = 1;
				loop();
				this_thread::sleep_for(1s);
				pickup_count++;
			}
		}
	}
	
	
	else if(silverdeposit == false && grarea == true && pickup_count == 2){
		if(obj[2].dist > 20){
			PID_track(img, obj[2].x, obj[2].y);
			result = 0;
			loop();
			
		}
		else{
			result = 2;
			loop();
			this_thread::sleep_for(1s);
			silverdeposit = true;
		}
		/*
		float ratio = 205200;
		float area = obj[2].width * obj[2].height;
		float turnangle = ratio / area;
		if(turnangle < 4){turnangle = 5;}
		
		if(revacturn){
			result = (int)turnangle;
		}
		else if(levacturn){
			result = -(int)turnangle;
		}
		else{
			result = 0; //drop off
		}*/
		
		
	}
	
	else if(silverdeposit == true && black == true && pickup_count < 3){
		if(obj[3].dist > 15){
			PID_track(img, obj[3].x, obj[3].y);
			result = 0;
			loop();
		}
		
		else{
			result = 1; //pickup
			loop();
			pickup_count++;
		}
	}
	
	else if(silverdeposit == true && rarea == true && pickup_count == 3){
		if(obj[4].dist > 20){
			PID_track(img, obj[4].x, obj[4].y);
			result = 0;
			loop();
		}
		
		else{
			result = 2;
			loop();
			this_thread::sleep_for(1s);
			blackdeposit = true;
		}
			
		/*float ratio = 205200;
		float area = obj[4].width * obj[4].height;
		float turnangle = ratio / area;
		if(turnangle < 4){turnangle = 5;}
		
		if (revacturn && levacturn) {
			result = 2;
		}
		else if(revacturn){
			result = (int)turnangle;
		}
		else if(levacturn){
			result = -(int)turnangle;
		}
		else{
			result = 0; //drop off
		}*/
	}
	
	else {
		flight = -45;
		fright = 45;
		result = 0;
		loop();
	}
	
	/*
	if (fsilver == true) {
		if(ssilver == true){
			if(obj[0].dist < obj[1].dist) {
				while(obj[0].dist > 7){
					PID_track(img, obj[0].x, obj[0].y);
					result = 0;
					loop();
				}
				result = 1; //pickup
				loop();
				pickup_count++;
				
			}
			else {
				while(obj[1].dist > 7){
					PID_track(img, obj[1].x, obj[1].y);
					result = 0;
					loop();
				}
				result = 1; //pickup
				loop();
				pickup_count++;
			}
		}
		else {
			while(obj[0].dist > 7){
				PID_track(img, obj[0].x, obj[0].y);
				result = 0;
				loop();
			}
			result = 1; //pickup
			loop();
			pickup_count++;
		}
	}
	
	else {
		flight = -50;
		fright = 50;
		result = 0;
		loop();
	}*/
	setZero();
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
	
	if(is_cuda){
		net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
		net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
	}
	else{
		net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
		net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
	}
	
	while (rx_length != 10) {
	
		if(!cam.getVideoFrame(img,99999999)) {
			printf("!ERROR!\n");
			break;
		}
		
		/*if (rx_length == 6) {
			linetrace(img);
		}
		else */if (rx_length == 6) {
			evacroom(img);
		}
		/*else if (rx_length == 4) {
			linetrace(img);
		}*/
		
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
		
		//printf("rxlength: %d", rx_length);
	}
	
	cam.stopVideo();
	destroyAllWindows();

	return 0;
}
