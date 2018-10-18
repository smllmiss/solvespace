//==== Color + Object Detection + Panelization code =====//
//
//-------------------------------------------------------//
// Author: Akash Kothari  <akothar3@ncsu.edu>
//-------------------------------------------------------//
#ifndef PANELIZE__H__
#define PANELIZE__H__
#include <iostream>
#include <vector>
#include <cstdint>
//#include <dirent.h>
#include <fstream>
#include <string>
#include <stdio.h>
#include <inttypes.h>
#include <map>
#include <sstream>

namespace Panelization {
// Generic class to represent colors. But it is abstract i.e. you are not
// to make a new Color object.
class Color {
private:
// HSV values that represent colors
	uint8_t hue = 0;
	uint8_t saturation = 0;
	uint8_t brightness = 0;

	friend class Wall;

// Constructors are private restricting use of this class
	Color() = default;

	Color(uint8_t h_val, uint8_t s_val, uint8_t v_val) {
		setColor(h_val, s_val, v_val);
	}

public:
// Get color info
	uint8_t getH() const {
		return hue;
	}

	uint8_t getS() const {
		return saturation;
	}

	uint8_t getV() const {
		return brightness;
	}

	std::vector<uint8_t> getHSV_Vector() const {
		std::vector<uint8_t> vect;
		vect.push_back(hue);
		vect.push_back(saturation);
		vect.push_back(brightness);
		return vect;
	}

private:
// Set colors
	void setColor(uint8_t hue, uint8_t saturation, uint8_t brightness) {
		this->hue = hue;
		this->saturation = saturation;
		this->brightness = brightness;
	}

// Order of parameters: LOW_H, HIGH_H, LOW_S, HIGH_S, LOW_V, HIGH_V
	void setColor(const std::vector<uint8_t> &hsv) {
		if(hsv.size() != 3) {
			std::cout << "Invalid HSV vector\n";
			exit(-1);
		}
		hue = hsv[0];
		saturation = hsv[1];
		brightness = hsv[2];
	}

	void printColor() {
		printf("HUE: %" PRIu8"\n", hue);
		printf("SATURATION: %" PRIu8"\n", saturation);
		printf("BRIGHTNESS: %" PRIu8"\n", brightness);
	}
	
	virtual void abstractColor() const = 0;
};

// Panel Object
class Panel {
private:
// Panel Dimensions
	double width = 0;
	double height = 0;

// Number of panels
	uint32_t numPanels = 0;

	friend class Wall;
	friend class Processor;

// Constrctors are private in order to restrict use of this class
	Panel() = default;

	Panel(double panelWidth, double panelHeight = 0) {
		width = panelWidth;
		height = panelHeight;
	}

	Panel(const Panel &) = delete;

	Panel operator =(const Panel &) = delete;

public:
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

private:
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
private:
// Wall dimensions
	double length = 0;
	double width = 0;
	double height = 0;

// Vector of vector of panels that best fit the wall
	std::vector<std::vector<Panel *> *> bestFitPanelList;

// We define the "head" of the panel list as the outermost panel for a wall.
// We map the head of the panels to the pointer to the panel list.
	std::map<const Panel *, std::vector<Panel *> *> panelHeadListMap;

// Cordinates of the wall in the print
	std::vector<double> rectSides;

	friend class Processor;

// Constructors need to be private to restrict how this class is used
	Wall() = default;

	Wall(double length, double width, double height = 0) {
		this->length = length;
		this->width = width;
		this->height = height;
	}

	Wall(uint8_t hue, uint8_t saturation, uint8_t brightness) {
		this->setColor(hue, saturation, brightness);
	}

	Wall(const Wall &) = delete;

	Wall operator = (const Wall &) = delete;

public:
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

	std::vector<std::vector<Panel *> *> &getBestFitPanelList() {
		return bestFitPanelList;
	}

private:
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

	void setRectSides(double side1, double side2) {
		rectSides.clear();
		rectSides.push_back(side1);
		rectSides.push_back(side2);
	}
	
	std::vector<double> &getRectSides() {
		return rectSides;
	}
	
