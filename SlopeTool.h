/***********************************************************************
SlopeTool - Tool class to show slope in an augmented
reality sandbox.
Copyright (c) 2013-2016 Oliver Kreylos

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

#ifndef SLOPETOOL_INCLUDED
#define SLOPETOOL_INCLUDED

#include <GL/gl.h>
#include <GL/GLObject.h>
#include <Vrui/Tool.h>
#include <Vrui/Application.h>

#include "Types.h"

/* Forward declarations: */
class Sandbox;
class SlopeTool;

class SlopeToolFactory:public Vrui::ToolFactory
	{
	friend class SlopeTool;
	
	/* Constructors and destructors: */
	public:
	SlopeToolFactory(Vrui::ToolManager& toolManager);
	virtual ~SlopeToolFactory(void);
	
	/* Methods from Vrui::ToolFactory: */
	virtual const char* getName(void) const;
	virtual const char* getButtonFunction(int buttonSlotIndex) const;
	virtual Vrui::Tool* createTool(const Vrui::ToolInputAssignment& inputAssignment) const;
	virtual void destroyTool(Vrui::Tool* tool) const;
	};

class SlopeTool:public Vrui::Tool,public Vrui::Application::Tool<Sandbox>
	{
	friend class SlopeToolFactory;
	
	/* Elements: */
	private:
	static SlopeToolFactory* factory; // Pointer to the factory object for this class
	
	/* Constructors and destructors: */
	public:
	static SlopeToolFactory* initClass(Vrui::ToolManager& toolManager);
	SlopeTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment);
	virtual ~SlopeTool(void);
	
	/* Methods from class Vrui::Tool: */
	virtual const Vrui::ToolFactory* getFactory(void) const;
	virtual void buttonCallback(int buttonSlotIndex,Vrui::InputDevice::ButtonCallbackData* cbData);
	};

#endif
