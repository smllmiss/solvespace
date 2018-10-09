//==== Color + Object Detection + Panelization code =====//
//										  				 //
//-------------------------------------------------------//
// Author: Akash Kothari  <akothar3@ncsu.edu>            //
//-------------------------------------------------------//

#include <C:\tools\opencv\build\include\opencv2\core\core.hpp> //#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cstdint>
// #include <dirent.h> // where is this? error C1083
#include <fstream>

using namespace cv;

namespace Panelization {

// Generic class to represent colors
class Color {
// HSV values that represent colors	
	uint8_t lowH = 0;
	uint8_t highH = 0;
	uint8_t lowS = 0;
	uint8_t highS = 0;
	uint8_t lowV = 0;
	uint8_t highV = 0;
	
// Color name	
	String color;
public:
// Constructors	
	Color() = default;
	
	Color(uint8_t lowH, uint8_t highH, uint8_t lowS, uint8_t highS,
		  uint8_t lowV, uint8_t highV, String color = String()) {
		setColor(lowH, highH, lowS, highS, lowV, highV, color);
	}
	
// Get color info
	uint8_t getLowH() const { 
		return lowH;
	}
	
	uint8_t getHighH() const { 
		return highH;
	}
	
	uint8_t getLowS() const { 
		return lowS;
	}
	
	uint8_t getHighS() const { 
		return highS;
	}
	
	uint8_t getLowV() const { 
		return lowV;
	}
	
	uint8_t getHighV() const { 
		return highV;
	}
	
	String getColor() const {
		return color;
	}
	
	std::vector<uint8_t> getHSV() const {
		std::vector<uint8_t> vect;
		vect.push_back(lowH);
		vect.push_back(highH);
		vect.push_back(lowS);
		vect.push_back(highS);
		vect.push_back(lowV);
		vect.push_back(highV);
		
		return vect;
	}
	
// Set colors	
	void setColor(uint8_t lowH, uint8_t highH, uint8_t lowS, uint8_t highS,
				  uint8_t lowV, uint8_t highV, String color = String()) {
		this->lowH = lowH;
		this->highH = highH;
		this->lowS = lowS;
		this->highS = highS;
		this->lowV = lowV;
		this->highV = highV;
		this->color = color;
	}
	
// Order of parameters: LOW_H, HIGH_H, LOW_S, HIGH_S, LOW_V, HIGH_V
	void setColor(std::vector<uint8_t> &HSV_vector) {
		if(HSV_vector.size() != 6) {
			std::cout << "Invalid HSV vector\n";
			exit(-1);
		}
		
		std::vector<uint8_t> vect = HSV_vector;
		highV = vect.back();
		vect.pop_back();
		lowV = vect.back();
		vect.pop_back();
		highS = vect.back();
		vect.pop_back();
		lowS = vect.back();
		vect.pop_back();
		highH = vect.back();
		vect.pop_back();
		lowH = vect.back();
		vect.pop_back();
	}
	
	void setColor(String color_name) {
	// Check of the color is valid
		color = color_name;
	}
	
	void setColor(const Color &color) {
		*this = color;
	}
};

// Panel Object
class Panel {
// Panel Dimensions	
	double width = 0;
	double height = 0;
	
// Number of panels	
	uint32_t numPanels = 0;
public:
	Panel() = default;
	
	Panel(double panelWidth, double panelHeight = 0) {
		width = panelWidth;
		height = panelHeight;
	}
	
// Get panel info	
	double getWidth() const {
		return width;
	}
	
	double getHeight() const {
		return height;
	}
	
	double getNumPanels() const {
		return numPanels;
	}
	
// Set parameters	
	void incrementNumPanels(uint32_t increment = 1) {
		numPanels += increment;
	}
	