	void addBestFitPanel(Panel *panel, uint32_t numPanels = 1) {
	// This is a panel for a new list
		std::vector<Panel *> *panelList = new std::vector<Panel *>();
		panelList->push_back(panel);
		bestFitPanelList.push_back(panelList);
		panelHeadListMap[panel] = panelList;
		panel->incrementNumPanels(numPanels);
	}

	void addBestFitPanel(Panel *panel, std::vector<Panel*> &headPanelList,
													uint32_t numPanels = 1) {
		if(headPanelList.empty()) {
		// This is a panel for a new list
			std::vector<Panel *> *panelList = new std::vector<Panel *>();
			panelList->push_back(panel);
			bestFitPanelList.push_back(panelList);

			panelHeadListMap[panel] = panelList;
			headPanelList.push_back(panel);
		} else {
		// Access the map to get the panel list corresponding to the given head panel
		// and add the new panel to the list.
			std::vector<Panel *>::iterator it = headPanelList.begin();
			while(it != headPanelList.end()) {
				(*panelHeadListMap[*it]).push_back(panel);
				it++;
			}
		}
		panel->incrementNumPanels(numPanels);
	}

// Make sure there is no repetition of panels in the panel list	
	void mergePanelsInPanelLists() {
		std::vector<std::vector<Panel *> *>::iterator panelList = bestFitPanelList.begin();
		while(panelList != bestFitPanelList.end()) {
		// Iterate over the panels to see if the panels in a list are repetitive. If they are
		// merge them.
			if((*panelList)->size() != 1) {
				std::vector<Panel *>::iterator panel = (*panelList)->begin();
				while(panel != (*panelList)->end()) {
						std::vector<Panel *>::iterator temp_panel = (*panelList)->begin();
						while(temp_panel != (*panelList)->end()) {
							if(temp_panel != panel) {
								if((*temp_panel)->getWidth() == (*panel)->getWidth()
								&& (*temp_panel)->getHeight() == (*panel)->getHeight()) {
								// Merge panels
									(*panel)->incrementNumPanels((*temp_panel)->getNumPanels());
									std::vector<Panel *>::iterator it = temp_panel - 1;
									delete *temp_panel;
									(*panelList)->erase(temp_panel);
									temp_panel = it;
								}
							}
							temp_panel++;
						}
					panel++;
				}
			}
			panelList++;
		}
	}
	
// This class is not abstract
	void abstractColor() const {}

// Verify wall info is correct
	bool wallInfoIsSane() {
		//return true;
		if(rectSides.size() != 2)
			return false;
		if(!length)
			return false;
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
		printColor();

	// Print Best fit Panel info
		std::vector<std::vector<Panel *> *>::iterator panel_list_it = bestFitPanelList.begin();
		while(panel_list_it != bestFitPanelList.end()) {
			std::cout << "-------------------- PANEL LIST ----------------------\n";
			std::vector<Panel *>::iterator panel_it = (*panel_list_it)->begin();
			while(panel_it != (*panel_list_it)->end()) {
				(*panel_it)->printPanelInfo();
				panel_it++;
			}
			std::cout << "------------------------------------------------------\n";
			panel_list_it++;
		}
		std::cout << "*****************************************\n";
	}
};

// Most of the functions of Processor are private so outside functions or
// classes cannot directly use it. If any function that involves interaction
// with the front-end could be added under public acess-modifier.
class Processor {
private:
// Wall list
	static std::vector<Wall *> wallList;

	struct PanelInfo {
	private:
	// Panel Dimensions
		double width = 0;
		double height = 0;

		PanelInfo(double panelWidth = 0, double panelHeight = 0) {
			width = panelWidth;
			height = panelHeight;
		}

		friend class Processor;
	};

// List of panels
	static std::vector<PanelInfo> panelList;

public:
	Processor() = default;
	
