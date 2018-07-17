/***********************************************************************
EarthquakeTool - Tool to create a circular or planar perturbation in the water 
table to mimic an earthquake.

This file is part of the Augmented Reality Sandbox (SARndbox).

The Augmented Reality Sandbox is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Augmented Reality Sandbox is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Augmented Reality Sandbox; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef EARTHQUAKETOOL_INCLUDED
#define EARTHQUAKETOOL_INCLUDED

#include <GL/gl.h>
#include <Vrui/Tool.h>
#include <Vrui/Application.h>

#include "Types.h"

/* Forward declarations: */
class WaterTable2;
class EarthquakeManager;
class Sandbox;
class EarthquakeTool;

class EarthquakeToolFactory:public Vrui::ToolFactory
	{
	friend class EarthquakeTool;
	
	/* Elements: */
	private:
	WaterTable2* waterTable; // Pointer to water table object from which to request bathymetry grids
	EarthquakeManager* earthquakeManager; // Pointer to earthquake manager object
	GLsizei gridSize[2]; // Width and height of the water table's bathymetry grid
	
	/* Constructors and destructors: */
	public:
	EarthquakeToolFactory(WaterTable2* sWaterTable, EarthquakeManager* sEarthquakeManager, Vrui::ToolManager& toolManager);
	virtual ~EarthquakeToolFactory(void);
	
	/* Methods from Vrui::ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Vrui::Tool* createTool(const Vrui::ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Vrui::Tool* tool) const;
	};

class EarthquakeTool:public Vrui::Tool,public Vrui::Application::Tool<Sandbox>
	{
	friend class EarthquakeToolFactory;
	typedef Geometry::Point<Scalar,3> OPoint; // Point in 3D object space
	
	/* Elements: */
	private:
	static EarthquakeToolFactory* factory; // Pointer to the factory object for this class
	GLfloat* bathymetryBuffer; // Bathymetry grid buffer
	bool requestPending; // Flag if this tool has a pending request to retrieve a bathymetry grid
	OPoint basePlaneCorners[4]; // Corners of the configured sandbox area
	int planePoints[2][3]; // Two points that determine the bounding line of a planar perturbation
	bool isCircular; // Flag if current perturbation is circular
	bool isPlanar; // Flag if current perturbation is planar
	
	/* Private methods: */
	void createCircularPerturbation(int center[], int radius, double perturbation); // Creates circular perturbation in bathymetry grid
	void createPlanarPerturbation(int p1[], int p2[], double perturbation); // Creates planar perturbation in bathymetry grid
	int getPixelPos(OPoint p1, const char comp); // Get the pixel position of a object space point compenent (x | y)
	
	/* Constructors and destructors: */
	public:
	static EarthquakeToolFactory* initClass(WaterTable2* sWaterTable, EarthquakeManager* sEarthquakeManager, Vrui::ToolManager& toolManager);
	EarthquakeTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment);
	virtual ~EarthquakeTool(void);
	
	/* Methods from class Vrui::Tool: */
	virtual const Vrui::ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,Vrui::InputDevice::ButtonCallbackData* cbData);
	virtual void frame(void);
	};

#endif