	void decrementNumPanels(uint32_t decrement = 1) {
		numPanels -= decrement;
	}
	
// Print Panel information
	void printPanelInfo() {
		std::cout << "+++++++++++++ PRINT PANEL INFO ++++++++++++++\n";
		std::cout << "Panel Width: " << width << "\n";
		std::cout << "Panel Height: " << height << "\n";
		std::cout << "Number of Panels: " << numPanels << "\n";
		std::cout << "+++++++++++++++++++++++++++++++++++++++++++++\n";
	}
};

// Wall object
class Wall: public Color {
// Wall dimensions
	double length = 0;
	double width = 0;
	double height = 0;

// Vector of vector of panels that best fit the wall
	std::vector<Panel *> bestFitPanelList;
	
// Cordinates of the wall in the print
	std::vector<Point2f> rectPoints;
public:
	Wall() = default;
	
	Wall(double length, double width, double height = 0) {
		this->length = length;
		this->width = width;
		this->height = height;
	}
	
	Wall(const Color &color) {
		this->setColor(color);
	}
	
// Get wall info	
	double getLength() const {
		return length;
	}
	
	double getWidth() const {
		return width;
	}
	
	double getHeight() const {
		return height;
	}
	
	std::vector<Panel *> getBestFitPanelList() const {
		return bestFitPanelList;
	}
	
	Point2f getVertex(unsigned index) const {
		return rectPoints[index];
	}
	
	bool panelIsListed(uint32_t width) {
		std::vector<Panel *>::iterator panel = bestFitPanelList.begin();
		while(panel != bestFitPanelList.end()) {
			if((*panel)->getWidth() == width)
				return true;
			panel++;
		}
		return false;
	}
	
// Setting all dimensions
	void setLength(double wallLength) {
		length = wallLength;
	}
	
	void setWidth(double wallWidth) {
		width = wallWidth;
	}
	
	void setHeight(double wallHeight) {
		height = wallHeight;
	}
	
	void setRectPoints(std::vector<Point2f> &wallCoordinates) {
		if(wallCoordinates.size() != 4) {
			std::cout << "Error in coordinates of wall\n";
			exit(-1);
		}
		rectPoints.clear();
		rectPoints.push_back(wallCoordinates[0]);
		rectPoints.push_back(wallCoordinates[1]);
		rectPoints.push_back(wallCoordinates[2]);
		rectPoints.push_back(wallCoordinates[3]);
	}
	
	void addBestFitPanels(Panel *panel, uint32_t numPanels = 1) {
		bestFitPanelList.push_back(panel);
		panel->incrementNumPanels(numPanels);
	}
	
// Verify wall info is correct
	bool wallInfoIsSane() {
		if(rectPoints.size() != 4) 
			return false;
		//if(!length || !width || !heigth)
			//return false;
		return true;
	}
	
// Print Wall info
	void printWallInfo() {
	// Verify wall info is sane
		if(!wallInfoIsSane()) {
			std::cout << "Wall info is not sane\n";
			exit(-1);
		}
		
		std::cout << "************ WALL INFO ******************\n";
		
	// Print wall dimensions	
		std::cout << "Wall Length: " << length << "\n";
		std::cout << "Wall Width: " << width << "\n";
		std::cout << "Wall Height: " << height << "\n";
	
	// Print wall color
		std::cout << "Wall Color: " << getColor() << "\n";
		
	// Print Best fit Panel info
		std::vector<Panel *>::iterator panel_it = bestFitPanelList.begin();
		while(panel_it != bestFitPanelList.end()) {
			(*panel_it)->printPanelInfo();
			panel_it++;
		}
		
	// Print wall cordinates
		std::vector<Point2f>::iterator points_it = rectPoints.begin();
		while(points_it != rectPoints.end()) {
			std::cout << "(" << (*points_it).x << ", " << (*points_it).y << ")\n";
			points_it++;
		}
		std::cout << "*****************************************\n";
	}
};

// This class represents images that uses Color class. We do not want this class
// to be accessed by any class other than Processor and HSV_Tracker.
class Image {
private:
	friend class Processor;
	friend class HSV_Tracker;
	
// Image matrix	
	Mat image;
	
	Image() = default;
	