	Processor(const std::string &imageName,
			  const std::string &panelListFileName, double scale, 
			  double noiseThreshold = 5,
			  const std::string &outputFileName = std::string()) {
	// Filter out the noise
		std::cout << "FILTERING NOISE\n";
		filterNoise(scale, noiseThreshold);

	// Compute panels needed for given wall layout
		printAvailablePanels();
		computeNumPanelsBestFit();
		//mergeRepetitivePanels();

	// Write data to output file
		writeToFile(outputFileName, imageName);

	// Print Walls and panels
		printWallList();
	}

// Get data collected by Processor
	const std::vector<Wall *> &getWallList() const {
		return wallList;
	}

// Print panel list
	void printAvailablePanels() {
		std::cout << "||||||||||||||| PRINTING AVAILABLE PANELS |||||||||||||||||\n";
		std::vector<PanelInfo>::iterator it = panelList.begin();
		while(it != panelList.end()) {
			std::cout << "+++++++++++++ PRINTING PANEL INFO ++++++++++++++\n";
			std::cout << "Panel Width: " << (*it).width << "\n";
			std::cout << "Panel Height: " << (*it).height << "\n";
			std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++\n";
			it++;
		}
		std::cout << "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n";
	}

// Print wall list
	void printWallList() {
		std::cout << "=================== PRINTING WALL LIST ===================\n";
		std::vector<Wall *>::iterator it = wallList.begin();
		while(it != wallList.end()) {
			(*it)->printWallInfo();
			it++;
		}
		std::cout << "==========================================================\n";
	}

private:
// API to add panels to available panel list
	void addPanel(double width, double height = 0) {
		PanelInfo panel(width, height);
		panelList.push_back(panel);
	}

	void getPanels(const std::string panelListFileName) {
	// Check if the panelListFileName is not valid
		if(panelListFileName.empty()) {
			std::cout << "Panel List File Invalid\n";
			exit(-1);
		}

	// Exit if the panelListName does not have a .csv extension
		std::string fileName = panelListFileName;
		std::string::iterator it = fileName.begin();
		std::string::iterator dot_it = it;
		while(it != fileName.end()) {
			if(*it == '.')
				dot_it = it;
			it++;
		}

	// Check the extension
		it = dot_it;
		if(*(it + 1) != 'c'
		|| *(it + 2) != 's'
		|| *(it + 3) != 'v') {
			std::cout << "Panel List File Invalid\n";
			exit(-1);
		}

	// File is sane. Read the panel list and collect the information. Open file.
		std::ifstream infile(panelListFileName);
		std::string line;
		uint32_t numCommas = 0;
		bool columnFound = false;
		while(std::getline(infile, line)) {
			if(columnFound == true) {
			// Read the line to get the panel width
				std::stringstream stream(line);
				double panelWidth = 0;
				uint32_t col = 0;	
				while(col++ <= numCommas) {
					stream >> panelWidth;
					std::cout << "READ PANEL WIDTH: " << panelWidth << "\n";
				}
					
			// Add panel to the panel list	
				addPanel(panelWidth);
				continue;
			}
			
		// Search for "Panel Width", "PanelWidth", "Panel width", "panel width", 
		// "PANEL WIDTH", "PANEL_WIDTH", "panelWidth", or "panel_width". 
		// Once found -- if found, we count the number of commas precede the string.
			size_t pos = line.find("Panel Width");
			if(pos != std::string::npos) {
				numCommas = numCharBeforePos(',', pos, line);
				columnFound = true;
				continue;
			}
			pos = line.find("PanelWidth");
			if(pos != std::string::npos) {
				numCommas = numCharBeforePos(',', pos, line);
				columnFound = true;
				continue;
			}
			pos = line.find("Panel width");
			if(pos != std::string::npos) {
				numCommas = numCharBeforePos(',', pos, line);
				columnFound = true;
				continue;
			}
			pos = line.find("panel width");
			if(pos != std::string::npos) {
				numCommas = numCharBeforePos(',', pos, line);
				columnFound = true;
				continue;
			}
			pos = line.find("PANEL WIDTH");
			if(pos != std::string::npos) {
				numCommas = numCharBeforePos(',', pos, line);
				columnFound = true;
				continue;
			}
			pos = line.find("PANEL_WIDTH");
			if(pos != std::string::npos) {
				numCommas = numCharBeforePos(',', pos, line);
				columnFound = true;
				continue;
			}
			pos = line.find("panel_width");
			if(pos != std::string::npos) {
				numCommas = numCharBeforePos(',', pos, line);
				columnFound = true;
				continue;
			}
			pos = line.find("panelWidth");
			if(pos != std::string::npos) {
				numCommas = numCharBeforePos(',', pos, line);
				columnFound = true;
				continue;
			}
		}
		if(columnFound == false) {
			std::cout << "Error: No panels specified in file or the panel"
					  << "column not named as expected.\n";
			exit(-1);
		}
	}
	
