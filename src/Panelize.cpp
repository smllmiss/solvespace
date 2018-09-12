//==== Color + Object Detection + Panelization code =====//
//										  				 //
//-------------------------------------------------------//
// Author: Akash Kothari  <akothar3@ncsu.edu>            //
//-------------------------------------------------------//

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cstdint>
#include <dirent.h>
#include <fstream>
#include "Panelize.h"

using namespace cv;
using namespace std;

namespace Panelize {

#ifndef BEST_AREA_FIT_HEURISTIC
	#ifndef BEST_SHORT_SIDE_FIT_HEURISTIC
		#ifndef BEST_LONG_SIDE_FIT_HEURISTIC
		/* Use the Best Area Fit Heuristic by default */
			#define BEST_AREA_FIT_HEURISTIC 
		#endif
	#endif
#endif

// This struct holds HSV values for a color
struct HSV_info {
	uint8_t lowH;
	uint8_t highH;
	uint8_t lowS;
	uint8_t highS;
	uint8_t lowV;
	uint8_t highV;
};

// Node to the HSV node
struct HSV_node {
	String color;
	struct HSV_info HSV_val;
	struct HSV_node *next;
};

// struct for storing wall coordinates
struct obj_info {
	Point2f rect_points[4];
	String color;
	struct obj_info *next;
};

struct wall_info {
	String color;
	Point2f rect_points[4];
	double length;
	struct wall_info *next;
};

struct inv_node {
	double panel_length;
	double panel_thickness;
	struct inventory_node *next;
};

struct HSV_node *HSVList = nullptr;
struct obj_info *objInfoList = nullptr;
struct wall_info *wallInfoList = nullptr;
struct inv_info *invInfoList = nullptr;

//===================================== IMAGE ANALYZER ==================================//

void Load_Image(Mat &image, const String &fileName)
{
// Load source image and convert it to gray
	image = imread(fileName);
	if(image.empty()) {
		cout << "Error: Could not read image file\n";
		exit(-1);
	}
}

void Display_Image(Mat &image, const String &imageName)
{
//Pop a window to display the image
	namedWindow(imageName , WINDOW_AUTOSIZE);
	imshow(imageName, image);
}

void Convert_Image_from_BGR_to_HSV(Mat &image)
{
	cvtColor(image, image, COLOR_BGR2HSV);
}

void Convert_Image_from_BGR_to_HSV(Mat &dest_image, Mat &image)
{
	cvtColor(image, dest_image, COLOR_BGR2HSV);
}

inline bool compareContourAreas(vector<Point> contour1, vector<Point> contour2) 
{
    return (fabs(contourArea(Mat(contour1))) < fabs(contourArea(Mat(contour2))));
}

void Find_Contours(Mat &image, vector<vector<Point>> &contours, vector<Vec4i> &hierarchy)
{
	findContours(image, contours, hierarchy, RETR_EXTERNAL,
							CHAIN_APPROX_SIMPLE, Point(0, 0));
	sort(contours.begin(), contours.end(), compareContourAreas);
}

void Draw_Contours(Mat &cont_image, Mat &image, vector<vector<Point>> &contours, 
					vector<Vec4i> &hierarchy, const String &windowName)
{
// Draw contours on an image with black background
	Mat new_image = Mat::zeros(image.size(), CV_64F);
	int i = 0;
	while(i != contours.size()) {
		drawContours(new_image, contours, i, (150, 50, 25), 2, 8, hierarchy, 0, Point());
		i++;
	}
	
// Display the image
	Display_Image(new_image, windowName);
}

void Add_HSV_node(struct HSV_info val, String color)
{
// Prepend the node to the list
	struct HSV_node *node = HSVList;
	HSVList = new HSV_node;
	HSVList->color = color;
	HSVList->next = node;
	HSVList->HSV_val.lowH = val.lowH;
	HSVList->HSV_val.highH = val.highH;
	HSVList->HSV_val.lowS = val.lowS;
	HSVList->HSV_val.highS = val.highS;
	HSVList->HSV_val.lowV = val.lowV;
	HSVList->HSV_val.highV = val.highV;
}

void Add_HSV_node(uint8_t lowH, uint8_t lowS, uint8_t lowV, 
				uint8_t highH, uint8_t highS, uint8_t highV, String color)
{
// Prepend the node to the list
	struct HSV_node *node = HSVList;
	HSVList = new HSV_node;
	HSVList->color = color;
	HSVList->next = node;
	HSVList->HSV_val.lowH = lowH;
	HSVList->HSV_val.highH = highH;
	HSVList->HSV_val.lowS = lowS;
	HSVList->HSV_val.highS = highS;
	HSVList->HSV_val.lowV = lowV;
	HSVList->HSV_val.highV = highV;
}

void Get_Masked_Image(Mat &masked_image, Mat &image, struct HSV_info HSV_val)
{
	inRange(image, Scalar(HSV_val.lowH, HSV_val.lowS, HSV_val.lowV), 
				   Scalar(HSV_val.highH, HSV_val.highS, HSV_val.highV), masked_image);
}

void Get_Masked_Image(Mat &masked_image, Mat &image, uint8_t lowH, uint8_t highH, 
							uint8_t lowS, uint8_t highS, uint8_t lowV, uint8_t highV)
{
	inRange(image, Scalar(lowH, lowS, lowV), Scalar(highH, highS,highV), masked_image);
}

double Euclidean_Distance(Point2f point1, Point2f point2)
{
// Calculate the euclidean distance between the points
	double dist = norm(point1 - point2);
	//cout << "POINT DISTANCE: " << dist << "\n";
	return norm(point1 - point2);
}

void Box_Objects(vector<vector<Point>> &contours)
{
// Store the contour rectangles in an array	
	vector<RotatedRect> minRect(contours.size());	
	int i = 0;
	while(i != contours.size()) {
		minRect[i] = minAreaRect(Mat(contours[i]));
		i++;
	}
	
//	Get the points in a vector
	i= 0;
	while(i != contours.size()) {
		Point2f rect_points[4];
		minRect[i].points(rect_points);
		
	// Print the corners
		cout << "Printing corners\n";
		int j = 0;
		while(j != 4) {
			cout << "(";
			cout << rect_points[j].x;
			cout << ",";
			cout << rect_points[j].y;
			cout << ")\n";
			j++;
		}
		i++;
	}
}

struct obj_info *Add_ObjInfo_node(void)
{
// Prepend the node to the list
	struct obj_info *node = objInfoList;
	objInfoList = new obj_info;
	objInfoList->next = node;
	return objInfoList;
}

struct obj_info *Add_ObjInfo_node(String &color)
{
	struct obj_info *node = Add_ObjInfo_node();
	node->color = color;
	return node;
}

void Box_Objects(vector<vector<Point>> &contours, String &color)
{
// Store the contour rectangles in an array	
	vector<RotatedRect> minRect(contours.size());	
	int i = 0;
	while(i != contours.size()) {
		minRect[i] = minAreaRect(Mat(contours[i]));
		i++;
	}
	
//	Get the points in a vector
	i= 0;
	while(i != contours.size()) {
	// Put the coordiinates in an object info list
		struct obj_info *node = Add_ObjInfo_node(color);
		minRect[i].points(node->rect_points);
		
	// Print the corners
		cout << "Printing corners\n";
		int j = 0;
		while(j != 4) {
			cout << "(";
			cout << node->rect_points[j].x;
			cout << ",";
			cout << node->rect_points[j].y;
			cout << ")\n";
			j++;
		}
		i++;
	}
}

void Add_WallInfo_Node(struct obj_info *objInfoNodePtr, double length)
{
	struct wall_info *node = wallInfoList;
	wallInfoList = new wall_info;
	wallInfoList->next = node;
	wallInfoList->color = objInfoNodePtr->color;
	
// Copy the coordinates to the new wall info node
	int i = 0;
	while(i != 4) {
		wallInfoList->rect_points[i] = objInfoNodePtr->rect_points[i];
		i++;
	}
	
// Put the length in	
	wallInfoList->length = length;
}

void Filter_Noise(void)
{
	cout << "+++++++++++ FILTER NOISE ++++++++\n";
// Filter noise. Avoid anything that does not have a proper, significant size
// Traverse the object list and filter the buggering noise out. For rectangles,
// there are only three unique distances. We pick two shortest lengths, ignoring
// the longest one which is most likely to be the diagonal.
	double wall_width = 0;
	struct obj_info *obj_node = objInfoList;
	while(obj_node) {
	// Compare the distance of a vertex of the rectangle from other vertices	
		double length = 0;
		double width = 0;
		int i = 1;
		while(i != 4) {
			double dist = Euclidean_Distance(obj_node->rect_points[0], obj_node->rect_points[i]);
			if(dist) {
				if(!length) {
					length = dist;
				} else {
					if(!width)
						width = dist;
				}
				if(length < width) {
					double temp = length;
					length = width;
					width = temp;
				}
				if(!(dist > length && dist > width)) {
					length = dist;
					if(length < width) {
						double temp = length;
						length = width;
						width = temp;
					}
				}
				//cout << "length: " << length << "\n";
				//cout << "width: " << width << "\n";
			}
			i++;
		}
		if(length && width) {
			cout << "length: " << length << "\n";
			cout << "width: " << width << "\n";
		// Check whether the width is much smaller than the lenghth
			if(length/width >= 2) {
			// Add a wall info node
				if(!wall_width) {
					wall_width = width;
				} else {
					if(width != wall_width) {
						obj_node = obj_node->next;
						continue;
					}
				}
				Add_WallInfo_Node(obj_node, length);
			}
		}
		obj_node = obj_node->next;
	}
}

void Print_WallInfo_List(void)
{
	cout << "\n\n********** PRINTING WALL INFO ************\n";
	struct wall_info *node = wallInfoList;
	while(node) {
	// Wall color	
		cout << "Color: " << node->color << "\n";
		
	// Print the corners
		cout << "Corners: \n";
		int j = 0;
		while(j != 4) {
			cout << "(";
			cout << node->rect_points[j].x;
			cout << ",";
			cout << node->rect_points[j].y;
			cout << ")\n";
			j++;
		}
		
	// Wall length
		cout << "Length: " << node->length << "\n\n";
		node = node->next;
	}
	cout << "\n*******************************************\n";
}

void Add_inventory(double panel_length, double panel_thickness)
{
// Prepend node
	//struct inv_info *node = invInfoList;
	//invInfoList->next = node;
	//invInfoList->panel_length = panel_length;
	//invInfoList->panel_thickness = panel_thickness;
}

double Get_Number_of_Panels(double wall_length, double wall_thickness)
{
	cout << "******* PANELIZE ********\n";
	cout << "WALL LENGTH: " << wall_length << "\n";
	//cout << "WALL THICKNESS: " << wall_thickness << "\n";
	
	MaxRectsBinPack *bin = new MaxRectsBinPack(wall_length, wall_thickness);
	//bin.Init(wall_length, wall_thickness);
	RectBin packedRect;
	
#ifdef BEST_AREA_FIT_HEURISTIC	
	MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBestAreaFit;
#endif
#ifdef BEST_SHORT_SIDE_FIT_HEURISTIC
	MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBestShortSideFit;
#endif
#ifdef BEST_LONG_SIDE_FIT_HEURISTIC
	MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBestLongSideFit;
#endif
	
	struct panel_info {
		double length;
		double thickness;
		double count;
	};
	
	struct panel_info panel1 =  {9.5, 1, 0}; 
	int i = 0;
	while(i++ != ((wall_length * wall_thickness)/(panel1.length * panel1.thickness))) {
		packedRect = (*bin).Insert(panel1.thickness, panel1.length, heuristic);
		if(!(packedRect.height > 0))
			break;
	}
	cout << "PANEL1 LENGTH: " << panel1.length << "\n";
	//cout << "PANEL1 THICKNESS: " << panel1.thickness << "\n";
	cout << "PANEL1 COUNT: " << i - 1 << "\n";
	
	struct panel_info panel2 =  {4.5, 1, 0}; 
	int j = 0;
	while(j++ != ((wall_length * wall_thickness)/(panel2.length * panel2.thickness))) {
		packedRect = (*bin).Insert(panel2.thickness, panel2.length, heuristic);
		if(!(packedRect.height > 0)) 
			break;
	}
	cout << "PANEL2 LENGTH: " << panel2.length << "\n";
	//cout << "PANEL2 THICKNESS: " << panel2.thickness << "\n";
	cout << "PANEL2 COUNT: " << j - 1 << "\n";
	
	cout << "\n********\n";
	return (i + j - 2);
}

void Export_Data_to_File(ofstream myfile)
{
	myfile.open("test.csv");

	myfile << "Wall Panel Type" << "," << "Quantity" << endl;
	myfile << "8 x 12" << "," << 120 << endl;
	myfile << "4 x 12" << "," << 32 << endl;
	myfile << "3 x 12" << "," << 5 << endl;
	myfile << "2 x 12" << "," << 12 << endl;
	myfile << "1 x 12" << "," << 8 << endl;

	myfile.close();

}

void Detect_Walls(const String &fileName)
{
	Mat image;
	Load_Image(image, fileName);
	Display_Image(image, "Source");
	Mat conv_image;
	Convert_Image_from_BGR_to_HSV(conv_image, image);
	
// Add HSV nodes for blue, red, yellow
	//Add_HSV_node(17, 15, 100, 50, 56, 200, "Red"); //Red
	//Add_HSV_node(86, 31, 4, 220, 88, 50, "Blue"); //Blue
	Add_HSV_node(120, 0, 0, 180, 255, 255, "Blue"); //Blue (WORKS)
	Add_HSV_node(30, 0, 0, 119, 255, 255, "Yellow"); //Yellow
	//Add_HSV_node(0, 20, 0, 175, 255, 255, "Green"); //Green probably

	struct HSV_node *node  = HSVList;
	while(node) {
		Mat masked_image;
		Get_Masked_Image(masked_image, conv_image, node->HSV_val);
		
		char little_bugger[1024] = {0};
		sprintf(little_bugger, "Masked image for %s colored walls", (node->color).c_str());
		Display_Image(masked_image, string(little_bugger));
		
	//	Find contours
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;
		Find_Contours(masked_image, contours, hierarchy);
		
		//findContours(masked_image, contours, hierarchy, RETR_EXTERNAL,
			//						CHAIN_APPROX_SIMPLE, Point(0, 0));
		//sort(contours.begin(), contours.end(), compareContourAreas);
		
		cout << "Color " << node->color << "\n"; 
		
	// Draw contours
		Mat cont_image;
		sprintf(little_bugger, "Boxed %s colored walls", (node->color).c_str());
		Draw_Contours(cont_image, masked_image, contours, hierarchy, string(little_bugger));
		
	// Get the cordinates of the vertices of boxed objects in the image
		Box_Objects(contours, node->color);
		
		waitKey(100000);
		node = node->next;
	}
	
	Filter_Noise();
	Print_WallInfo_List();
	double count1 = Get_Number_of_Panels(100, 1);
	cout << "TOTAL NUMBER OF PANELS " << count1 << "\n\n";
	double count2 = Get_Number_of_Panels(200, 1);
	cout << "TOTAL NUMBER OF PANELS " << count2 << "\n\n";
}


//======================================= HSV TRACKER ============================================//

void Create_Trackbar_Window(const String &windowName)
{
// Create a window
	namedWindow(windowName);
	
// Create trackbars	
	int min_slide_val = 0;
	int max_slide_val = 180;
	createTrackbar("lowH", windowName, &min_slide_val, 180);
	createTrackbar("highH", windowName, &max_slide_val, 180);
	max_slide_val = 255;
	createTrackbar("lowS", windowName, &min_slide_val, 255);
	createTrackbar("highS", windowName, &max_slide_val, 255);
	createTrackbar("lowV", windowName, &min_slide_val, 255);
	createTrackbar("highV", windowName, &max_slide_val, 255);
}

void Analyse_HSV_of_Image(const String &imageName, const String &winName)
{
// Read the image
	Mat image;
	Load_Image(image, imageName);

// Create a window with track bars on it	
	Create_Trackbar_Window(winName);

// Analyse image
	while(1) {
	// Get the positions of the trackbars
		int lowH = getTrackbarPos("lowH", winName);
		int highH = getTrackbarPos("highH", winName);
		int lowS = getTrackbarPos("lowS", winName);
		int highS = getTrackbarPos("highS", winName);
		int lowV = getTrackbarPos("lowV", winName);
		int highV = getTrackbarPos("highV", winName);
		
		Mat cont_image;
		Convert_Image_from_BGR_to_HSV(cont_image, image);
		Get_Masked_Image(cont_image, cont_image, lowH, highH, lowS, highS, lowV, highV);
		resize(cont_image, cont_image, Size(), 0.75, 0.75);
		Display_Image(cont_image, winName);
		waitKey(1000);  //large wait time to avoid freezing
	}
}
}

int main(int argc, char** argv )
{
	Panelize::Detect_Walls(argv[1]);
	//Analyse_HSV_of_Image(argv[1], "Trackwin");
	return 0;
}