	Image(const String &fileName) {
		loadImage(fileName);
	}
	
	Mat &getImage() {
		return image;
	}
	
	void setImage(Mat &image) {
		this->image = image;
	}
	
	void loadImage(const String &fileName) {
	// Load source image and convert it to gray
		image = imread(fileName);
		if(image.empty()) {
			std::cout << "Error: Could not read image file\n";
			exit(-1);
		}
	}
	
	void convertBGR2HSV() {
		convertBGR2HSV(*this);
	}

	void convertBGR2HSV(Image &newImage) {
		Mat new_image;
		cvtColor(image, new_image, CV_BGR2HSV);//COLOR_BGR2HSV);
		newImage.setImage(new_image);
	}
	
	void displayImage(const String &imageName) const {
	//Pop a window to display the image
		namedWindow(imageName , WINDOW_AUTOSIZE);
		imshow(imageName, image);
	}
	
	void getMaskedImage(Image &masked_image, Color &color) {
		Scalar lowVal = Scalar(color.getLowH(), color.getLowS(), color.getLowV());
		Scalar highVal = Scalar(color.getHighH(), color.getHighS(), color.getHighV());
		Mat newImage;
		inRange(image, lowVal, highVal, newImage);
		masked_image.setImage(newImage);
	}
	
	void maskImage(Color &color) {
		getMaskedImage(*this, color);
	}

	void findContoursAndSort(std::vector<std::vector<Point>> &contours, 
											 std::vector<Vec4i> &hierarchy) {
		findContours(image, contours, hierarchy, RETR_EXTERNAL, 
										CHAIN_APPROX_SIMPLE, Point(0, 0));
		std::sort(contours.begin(), contours.end(), 
				[this](std::vector<Point> a, std::vector<Point> b) {
					return compareContourAreas(a, b); 
				});
	}
	
	void drawImageContours(Image &newImage, std::vector<std::vector<Point>> &contours, 
														std::vector<Vec4i> &hierarchy) {
	// Draw contours on an image with black background
		Mat new_image = Mat::zeros(image.size(), CV_64F);
		unsigned long i = 0;
		while(i != contours.size()) {
			drawContours(new_image, contours, i, (150, 50, 25), 2, 8, hierarchy, 0, Point());
			i++;
		}
		newImage.setImage(new_image);
	}
	
	void drawImageContours(std::vector<std::vector<Point>> &contours,
													  std::vector<Vec4i> &hierarchy) {
		 drawImageContours(*this, contours, hierarchy); 
	}
	
	void resizeImage(Image &newImage, Size outputSize = Size(), double xScale = 0, 
								double yScale = 0, int interpolation = INTER_LINEAR) {
		Mat new_image;
		resize(image, new_image, outputSize, xScale, yScale, interpolation);
		newImage.setImage(new_image);
	}
	
	void resizeImage(Size outputSize = Size(), double xScale = 0, double yScale = 0, 
			                                       int interpolation = INTER_LINEAR) {
		resizeImage(*this, outputSize, xScale, yScale, interpolation);
	}
	