	uint32_t numCharBeforePos(char search_char, size_t stop_pos, 
							  const std::string &line) const {
		uint32_t numCommas = 0;
		size_t index = 0;
		while(index != stop_pos) {
			if(line[index] == ',')
				numCommas++;
			index++;
		}
		return numCommas;
	}

	static Wall *allocateWall(uint8_t hue, uint8_t saturation, 
										   uint8_t brightness) {
		Wall *wall = new Wall(hue, saturation, brightness);
		if(!wall) {
			std::cout << "Error in allocating wall.\n";
			exit(-1);
		}
		return wall;
	}

	static Wall *allocateWall(double length = 0, double width = 0,
												double height = 0) {
		Wall *wall = new Wall(length, width, height);
		if(!wall) {
			std::cout << "Error in allocating wall.\n";
			exit(-1);
		}
		return wall;
	}

	static Panel *allocatePanel(double width = 0, double height = 0) {
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
		// If not, we keep track of error and panel width the lowest leftover wall
		// space to get the "best" panel. Put half panels in vector too.
			double wallLength = ((*wall)->getLength()) / 2;
			std::cout << "****************************************\n";
			std::cout << "WALL LENGTH: " << 2 * wallLength << "\n";
			double extraSpace = wallLength;
			double smallerExtraSpace;
			int zeroSpaceFitFound = -1;
			std::vector<PanelInfo> panelVect = panelList;
			std::vector<PanelInfo>::iterator panel = panelList.begin();
			while(panel != panelList.end()) {
				PanelInfo temp_panel(((*panel).width / 2));
				panelVect.push_back(temp_panel);
				panel++;
			}
			panel = panelVect.begin();
			std::vector<Panel *> headPanelList;
			std::vector<Panel *> perfectFitPanelList;
			std::vector<std::vector<double>> panelsAdded;
			while(panel != panelVect.end()) {
				double panelWidth = panel->width;
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
						std::vector<PanelInfo>::iterator temp_panel = panelList.begin();
						while(temp_panel != panelList.end()) {
							if(panelWidth == (*temp_panel).width) {
								Panel *panel = allocatePanel(panelWidth);
								numPanels = 2 * (wallLength / panelWidth);
								(*wall)->addBestFitPanel(panel, numPanels);
								std::cout << "NUM PANELS: " << numPanels << "\n";
								headPanelList.push_back(panel);
								goto next_panel;
							}
							temp_panel++;
						}

					// Check if the full panel corresponding to the full panel has already
					// been added to the wall. If yes, we can move on.
						panelWidth *= 2;
						std::vector<Panel *>::iterator it = headPanelList.begin();
						while(it != headPanelList.end()) {
							if((*it)->getWidth() == panelWidth)
								goto next_panel;
							it++;
						}
						Panel *panel = allocatePanel(panelWidth);
						numPanels = ((uint32_t)(2 * (wallLength / panelWidth)) - 1) + 1;
						(*wall)->addBestFitPanel(panel, numPanels);
						std::cout << "NUM PANELS: " << numPanels << "\n";
						goto next_panel;
					}
					//std::cout << "BUGGER\n";
					std::vector<PanelInfo>::iterator temp_panel = panelList.begin();
					while(temp_panel != panelList.end()) {
						if(panelWidth == (*temp_panel).width) {
						// Get the lowest space left over
							std::cout << "EXTRA SPACE: " << extraSpace << "\n";
							if(spaceLeft == extraSpace) {
								//std::cout << "BUGGER\n";
								std::vector<double> temp_vect;
								temp_vect.push_back(spaceLeft);
								temp_vect.push_back(panelWidth);
								panelsAdded.push_back(temp_vect);
								extraSpace = spaceLeft;
								std::cout << "EXTRA\n";
								std::cout << "PANELS ADDED VECT SIZE: "
													<< panelsAdded.size() << "\n";
							} else {
								if(spaceLeft < extraSpace) {
									std::cout << "SPACE LEFT LESS THAN EXTRA SPACE\n";
									std::vector<std::vector<double>>::iterator it = panelsAdded.begin();
									if(it != panelsAdded.end()) {
										while(it != panelsAdded.end()) {
											if((*it)[0] == extraSpace) {
												(*it)[0] = spaceLeft;
												(*it)[1] = panelWidth;
											}
											it++;
										}
									} else {
										std::vector<double> temp_vect;
										temp_vect.push_back(spaceLeft);
										temp_vect.push_back(panelWidth);
										panelsAdded.push_back(temp_vect);
									}
									extraSpace = spaceLeft;
									std::cout << "ppEXTRA\n";
									std::cout << "PANELS ADDED VECT SIZE: "
														<< panelsAdded.size() << "\n";
								}
							}
							break;
						}
						temp_panel++;
					}
				}
			next_panel:
				panel++;
			}

