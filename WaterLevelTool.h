/***********************************************************************
WaterLevelTool - Tool class to increase/decrease the water level in an
augmented reality sandbox.
Copyright (c) 2012-2013 Oliver Kreylos

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

#ifndef WATERLEVELTOOL_INCLUDED
#define WATERLEVELTOOL_INCLUDED

#include <Vrui/Tool.h>
#include <Vrui/GenericToolFactory.h>
#include <Vrui/Application.h>

/* Forward declarations: */
class Sandbox;
class WaterLevelTool;
typedef Vrui::GenericToolFactory<WaterLevelTool> WaterLevelToolFactory;

class WaterLevelTool:public Vrui::Tool,public Vrui::Application::Tool<Sandbox>
	{
	friend class Vrui::GenericToolFactory<WaterLevelTool>;
	
	/* Elements: */
	private:
	static WaterLevelToolFactory* factory; // Pointer to the factory object for this class
	float waterLevelStep;
	
	/* Constructors and destructors: */
	public:
	static WaterLevelToolFactory* initClass(Vrui::ToolManager& toolManager);
	WaterLevelTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment);
	virtual ~WaterLevelTool(void);
	
	/* Methods from class Vrui::Tool: */
	virtual void configure(const Misc::ConfigurationFileSection& configFileSection);
	virtual const Vrui::ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,Vrui::InputDevice::ButtonCallbackData* cbData);
	};

#endif
