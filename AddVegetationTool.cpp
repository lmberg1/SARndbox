/***********************************************************************
AddVegetationTool - Tool class to locally add or remove vegetation from an
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

#include "AddVegetationTool.h"

#include <Misc/FunctionCalls.h>
#include <GL/gl.h>
#include <GL/Extensions/GLARBVertexProgram.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <Vrui/Vrui.h>
#include <Vrui/ToolManager.h>
#include <Vrui/DisplayState.h>

#include "WaterTable2.h"
#include "Sandbox.h"

/***************************************
Static elements of class AddVegetationTool:
***************************************/

AddVegetationToolFactory* AddVegetationTool::factory=0;

/*******************************
Methods of class AddVegetationTool:
*******************************/

AddVegetationToolFactory* AddVegetationTool::initClass(Vrui::ToolManager& toolManager)
	{
	/* Create the tool factory: */
	factory=new AddVegetationToolFactory("AddVegetationTool","Manage Vegetation",0,toolManager);
	
	/* Set up the tool class' input layout: */
	factory->setNumButtons(2);
	factory->setButtonFunction(0,"Add");
	factory->setButtonFunction(1,"Remove");
	
	/* Register and return the class: */
	toolManager.addClass(factory,Vrui::ToolManager::defaultToolFactoryDestructor);
	return factory;
	}

AddVegetationTool::AddVegetationTool(const Vrui::ToolFactory* factory,const Vrui::ToolInputAssignment& inputAssignment)
	:Vrui::Tool(factory,inputAssignment),
	 addVegetationFunction(0),
	 adding(0.0f)
	{
	}

AddVegetationTool::~AddVegetationTool(void)
	{
	}

void AddVegetationTool::initialize(void)
	{
	/* Register a render function with the water table: */
	if(application->waterTable!=0)
		{
		addVegetationFunction=Misc::createFunctionCall(this,&AddVegetationTool::addVegetation);
		application->waterTable->addVegetationRenderFunction(addVegetationFunction);
		}
	}

void AddVegetationTool::deinitialize(void)
	{
	/* Unregister the render function from the water table: */
	if(application->waterTable!=0)
		application->waterTable->removeVegetationRenderFunction(addVegetationFunction);
	delete addVegetationFunction;
	addVegetationFunction=0;
	}

const Vrui::ToolFactory* AddVegetationTool::getFactory(void) const
	{
	return factory;
	}

void AddVegetationTool::buttonCallback(int buttonSlotIndex,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	GLfloat waterAmount=application->rainStrength;
	if(!cbData->newButtonState)
		waterAmount=-waterAmount;
	if (buttonSlotIndex==0)
		application->waterTable->setClearVegetation(false);
	if(buttonSlotIndex==1)
		{
		waterAmount=-waterAmount;
		application->waterTable->setClearVegetation(true);
		}
	adding+=waterAmount;
	}

void AddVegetationTool::initContext(GLContextData& contextData) const
	{
	/* Initialize the required OpenGL extensions: */
	GLARBVertexProgram::initExtension();
	}

void AddVegetationTool::addVegetation(GLContextData& contextData) const
	{
	if(adding!=0.0f)
		{
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_CULL_FACE);
		
		/* Get the current vegetation disk position and size in camera coordinates: */
		Vrui::Point vegetationPos=Vrui::getInverseNavigationTransformation().transform(getButtonDevicePosition(0));
		Vrui::Scalar vegetationRadius=Vrui::getPointPickDistance()*Vrui::Scalar(0.5);
		
		/* Render the vegetation disk: */
		Vrui::Vector z=application->waterTable->getBaseTransform().inverseTransform(Vrui::Vector(0,0,1));
		Vrui::Vector x=Geometry::normal(z);
		Vrui::Vector y=Geometry::cross(z,x);
		x*=vegetationRadius/Geometry::mag(x);
		y*=vegetationRadius/Geometry::mag(y);
		
		glVertexAttrib1fARB(1,adding/application->waterSpeed);
		glBegin(GL_POLYGON);
		for(int i=0;i<32;++i)
			{
			Vrui::Scalar angle=Vrui::Scalar(2)*Math::Constants<Vrui::Scalar>::pi*Vrui::Scalar(i)/Vrui::Scalar(32);
			glVertex(vegetationPos+x*Math::cos(angle)+y*Math::sin(angle));
			}
		glEnd();
		
		glPopAttrib();
		}
	}