		// If the best fit with zero space left is not found deal with the panels
		// that work.
			if(!zeroSpaceFitFound && !panelsAdded.empty()) {
				std::vector<std::vector<double>>::iterator it = panelsAdded.begin();
				while(it != panelsAdded.end()) {
					Panel *panel = allocatePanel((*it)[1]);
					uint32_t numPanels = 2 * (uint32_t)(wallLength / (*it)[1]);
					(*wall)->addBestFitPanel(panel, numPanels);
					std::cout << "zPANEL WIDTH: " <<  (*it)[1] << "\n";
					std::cout << "zNUM PANELS: " << numPanels << "\n";
					//extraSpace = wallLength - ((uint32_t)(wallLength / (*it)[1]) * (*it)[1]);

				// Since this is a head panel, we add this to the head panel list
					headPanelList.push_back(panel);
					it++;
				}
			} else {
				goto next_wall;
			}

		// Try to fit panels in the left over space. Half panels would do as well.
			//extraSpace = wallLength - ((uint32_t)(wallLength / optPanelWidth) * optPanelWidth);
			std::cout << "EXTRA SPACE LEFT: " << 2 * extraSpace << "\n";
			zeroSpaceFitFound = -1;
			panelsAdded.clear();
			smallerExtraSpace = extraSpace;
			panel = panelVect.begin();
			while(panel != panelVect.end()) {
				double panelWidth = panel->width;
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
						std::vector<PanelInfo>::iterator temp_panel = panelList.begin();
						while(temp_panel != panelList.end()) {
							if(panelWidth == (*temp_panel).width) {
								Panel *panel = allocatePanel(panelWidth);
								numPanels = 2 * (extraSpace / panelWidth);
								(*wall)->addBestFitPanel(panel, headPanelList, numPanels);
								std::cout << "--NUM PANELS: " << numPanels << "\n";
								perfectFitPanelList.push_back(panel);
								goto next_panel2;
							}
							temp_panel++;
						}
						panelWidth *= 2;
						std::vector<Panel *>::iterator it = perfectFitPanelList.begin();
						while(it != perfectFitPanelList.end()) {
							if((*it)->getWidth() == panelWidth)
								goto next_panel2;
							it++;
						}
						Panel *panel = allocatePanel(panelWidth);
						numPanels = ((uint32_t)(2 * (wallLength / panelWidth)) - 1) + 1;
						(*wall)->addBestFitPanel(panel, headPanelList, numPanels);
						std::cout << "--NUM PANELS: " << numPanels << "\n";
						goto next_panel2;
					}