	bool compareContourAreas(std::vector<Point> contour1, std::vector<Point> contour2) {
		return (fabs(contourArea(Mat(contour1))) < fabs(contourArea(Mat(contour2))));
	}
};

// Most of the functions of Processor are private so outside functions or 
// classes cannot directly use it. If any function that involves interaction
// with the front-end could be added under public acess-modifier.
class Processor {
// Wall list
	std::vector<Wall *> wallList;
	
// List of panels
	std::vector<Panel> panelList;
	
// List of valid colors
	std::vector<Color> validColorList;

// Map to map names of colors and HSV values to see if colors are valid
	std::map<const String, bool> colorNameMap;
	std::map<const std::vector<uint8_t>, bool> HSVMap;
	
// Map color names to HSV values
	std::map<const String, std::vector<uint8_t>> colorNameHSVMap;
public:
	Processor(const String &imageName, double scale = 0.1, double noiseThreshold = 10) {
	// Get the image
		Image image(imageName);
		image.convertBGR2HSV();
		
	// List of avaliable panels 
		addPanel(2, 1);
		addPanel(1.5, 1);
		addPanel(0.5, 1);
		
	// WARNING: A few colors are hard coded. Make sure these colors work
		addColor(72, 180,72, 255, 0, 255, "Blue");   //Blue
		addColor(18, 56, 0, 255, 0, 255, "Yellow");  // Yellow
		
	// Start detecting walls 	
		std::vector<Color>::iterator it = validColorList.begin();
		while(it != validColorList.end()) {
		// Get masked images
			Image masked_image;
			image.getMaskedImage(masked_image, *it);
			masked_image.displayImage("Masked Image");
			
		// Find contours of the given color	
			std::vector<std::vector<Point>> contours;
			std::vector<Vec4i> hierarchy;
			masked_image.findContoursAndSort(contours, hierarchy);
		
		// Box the contours found and get wall cordinates
			boxContours(contours, *it);
			waitKey(10000);
			it++;
		}
		
	// Filter out the noise	
		filterNoise(scale, noiseThreshold);
		
	// Compute panels needed for given wall layout
		printAvailablePanels();
		computeNumPanelsBestFit();
		
	// Print Walls and panels
		//printWallList();
	}
	
// Get data collected by Processor
	const std::vector<Wall *> &getWallList() const {
		return wallList;
	}
	
// Print panel list
	void printAvailablePanels() {
		std::cout << "||||||||||||||| PRINTING AVAILABLE PANELS |||||||||||||||||\n";
		std::vector<Panel>::iterator it = panelList.begin();
		while(it != panelList.end()) {
			std::cout << "+++++++++++++ PRINTING PANEL INFO ++++++++++++++\n";
			std::cout << "Panel Width: " << (*it).getWidth() << "\n";
			std::cout << "Panel Height: " << (*it).getHeight() << "\n";
			std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++\n";
			it++;
		}
		std::cout << "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n";
	}
	
// Print wall list
	void printWallList() {
		std::cout << "===================== PRINTING WALL LIST ===================\n";
		std::vector<Wall *>::iterator it = wallList.begin();
		while(it != wallList.end()) {
			(*it)->printWallInfo();
			it++;
		}
		std::cout << "============================================================\n";
	}
	
// Print valid color list
	void printValidColorList() {
		std::cout << "-------------------- PRINTING COLOR LIST --------------------\n";
		std::vector<Color>::iterator it = validColorList.begin();
		while(it != validColorList.end()) {
			std::cout << (*it).getColor() << "\n";
			it++;
		}
		std::cout << "-------------------------------------------------------------\n";
	}
private:
// API to add panels to available panel list
	void addPanel(double width, double height) {
		Panel panel(width, height);
		panelList.push_back(panel);
	}
	
// API to add colors to valid colors list
	void addColor(uint8_t lowH, uint8_t highH, uint8_t lowS, uint8_t highS,
				  uint8_t lowV, uint8_t highV, const String &colorName) {
	// Build Color info	
		Color color(lowH, highH, lowS, highS, lowV, highV, colorName);
		
	// Add the color to the list	
		validColorList.push_back(color);
		
	// Add the color name to the valid color map	
		colorNameMap[colorName] = true;
		
	// Add HSV values to map	
		std::vector<uint8_t> vect;
		vect.push_back(lowH);
		vect.push_back(highH);
		vect.push_back(lowS);
		vect.push_back(highS);
		vect.push_back(lowV);
		vect.push_back(highV);
		HSVMap[vect] = true;
		
	// Map color name with HSV values
		colorNameHSVMap[colorName] = vect;
	}