					if(spaceLeft == smallerExtraSpace) {
						std::vector<double> temp_vect;
						temp_vect.push_back(spaceLeft);
						temp_vect.push_back(panelWidth);
						panelsAdded.push_back(temp_vect);
						smallerExtraSpace = spaceLeft;
						std::cout << "--EXTRA\n";
					} else {
						if(spaceLeft < smallerExtraSpace) {
							std::vector<std::vector<double>>::iterator it = panelsAdded.begin();
							if(it != panelsAdded.end()) {
								while(it != panelsAdded.end()) {
									if((*it)[0] == smallerExtraSpace) {
										(*it)[0] = spaceLeft;
										(*it)[1] = panelWidth;
									}
									it++;
								}
							} else {
								std::vector<double> temp_vect;
								temp_vect.push_back(spaceLeft);
								temp_vect.push_back(panelWidth);
								panelsAdded.push_back(temp_vect);
							}
							smallerExtraSpace = spaceLeft;
							std::cout << "--ppEXTRA\n";
						}
					}
				}
			next_panel2:
				panel++;
			}

		// If the best fit with zero space left is not found deal with the panels
		// that work.
			if(!zeroSpaceFitFound && !panelsAdded.empty()) {
				std::vector<std::vector<double>>::iterator it = panelsAdded.begin();
				while(it != panelsAdded.end()) {
					double panelWidth = (*it)[1];
					uint32_t numPanels;

				// Check if it is half panel
					std::vector<PanelInfo>::iterator panel_it = panelList.begin();
					while(panel_it != panelList.end()) {
						if(panel_it->width == panelWidth) {
							numPanels = 2 * (uint32_t)(extraSpace / panelWidth);
							goto allocate_panel;
						}
						panel_it++;
					}
					panelWidth *= 2;
					numPanels = ((uint32_t)(2 * (extraSpace / panelWidth)) - 1) + 1;

				allocate_panel:
					Panel *panel = allocatePanel(panelWidth);
					(*wall)->addBestFitPanel(panel, headPanelList, numPanels);
					std::cout << "--zPANEL WIDTH: " <<  (*it)[1] << "\n";
					std::cout << "--zNUM PANELS: " << numPanels << "\n";
					it++;
				}
			}

		next_wall:
			std::cout << "***********************************************\n";
			wall++;
		}
	}

	void filterNoise(double scale, double noiseThreshold) {
	// Filter noise. Avoid anything that does not have a proper, significant size
	// Traverse the object list and filter the bugger noise out. For rectangles,
	// there are only three unique distances. We pick two shortest lengths,
	// ignoring the longest one which is most likely to be the diagonal.
		std::vector<Wall *> tempList = wallList;
		wallList.clear();
		std::vector<Wall *>::iterator wall_it = tempList.begin();
		while(wall_it != tempList.end()) {
		// Walls are treated as rectangles. Get two rectangle sides	and check if the
		// ratio of sides is more than the threshold. If it is, put the wall in the
		// wall list and save the scaled lengths of the walls.
			double side1 = (*wall_it)->getRectSides()[0];
			double side2 = (*wall_it)->getRectSides()[1];
			if(side1 && side2) {
				if(side1 > side2) {
					if(side1 / side2 >= noiseThreshold) {
						if(side2 <= 1000 && side2 >= 5) {
							wallList.push_back(*wall_it);
							(*wall_it)->setLength(side1 * scale);
						}
						std::cout << " WALL SIDE WIDTH: " << side1 << "\n";
					} else {
					// Delete the wall
						delete *wall_it;
					}
				} else {
					if(side2 / side1 >= noiseThreshold) {
						//if(side1 <= 1000 && side1 >= 5) {
							wallList.push_back(*wall_it);
							(*wall_it)->setLength(side2 * scale);
						//}
						std::cout << " WALL SIDE WIDTH: " << side1 << "\n";
					} else {
					// Delete the wall	
						delete *wall_it;
					}
				}
			} else {
			// Delete the wall	
				delete *wall_it;
			}
			wall_it++;
		}
	}
	