	void boxContours(std::vector<std::vector<Point>> &contours, const Color &color) {
	// Store the contour rectangles in an array	
		std::vector<RotatedRect> minRect(contours.size());	
		unsigned long i = 0;
		while(i != contours.size()) {
			minRect[i] = minAreaRect(Mat(contours[i]));
			
		// Allocate wall
			Wall *wall = allocateWall(color);
			
		// Get the rectangle points and add it to the allocated wall
			Point2f rect_points[4];
			minRect[i].points(rect_points);
			std::vector<Point2f> vect;
			vect.push_back(rect_points[0]);
			vect.push_back(rect_points[1]);
			vect.push_back(rect_points[2]);
			vect.push_back(rect_points[3]);
			wall->setRectPoints(vect);
			
		// Add the wall to wall list
			wallList.push_back(wall);
			
			i++;
		}
	}
	
	Wall *allocateWall(const Color &color) {
		Wall *wall = new Wall(color);
		if(!wall) {
			std::cout << "Error in allocating wall.\n";
			exit(-1);
		}
		return wall;
	}
	
	Wall *allocateWall(double length = 0, double width = 0, double height = 0) {
		Wall *wall = new Wall(length, width, height);
		if(!wall) {
			std::cout << "Error in allocating wall.\n";
			exit(-1);
		}
		return wall;
	}
	
	Panel *allocatePanel(double width = 0, double height = 0) {
		Panel *panel = new Panel(width, height);
		if(!panel) {
			std::cout << "Error in allocating panel.\n";
			exit(-1);
		}
		return panel;
	}