// Truncate file truncate extension
	std::string truncateExtension(const std::string &fileName) {
		std::string name = fileName;
		std::string::iterator it = name.begin();
		std::string::iterator dot_it = name.begin();
		while(it != name.end()) {
			if(*it == '.')
				dot_it = it;
			it++;
		}

	// Erase image extension
		it = dot_it;
		while(it != name.end()) {
			name.erase(it);
			it = dot_it;
		}
		return name;
	}

// Write all the collected data to a file
	void writeToFile(const std::string &outputFileName,
									 const std::string &imageName) {
	// If the output file name has not be given, we use the image name to make
	// output file name.
		std::string output;
		if(outputFileName.empty()) {
			output = truncateExtension(imageName);

			std::cout << "TRUNCATED OUPUT FILE NAME: " << output << "\n";
		// Append new extension
			output.append(".csv");
		} else {
			output = outputFileName;
		}
		std::cout << "OUPUT FILE NAME: " << output << "\n";
	// Write to output file
		std::ofstream outputFile;
		outputFile.open(output);
		outputFile << "Wall Length (ft),Wall Width (ft),Wall Height (ft),"
						"Panel Width (ft),Number of Panels" << std::endl;
		std::vector<Wall *>::iterator wall = wallList.begin();
		while(wall != wallList.end()) {
		// Write wall dimensions
			std::ostringstream strs;
			std::string str;
			str.append("\"");
			strs << (*wall)->getLength();
			str.append(strs.str());
			str.append("\"");
			outputFile << str << ",";
			std::ostringstream strs2;
			str.clear();
			str.append("\"");
			strs2 << (*wall)->getWidth();
			str.append(strs2.str());
			str.append("\"");
			outputFile << str << ",";
			std::ostringstream strs3;
			str.clear();
			str.append("\"");
			strs3 << (*wall)->getHeight();
			str.append(strs3.str());
			str.append("\"");
			outputFile << str << ",";

			std::cout << "WALL DIMENSIONS ADDED\n";
		// List the panels
			std::vector<std::vector<Panel *> *>::iterator panel_list =
										((*wall)->getBestFitPanelList()).begin();
			while(panel_list != ((*wall)->getBestFitPanelList()).end()) {
				std::string str;
				str.append("\"");
				std::vector<Panel *>::iterator panel = (*panel_list)->begin();
				while(panel != (*panel_list)->end()) {
					std::ostringstream strs;
					strs << (*panel)->getWidth();
					str.append(strs.str());
					str.append(", ");
					panel++;
				}
				std::string::reverse_iterator rit = str.rbegin() + 1;
				std::cout << "LAST CHAR: " << *rit << "\n";
				*rit = '\"';
				outputFile << str;
				std::cout << "STRING: " << str << "\n";
				std::cout << "PANEL WIDTH ADDED\n";
				outputFile << ",";
				std::string str2;
				str2.append("\"");
				panel = (*panel_list)->begin();
				while(panel != (*panel_list)->end()) {
					std::ostringstream strs;
					strs << (*panel)->getNumPanels();
					str2.append(strs.str());
					str2.append(", ");
					panel++;
				}
				rit = str2.rbegin() + 1;
				std::cout << "LAST CHAR: " << *rit << "\n";
				*rit = '\"';
				outputFile << str2;
				std::cout << "STRING: " << str2 << "\n";
				std::cout << "PANEL NUM ADDED\n";
				outputFile << std::endl;
				panel_list++;
				if(panel_list != ((*wall)->getBestFitPanelList()).end()) {
					outputFile << ",,,";
				}
			}
			outputFile << std::endl;
			wall++;
		}

	// Done wrtiting to the file. Save the god damn thing.
		outputFile.close();
	}
	
// Merge necessary panels for the walls in the wall list
	void mergeRepetitivePanels() {
		std::vector<Wall *>::iterator wall = wallList.begin();
		while(wall != wallList.end()) {
			(*wall)->mergePanelsInPanelLists();
			wall++;
		}
	}

// This class is not abstract
	void abstractImage() const {}
};

std::vector<Wall *> Processor::wallList = std::vector<Wall *>();
std::vector<Processor::PanelInfo> Processor::panelList = 
							std::vector<Processor::PanelInfo>();
} // namespace Panelization

#endif