	void computeNumPanelsBestFit() {
	// Iterate over the wall list and compute the number of panels needed
		std::vector<Wall *>::iterator wall = wallList.begin();
		while(wall != wallList.end()) {
		// Check if any panel type fits perfectly with the half wall of given length. 
		// If not, we keep track of error and panel width the lowest leftover wall space
		// to get the "best" panel. Put half panels in vector too.
			double wallLength = ((*wall)->getLength()) / 2;
			std::cout << "****************************************\n";
			std::cout << "WALL LENGTH: " << 2 * wallLength << "\n";
			double extraSpace = wallLength;
			double optPanelWidth = 0;
			double optPanelWidth2 = 0;
			double smallerExtraSpace;
			int zeroSpaceFitFound = -1;
			std::vector<Panel> panelVect = panelList;
			std::vector<Panel>::iterator panel = panelList.begin();
			while(panel != panelList.end()) {
				Panel temp_panel(((*panel).getWidth()) / 2);
				panelVect.push_back(temp_panel);
				panel++;
			}
			panel = panelVect.begin();
			std::vector<double> panelsAdded;
			while(panel != panelVect.end()) {
				double panelWidth = panel->getWidth();
				if(wallLength >= panelWidth) {
					if(zeroSpaceFitFound == -1)
						zeroSpaceFitFound = 0;
					uint32_t numPanels = wallLength / panelWidth;
					double spaceLeft = wallLength - (numPanels * panelWidth);
					std::cout << "PANEL WIDTH: " << panelWidth << "\n";
					std::cout << "SPACE LEFT: " << spaceLeft << "\n";
					if(!spaceLeft) {
					// Perfect match found! Allocate panel. But before that, check if
					// its a half panel.
						zeroSpaceFitFound = 1;
						std::vector<Panel>::iterator temp_panel = panelList.begin();
						while(temp_panel != panelList.end()) {
							if(panelWidth == (*temp_panel).getWidth()) {
								Panel *panel = allocatePanel(panelWidth);
								numPanels = wallLength / panelWidth;
								(*wall)->addBestFitPanels(panel, 2 * numPanels);
								std::cout << "NUM PANELS: " << 2 * numPanels << "\n";
								panelsAdded.push_back(panelWidth);
								goto next_panel;
							}
							temp_panel++;
						}
						
					// Check if the full panel corresponding to the full panel has already
					// been added to the wall. If yes, we can move on.
						panelWidth *= 2;
						if(find(panelsAdded.begin(), panelsAdded.end(), 
											panelWidth) == panelsAdded.end()) {
							Panel *panel = allocatePanel(panelWidth);
							numPanels = wallLength / panelWidth;
							(*wall)->addBestFitPanels(panel, (2 * numPanels) + 1);
							std::cout << "NUM PANELS: " << (2 * numPanels) + 1 << "\n";	
						}
						goto next_panel;
					}
					
					std::vector<Panel>::iterator temp_panel = panelList.begin();
					while(temp_panel != panelList.end()) {
						if(panelWidth == (*temp_panel).getWidth()) {
						// Get the lowest space left over
							if(spaceLeft < extraSpace) {
								extraSpace = spaceLeft;
								optPanelWidth = panelWidth;
								std::cout << "EXTRA\n";
							}
						
						// Check if the space left can be filled up by some other panels
							std::vector<double> panelsAdded2;
							Panel *optPanel = nullptr;
							smallerExtraSpace = extraSpace;
							std::vector<Panel>::iterator panel_it = panelVect.begin();
							while(panel_it != panelVect.end()) {
								double panelWidth = panel_it->getWidth();
								if(extraSpace >= panelWidth) {
									uint32_t numPanels = extraSpace / panelWidth;
									double spaceLeft = extraSpace - (numPanels * panelWidth);
									std::cout << "--PANEL WIDTH: " << panelWidth << "\n";
									std::cout << "--SPACE LEFT: " << spaceLeft << "\n";
									if(!spaceLeft) {
									// Perfect match found! Allocate panel. But before that, 
									// check if its a half panel.
										zeroSpaceFitFound = 1;
										std::vector<Panel>::iterator temp_panel = panelList.begin();
										while(temp_panel != panelList.end()) {
											if(panelWidth == (*temp_panel).getWidth()) {
											// Add the panel that we found fits first
												if(!optPanel) {
													std::cout << "--OPT PANEL WIDTH: " << optPanelWidth << "\n";
													optPanel = allocatePanel(optPanelWidth);
													numPanels = wallLength / optPanelWidth;
													(*wall)->addBestFitPanels(optPanel, 2 * numPanels);
													std::cout << "--NUM PANELS: " << 2 * numPanels << "\n";
												}
												
											// Alocate the rest of panels	
												Panel *panel = allocatePanel(panelWidth);
												numPanels = extraSpace / panelWidth;
												(*wall)->addBestFitPanels(panel, 2 * numPanels);
												std::cout << "--NUM PANELS: " << 2 * numPanels << "\n";
												panelsAdded2.push_back(panelWidth);
												goto next_panel_in;
											}
											temp_panel++;
										}
										panelWidth *= 2;
										if(find(panelsAdded2.begin(), panelsAdded2.end(), 
																panelWidth) == panelsAdded2.end()) {
										// Add the panel that we found fits first
											if(!optPanel) {
												std::cout << "--OPT PANEL WIDTH: " << optPanelWidth << "\n";
												optPanel = allocatePanel(optPanelWidth);
												numPanels = wallLength / optPanelWidth;
												(*wall)->addBestFitPanels(optPanel, 2 * numPanels);	
												std::cout << "--NUM PANELS: " << 2 * numPanels << "\n";
											}
											
											Panel *panel = allocatePanel(panelWidth);
											numPanels = extraSpace / panelWidth;
											(*wall)->addBestFitPanels(panel, (2 * numPanels) + 1);
											std::cout << "--NUM PANELS: " << (2 * numPanels) + 1 << "\n";
										}
										goto next_panel_in;
									}
									std::vector<Panel>::iterator temp_panel = panelList.begin();
									while(temp_panel != panelList.end()) {
										if(panelWidth == (*temp_panel).getWidth()) {
											if(spaceLeft < smallerExtraSpace) {
												smallerExtraSpace = spaceLeft;
												optPanelWidth2 = panelWidth;
												std::cout << "--EXTRA\n";
											}
											break;
										}
										temp_panel++;
									}
								}
							next_panel_in:
								panel_it++;
							}
							
						// If the extra space after panelizing small space is less than what we
						// got earlier, this is a better solution.
							if(smallerExtraSpace < extraSpace)
								extraSpace = smallerExtraSpace;
							
							break;
						}
						temp_panel++;
					}
				}
			next_panel:
				panel++;
			}
			
		// If the best fit with zero space left is not found deal with the panels that work	
			if(!zeroSpaceFitFound && optPanelWidth) {
				Panel *panel = allocatePanel(optPanelWidth);
				uint32_t numPanels = wallLength / optPanelWidth;
				(*wall)->addBestFitPanels(panel, 2 * numPanels);
				std::cout << "zPANEL WIDTH: " << optPanelWidth << "\n";
				std::cout << "zNUM PANELS: " << 2 * numPanels << "\n";
				
			// Get the extra space left after panelizing using "best" panel. 
				extraSpace = wallLength - ((uint32_t)(wallLength / optPanelWidth) * optPanelWidth);
				
			// Check if there is a second panel type that needs to be added
				if(optPanelWidth2) {
					Panel *panel = allocatePanel(optPanelWidth2);
					uint32_t numPanels = extraSpace / optPanelWidth2;
					(*wall)->addBestFitPanels(panel, 2 * numPanels);
					std::cout << "zPANEL WIDTH2: " << optPanelWidth2 << "\n";
					std::cout << "zNUM PANELS: " << 2 * numPanels << "\n";
					
				// Recompute the extra space	
					extraSpace = extraSpace - 
									((uint32_t)(extraSpace / optPanelWidth2) * optPanelWidth2);
				}
				std::cout << "EXTRA SPACE LEFT: " << 2 * extraSpace << "\n";
			} else {
				goto next_wall;
			}
				
		// Try to fit panels in the left over space. Half panels would do as well.
			zeroSpaceFitFound = -1;
			panelsAdded.clear();
			smallerExtraSpace = extraSpace;
			optPanelWidth = 0;
			panel = panelVect.begin();
			while(panel != panelVect.end()) {
				double panelWidth = panel->getWidth();
				if(extraSpace >= panelWidth) {
					if(zeroSpaceFitFound == -1)
						zeroSpaceFitFound = 0;
					uint32_t numPanels = extraSpace / panelWidth;
					double spaceLeft = extraSpace - (numPanels * panelWidth);
					std::cout << "--PANEL WIDTH: " << panelWidth << "\n";
					std::cout << "--SPACE LEFT: " << spaceLeft << "\n";
					if(!spaceLeft) {
					// Perfect match found! Allocate panel. But before that, check if
					// its a half panel.
						zeroSpaceFitFound = 1;
						std::vector<Panel>::iterator temp_panel = panelList.begin();
						while(temp_panel != panelList.end()) {
							if(panelWidth == (*temp_panel).getWidth()) {
								Panel *panel = allocatePanel(panelWidth);
								numPanels = extraSpace / panelWidth;
								(*wall)->addBestFitPanels(panel, 2 * numPanels);
								std::cout << "--NUM PANELS: " << 2 * numPanels << "\n";
								panelsAdded.push_back(panelWidth);
								goto next_panel2;
							}
							temp_panel++;
						}
						panelWidth *= 2;
						if(find(panelsAdded.begin(), panelsAdded.end(), 
												panelWidth) == panelsAdded.end()) {
							Panel *panel = allocatePanel(panelWidth);
							numPanels = extraSpace / panelWidth;
							(*wall)->addBestFitPanels(panel, (2 * numPanels) + 1);
							std::cout << "--NUM PANELS: " << (2 * numPanels) + 1 << "\n";
						}
						goto next_panel2;
					}
					if(spaceLeft < smallerExtraSpace) {
						smallerExtraSpace = spaceLeft;
						optPanelWidth = panelWidth;
						std::cout << "--EXTRA\n";
					}	
				}
			next_panel2:
				panel++;
			}
			
		// If the best fit with zero space left is not found deal with the panels that work	
			if(!zeroSpaceFitFound && optPanelWidth) {
			// Now, add the rest of the panels
				Panel *panel = allocatePanel(optPanelWidth);
				uint32_t numPanels = extraSpace / optPanelWidth;
				(*wall)->addBestFitPanels(panel, 2 * numPanels);
				std::cout << "--zPANEL WIDTH: " << optPanelWidth << "\n";
				std::cout << "--zNUM PANELS: " << 2 * numPanels << "\n";
			}
	
		next_wall:
			std::cout << "***********************************************\n";
			wall++;
		}
	}

	void filterNoise(double scale, double noiseThreshold) {
	// Filter noise. Avoid anything that does not have a proper, significant size
	// Traverse the object list and filter the bugger noise out. For rectangles,
	// there are only three unique distances. We pick two shortest lengths, ignoring
	// the longest one which is most likely to be the diagonal.
		std::vector<Wall *> tempList = wallList;
		wallList.clear();
		std::vector<Wall *>::iterator wall_it = tempList.begin();
		while(wall_it != tempList.end()) {
		// Walls are treated as rectangles. Get two rectangle sides	and check if the ratio of
		// sides is more than the threshold. If it is, put the wall in the wall list and save 
		// the scaled lengths of the walls.
			double side1 = euclideanDistance((*wall_it)->getVertex(0), (*wall_it)->getVertex(1));
			double side2 = euclideanDistance((*wall_it)->getVertex(1), (*wall_it)->getVertex(2));
			if(side1 && side2) {
				if(side1 > side2) {
					if(side1 / side2 >= noiseThreshold) {
						wallList.push_back(*wall_it);
						(*wall_it)->setLength(side1 * scale);
					}
				} else {
					if(side2 / side1 >= noiseThreshold) {
						wallList.push_back(*wall_it);
						(*wall_it)->setLength(side2 * scale);
					}
				}
			}
			wall_it++;
		}
	}

// Calculate the euclidean distance between the points
	double euclideanDistance(Point2f point1, Point2f point2) const {
		return norm(point1 - point2);
	}

// API to check the validity of color	
	bool isValidColor(uint8_t lowH, uint8_t highH, uint8_t lowS, uint8_t highS,
					  uint8_t lowV, uint8_t highV) {
		std::vector<uint8_t> vect;
		vect.push_back(lowH);
		vect.push_back(highH);
		vect.push_back(lowS);
		vect.push_back(highS);
		vect.push_back(lowV);
		vect.push_back(highV);
		return HSVMap[vect];
	}
	
	bool isValidColor(std::vector<uint8_t> hsvVect) {
		if(hsvVect.size() != 6) {
			std::cout << "Invalid color\n";
			exit(-1);
		}
		return HSVMap[hsvVect];
	}
	
	bool isValidColor(String colorName) {
		std::vector<Color>::iterator it = validColorList.begin();
		while(it != validColorList.end()) {
			if(colorName == (*it).getColor())
				return true;
			it++;
		}
		return false;
	}
};

// Class for making the HSV tracker to pop up and enable a user to adjust HSV values
class HSV_Tracker {
public:	
	HSV_Tracker(const String &imageName, const String &winName = "HSV_Tracker") {
	// Create a window with track bars on it	
		createTrackbarWindow(winName);

	// Analyse image
		Image image(imageName);
		image.convertBGR2HSV();
		while(1) {
		// Get the positions of the trackbars
			int lowH = getTrackbarPos("lowH", winName);
			int highH = getTrackbarPos("highH", winName);
			int lowS = getTrackbarPos("lowS", winName);
			int highS = getTrackbarPos("highS", winName);
			int lowV = getTrackbarPos("lowV", winName);
			int highV = getTrackbarPos("highV", winName);
			Color color(lowH, highH, lowS, highS, lowV, highV);
			Image newImage;
			image.getMaskedImage(newImage, color);
			newImage.resizeImage(Size(), 0.75, 0.75);
			newImage.displayImage(winName);
			waitKey(1000);  //large wait time to avoid freezing
		}
	}
	
private:
	void createTrackbarWindow(const String &windowName) const {
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
};

} // namespace Panelization
