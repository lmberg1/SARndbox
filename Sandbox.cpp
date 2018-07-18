/***********************************************************************
Sandbox - Vrui application to drive an augmented reality sandbox.
Copyright (c) 2012-2016 Oliver Kreylos

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

#include "Sandbox.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <Misc/SizedTypes.h>
#include <Misc/SelfDestructPointer.h>
#include <Misc/FixedArray.h>
#include <Misc/FunctionCalls.h>
#include <Misc/FileNameExtensions.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ArrayValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <IO/File.h>
#include <IO/ValueSource.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Math/Interval.h>
#include <Math/MathValueCoders.h>
#include <Geometry/Point.h>
#include <Geometry/AffineCombiner.h>
#include <Geometry/HVector.h>
#include <Geometry/Plane.h>
#include <Geometry/LinearUnit.h>
#include <Geometry/GeometryValueCoders.h>
#include <Geometry/OutputOperators.h>
#include <GL/gl.h>
#include <GL/GLMaterialTemplates.h>
#include <GL/GLColorMap.h>
#include <GL/GLLightTracker.h>
#include <GL/Extensions/GLEXTFramebufferObject.h>
#include <GL/Extensions/GLARBTextureRectangle.h>
#include <GL/Extensions/GLARBTextureFloat.h>
#include <GL/Extensions/GLARBTextureRg.h>
#include <GL/Extensions/GLARBDepthTexture.h>
#include <GL/Extensions/GLARBShaderObjects.h>
#include <GL/Extensions/GLARBVertexShader.h>
#include <GL/Extensions/GLARBFragmentShader.h>
#include <GL/Extensions/GLARBMultitexture.h>
#include <GL/GLContextData.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/Menu.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/Margin.h>
#include <GLMotif/Label.h>
#include <GLMotif/TextField.h>
#include <Vrui/Vrui.h>
#include <Vrui/CoordinateManager.h>
#include <Vrui/Lightsource.h>
#include <Vrui/LightsourceManager.h>
#include <Vrui/Viewer.h>
#include <Vrui/ToolManager.h>
#include <Vrui/DisplayState.h>
#include <Vrui/OpenFile.h>
#include <Kinect/FileFrameSource.h>
#include <Kinect/DirectFrameSource.h>
#include <Kinect/OpenDirectFrameSource.h>

#define SAVEDEPTH 0

#if SAVEDEPTH
#include <Images/RGBImage.h>
#include <Images/WriteImageFile.h>
#endif

#include "FrameFilter.h"
#include "DepthImageRenderer.h"
#include "ElevationColorMap.h"
#include "DEM.h"
#include "Image.h"
#include "SurfaceRenderer.h"
#include "WaterTable2.h"
#include "HandExtractor.h"
#include "WaterRenderer.h"
#include "GlobalWaterTool.h"
#include "LocalWaterTool.h"
#include "DEMTool.h"
#include "ImageTool.h"
#include "SlopeTool.h"
#include "WaterLevelTool.h"
#include "AddVegetationTool.h"
#include "BathymetrySaverTool.h"
#include "EarthquakeTool.h"
#include "EarthquakeManager.h"

#include "Config.h"

/**********************************
Methods of class Sandbox::DataItem:
**********************************/

Sandbox::DataItem::DataItem(void)
	:waterTableTime(0.0),
	 shadowFramebufferObject(0),shadowDepthTextureObject(0)
	{
	/* Check if all required extensions are supported: */
	bool supported=GLEXTFramebufferObject::isSupported();
	supported=supported&&GLARBTextureRectangle::isSupported();
	supported=supported&&GLARBTextureFloat::isSupported();
	supported=supported&&GLARBTextureRg::isSupported();
	supported=supported&&GLARBDepthTexture::isSupported();
	supported=supported&&GLARBShaderObjects::isSupported();
	supported=supported&&GLARBVertexShader::isSupported();
	supported=supported&&GLARBFragmentShader::isSupported();
	supported=supported&&GLARBMultitexture::isSupported();
	if(!supported)
		Misc::throwStdErr("Sandbox: Not all required extensions are supported by local OpenGL");
	
	/* Initialize all required extensions: */
	GLEXTFramebufferObject::initExtension();
	GLARBTextureRectangle::initExtension();
	GLARBTextureFloat::initExtension();
	GLARBTextureRg::initExtension();
	GLARBDepthTexture::initExtension();
	GLARBShaderObjects::initExtension();
	GLARBVertexShader::initExtension();
	GLARBFragmentShader::initExtension();
	GLARBMultitexture::initExtension();
	}

Sandbox::DataItem::~DataItem(void)
	{
	/* Delete all shaders, buffers, and texture objects: */
	glDeleteFramebuffersEXT(1,&shadowFramebufferObject);
	glDeleteTextures(1,&shadowDepthTextureObject);
	}

/****************************************
Methods of class Sandbox::RenderSettings:
****************************************/

Sandbox::RenderSettings::RenderSettings(void)
	:fixProjectorView(false),projectorTransform(PTransform::identity),projectorTransformValid(false),
	 hillshade(false),surfaceMaterial(GLMaterial::Color(1.0f,1.0f,1.0f)),
	 useShadows(false),
	 elevationColorMap(0),
	 slopeColorMap(0),
	 useContourLines(true),contourLineSpacing(0.75f),
	 renderWaterSurface(false),waterOpacity(2.0f),
	 useVegetation(false),
	 showSlope(false),
	 surfaceRenderer(0),waterRenderer(0)
	{
	/* Load the default projector transformation: */
	loadProjectorTransform(CONFIG_DEFAULTPROJECTIONMATRIXFILENAME);
	}

Sandbox::RenderSettings::RenderSettings(const Sandbox::RenderSettings& source)
	:fixProjectorView(source.fixProjectorView),projectorTransform(source.projectorTransform),projectorTransformValid(source.projectorTransformValid),
	 hillshade(source.hillshade),surfaceMaterial(source.surfaceMaterial),
	 useShadows(source.useShadows),
	 elevationColorMap(source.elevationColorMap!=0?new ElevationColorMap(*source.elevationColorMap):0),
	 slopeColorMap(source.slopeColorMap!=0?new ElevationColorMap(*source.slopeColorMap):0),
	 useContourLines(source.useContourLines),contourLineSpacing(source.contourLineSpacing),
	 renderWaterSurface(source.renderWaterSurface),waterOpacity(source.waterOpacity),
	 useVegetation(source.useVegetation),
	 showSlope(source.showSlope),
	 surfaceRenderer(0),waterRenderer(0)
	{
	}

Sandbox::RenderSettings::~RenderSettings(void)
	{
	delete surfaceRenderer;
	delete waterRenderer;
	delete elevationColorMap;
	delete slopeColorMap;
	}

void Sandbox::RenderSettings::loadProjectorTransform(const char* projectorTransformName)
	{
	std::string fullProjectorTransformName;
	try
		{
		/* Open the projector transformation file: */
		if(projectorTransformName[0]=='/')
			{
			/* Use the absolute file name directly: */
			fullProjectorTransformName=projectorTransformName;
			}
		else
			{
			/* Assemble a file name relative to the configuration file directory: */
			fullProjectorTransformName=CONFIG_CONFIGDIR;
			fullProjectorTransformName.push_back('/');
			fullProjectorTransformName.append(projectorTransformName);
			}
		IO::FilePtr projectorTransformFile=Vrui::openFile(fullProjectorTransformName.c_str(),IO::File::ReadOnly);
		projectorTransformFile->setEndianness(Misc::LittleEndian);
		
		/* Read the projector transformation matrix from the binary file: */
		Misc::Float64 pt[16];
		projectorTransformFile->read(pt,16);
		projectorTransform=PTransform::fromRowMajor(pt);
		
		projectorTransformValid=true;
		}
	catch(std::runtime_error err)
		{
		/* Print an error message and disable calibrated projections: */
		std::cerr<<"Unable to load projector transformation from file "<<fullProjectorTransformName<<" due to exception "<<err.what()<<std::endl;
		projectorTransformValid=false;
		}
	}

void Sandbox::RenderSettings::loadHeightMap(const char* heightMapName)
	{
	try
		{
		/* Load the elevation color map of the given name: */
		ElevationColorMap* newElevationColorMap=new ElevationColorMap(heightMapName);
		
		/* Delete the previous elevation color map and assign the new one: */
		delete elevationColorMap;
		elevationColorMap=newElevationColorMap;
		}
	catch(std::runtime_error err)
		{
		std::cerr<<"Ignoring height map due to exception "<<err.what()<<std::endl;
		}
	}
void Sandbox::RenderSettings::loadSlopeMap(const char* heightMapName)
	{
	try
		{
		/* Load the elevation color map of the given name: */
		ElevationColorMap* newSlopeColorMap=new ElevationColorMap(heightMapName);
		
		/* Delete the previous elevation color map and assign the new one: */
		delete slopeColorMap;
		slopeColorMap=newSlopeColorMap;
		}
	catch(std::runtime_error err)
		{
		std::cerr<<"Ignoring height map due to exception "<<err.what()<<std::endl;
		}
	}

/************************
Methods of class Sandbox:
************************/

void Sandbox::rawDepthFrameDispatcher(const Kinect::FrameBuffer& frameBuffer)
	{
	/* Pass the received frame to the frame filter and the hand extractor: */
	if(frameFilter!=0&&!pauseUpdates)
		frameFilter->receiveRawFrame(frameBuffer);
	if(handExtractor!=0)
		handExtractor->receiveRawFrame(frameBuffer);
	}

void Sandbox::receiveFilteredFrame(const Kinect::FrameBuffer& frameBuffer)
	{
	/* Put the new frame into the frame input buffer: */
	filteredFrames.postNewValue(frameBuffer);
	
	/* Wake up the foreground thread: */
	Vrui::requestUpdate();
	}

void Sandbox::toggleDEM(DEM* dem)
	{
	/* Check if this is the active DEM: */
	if(activeDem==dem)
		{
		/* Deactivate the currently active DEM: */
		activeDem=0;
		}
	else
		{
		/* Activate this DEM: */
		activeDem=dem;
		
		/* Set the dem control dialog slider values */
		if (demVerticalScaleSlider != 0)
			demVerticalScaleSlider->setValue(activeDem->getDemVerticalScale());
		if (demVerticalShiftSlider != 0);
			demVerticalShiftSlider->setValue(activeDem->getDemVerticalShift());
		}
	/* Enable DEM matching in all surface renderers that use a fixed projector matrix, i.e., in all physical sandboxes: */
	for(std::vector<RenderSettings>::iterator rsIt=renderSettings.begin();rsIt!=renderSettings.end();++rsIt)
		if(rsIt->fixProjectorView)
			rsIt->surfaceRenderer->setDem(activeDem);
	}
	
void Sandbox::toggleImage(Image* image)
	{
	/* Check if this is the active Image: */
	if(activeImage==image)
		{
		/* Deactivate the currently active Image: */
		activeImage=0;
		}
	else
		{
		/* Activate this Image: */
		activeImage=image;
		}
	
	/* Enable image matching in all surface renderers */
	for(std::vector<RenderSettings>::iterator rsIt=renderSettings.begin();rsIt!=renderSettings.end();++rsIt)
		rsIt->surfaceRenderer->setImage(activeImage);
	}
	
void Sandbox::toggleSlope(void)
	{
	/* Enable slope matching in all surface renderers */
	for(std::vector<RenderSettings>::iterator rsIt=renderSettings.begin();rsIt!=renderSettings.end();++rsIt)
		{
		rsIt->showSlope=!rsIt->showSlope;
		rsIt->surfaceRenderer->setShowSlope(rsIt->showSlope);
		}
	}

void Sandbox::addWater(GLContextData& contextData) const
	{
	/* Check if the most recent rain object list is not empty: */
	if(handExtractor!=0&&!handExtractor->getLockedExtractedHands().empty())
		{
		/* Render all rain objects into the water table: */
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_CULL_FACE);
		
		/* Create a local coordinate frame to render rain disks: */
		Vector z=waterTable->getBaseTransform().inverseTransform(Vector(0,0,1));
		Vector x=Geometry::normal(z);
		Vector y=Geometry::cross(z,x);
		x.normalize();
		y.normalize();
		
		glVertexAttrib1fARB(1,rainStrength/waterSpeed);
		for(HandExtractor::HandList::const_iterator hIt=handExtractor->getLockedExtractedHands().begin();hIt!=handExtractor->getLockedExtractedHands().end();++hIt)
			{
			/* Render a rain disk approximating the hand: */
			glBegin(GL_POLYGON);
			for(int i=0;i<32;++i)
				{
				Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(32);
				glVertex(hIt->center+x*(Math::cos(angle)*hIt->radius*0.75)+y*(Math::sin(angle)*hIt->radius*0.75));
				}
			glEnd();
			}
		
		glPopAttrib();
		}
	}

void Sandbox::pauseUpdatesCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	pauseUpdates=cbData->set;
	}

void Sandbox::showWaterControlDialogCallback(Misc::CallbackData* cbData)
	{
	Vrui::popupPrimaryWidget(waterControlDialog);
	}
	
void Sandbox::showEarthquakeControlDialogCallback(Misc::CallbackData* cbData)
	{
	Vrui::popupPrimaryWidget(earthquakeControlDialog);
	}
	
void Sandbox::showDemControlDialogCallback(Misc::CallbackData* cbData)
	{
	Vrui::popupPrimaryWidget(demControlDialog);
	}

void Sandbox::waterSpeedSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	waterSpeed=cbData->value;
	}

void Sandbox::waterMaxStepsSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	waterMaxSteps=int(Math::floor(cbData->value+0.5));
	}

void Sandbox::waterAttenuationSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	waterTable->setAttenuation(GLfloat(1.0-cbData->value));
	}

void Sandbox::baseWaterLevelSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	baseWaterLevel = GLfloat(cbData->value);
	}
	
void Sandbox::earthquakeRadiusSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	earthquakeManager->setEarthquakeRadius(GLfloat(cbData->value));
	}

void Sandbox::earthquakeStrengthSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	earthquakeManager->setEarthquakePerturbation(GLfloat(cbData->value));
	}
	
void Sandbox::demVerticalShiftSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	activeDem->setDemVerticalShift((float) cbData->value);
	}

void Sandbox::demVerticalScaleSliderCallback(GLMotif::TextFieldSlider::ValueChangedCallbackData* cbData)
	{
	activeDem->setDemVerticalScale((float) cbData->value);
	}

GLMotif::PopupMenu* Sandbox::createMainMenu(void)
	{
	/* Create a popup shell to hold the main menu: */
	GLMotif::PopupMenu* mainMenuPopup=new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
	mainMenuPopup->setTitle("AR Sandbox");
	
	/* Create the main menu itself: */
	GLMotif::Menu* mainMenu=new GLMotif::Menu("MainMenu",mainMenuPopup,false);
	
	/* Create a button to pause topography updates: */
	pauseUpdatesToggle=new GLMotif::ToggleButton("PauseUpdatesToggle",mainMenu,"Pause Topography");
	pauseUpdatesToggle->setToggle(false);
	pauseUpdatesToggle->getValueChangedCallbacks().add(this,&Sandbox::pauseUpdatesCallback);
	
	if(waterTable!=0)
		{
		/* Create a button to show the water control dialog: */
		GLMotif::Button* showWaterControlDialogButton=new GLMotif::Button("ShowWaterControlDialogButton",mainMenu,"Show Water Simulation Control");
		showWaterControlDialogButton->getSelectCallbacks().add(this,&Sandbox::showWaterControlDialogCallback);
		
		/* Create a button to show the earthquake control dialog: */
		GLMotif::Button* showEarthquakeControlDialogButton=new GLMotif::Button("ShowEarthquakeControlDialogButton",mainMenu,"Show Earthquake Simulation Control");
		showEarthquakeControlDialogButton->getSelectCallbacks().add(this,&Sandbox::showEarthquakeControlDialogCallback);
		}
		
	/* Create a button to show the dem control dialog: */
	GLMotif::Button* showDemControlDialogButton=new GLMotif::Button("ShowDemControlDialogButton",mainMenu,"Show DEM Control");
	showDemControlDialogButton->getSelectCallbacks().add(this,&Sandbox::showDemControlDialogCallback);

	/* Finish building the main menu: */
	mainMenu->manageChild();
	
	return mainMenuPopup;
	}

GLMotif::PopupWindow* Sandbox::createWaterControlDialog(void)
	{
	const GLMotif::StyleSheet& ss=*Vrui::getWidgetManager()->getStyleSheet();
	
	/* Create a popup window shell: */
	GLMotif::PopupWindow* waterControlDialogPopup=new GLMotif::PopupWindow("WaterControlDialogPopup",Vrui::getWidgetManager(),"Water Simulation Control");
	waterControlDialogPopup->setCloseButton(true);
	waterControlDialogPopup->setResizableFlags(true,false);
	waterControlDialogPopup->popDownOnClose();
	
	GLMotif::RowColumn* waterControlDialog=new GLMotif::RowColumn("WaterControlDialog",waterControlDialogPopup,false);
	waterControlDialog->setOrientation(GLMotif::RowColumn::VERTICAL);
	waterControlDialog->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	waterControlDialog->setNumMinorWidgets(2);
	
	new GLMotif::Label("WaterSpeedLabel",waterControlDialog,"Speed");
	
	waterSpeedSlider=new GLMotif::TextFieldSlider("WaterSpeedSlider",waterControlDialog,8,ss.fontHeight*10.0f);
	waterSpeedSlider->getTextField()->setFieldWidth(7);
	waterSpeedSlider->getTextField()->setPrecision(4);
	waterSpeedSlider->getTextField()->setFloatFormat(GLMotif::TextField::SMART);
	waterSpeedSlider->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
	waterSpeedSlider->setValueType(GLMotif::TextFieldSlider::UINT);
	waterSpeedSlider->setValueRange(0.0,10.0,0.05);
	waterSpeedSlider->getSlider()->addNotch(1.0);
	waterSpeedSlider->setValue(waterSpeed);
	waterSpeedSlider->getValueChangedCallbacks().add(this,&Sandbox::waterSpeedSliderCallback);
	
	new GLMotif::Label("WaterMaxStepsLabel",waterControlDialog,"Max Steps");
	
	waterMaxStepsSlider=new GLMotif::TextFieldSlider("WaterMaxStepsSlider",waterControlDialog,8,ss.fontHeight*10.0f);
	waterMaxStepsSlider->getTextField()->setFieldWidth(7);
	waterMaxStepsSlider->getTextField()->setPrecision(0);
	waterMaxStepsSlider->getTextField()->setFloatFormat(GLMotif::TextField::FIXED);
	waterMaxStepsSlider->setSliderMapping(GLMotif::TextFieldSlider::LINEAR);
	waterMaxStepsSlider->setValueType(GLMotif::TextFieldSlider::UINT);
	waterMaxStepsSlider->setValueRange(0,200,1);
	waterMaxStepsSlider->setValue(waterMaxSteps);
	waterMaxStepsSlider->getValueChangedCallbacks().add(this,&Sandbox::waterMaxStepsSliderCallback);
	
	new GLMotif::Label("FrameRateLabel",waterControlDialog,"Frame Rate");
	
	GLMotif::Margin* frameRateMargin=new GLMotif::Margin("FrameRateMargin",waterControlDialog,false);
	frameRateMargin->setAlignment(GLMotif::Alignment::LEFT);
	
	frameRateTextField=new GLMotif::TextField("FrameRateTextField",frameRateMargin,8);
	frameRateTextField->setFieldWidth(7);
	frameRateTextField->setPrecision(2);
	frameRateTextField->setFloatFormat(GLMotif::TextField::FIXED);
	frameRateTextField->setValue(0.0);
	
	frameRateMargin->manageChild();
	
	new GLMotif::Label("WaterAttenuationLabel",waterControlDialog,"Attenuation");
	
	waterAttenuationSlider=new GLMotif::TextFieldSlider("WaterAttenuationSlider",waterControlDialog,8,ss.fontHeight*10.0f);
	waterAttenuationSlider->getTextField()->setFieldWidth(7);
	waterAttenuationSlider->getTextField()->setPrecision(5);
	waterAttenuationSlider->getTextField()->setFloatFormat(GLMotif::TextField::SMART);
	waterAttenuationSlider->setSliderMapping(GLMotif::TextFieldSlider::EXP10);
	waterAttenuationSlider->setValueRange(0.001,1.0,0.01);
	waterAttenuationSlider->getSlider()->addNotch(Math::log10(1.0-double(waterTable->getAttenuation())));
	waterAttenuationSlider->setValue(1.0-double(waterTable->getAttenuation()));
	waterAttenuationSlider->getValueChangedCallbacks().add(this,&Sandbox::waterAttenuationSliderCallback);

	new GLMotif::Label("BaseWaterLevelLabel",waterControlDialog,"Base Water Level");
	
	baseWaterLevelSlider=new GLMotif::TextFieldSlider("BaseWaterLevelSlider",waterControlDialog,8,ss.fontHeight*10.0f);
	baseWaterLevelSlider->getTextField()->setFieldWidth(7);
	baseWaterLevelSlider->getTextField()->setPrecision(4);
	baseWaterLevelSlider->getTextField()->setFloatFormat(GLMotif::TextField::SMART);
	baseWaterLevelSlider->setValueRange(-10.0,10.0,0.05);
	baseWaterLevelSlider->getSlider()->addNotch(double(baseWaterLevel));
	baseWaterLevelSlider->setValue(double(baseWaterLevel));
	baseWaterLevelSlider->getValueChangedCallbacks().add(this,&Sandbox::baseWaterLevelSliderCallback);
	
	waterControlDialog->manageChild();
	
	return waterControlDialogPopup;
	}
	
GLMotif::PopupWindow* Sandbox::createEarthquakeControlDialog(void)
	{
	const GLMotif::StyleSheet& ss=*Vrui::getWidgetManager()->getStyleSheet();
	
	/* Create a popup window shell: */
	GLMotif::PopupWindow* earthquakeControlDialogPopup=new GLMotif::PopupWindow("EarthquakeControlDialogPopup",Vrui::getWidgetManager(),"Earthquake Simulation Control");
	earthquakeControlDialogPopup->setCloseButton(true);
	earthquakeControlDialogPopup->setResizableFlags(true,false);
	earthquakeControlDialogPopup->popDownOnClose();
	
	GLMotif::RowColumn* earthquakeControlDialog=new GLMotif::RowColumn("EarthquakeControlDialog",earthquakeControlDialogPopup,false);
	earthquakeControlDialog->setOrientation(GLMotif::RowColumn::VERTICAL);
	earthquakeControlDialog->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	earthquakeControlDialog->setNumMinorWidgets(2);
	
	new GLMotif::Label("EarthquakeRadius",earthquakeControlDialog,"Radius");
	
	earthquakeRadiusSlider=new GLMotif::TextFieldSlider("earthquakeRadiusSlider",earthquakeControlDialog,8,ss.fontHeight*10.0f);
	earthquakeRadiusSlider->getTextField()->setFieldWidth(7);
	earthquakeRadiusSlider->getTextField()->setPrecision(4);
	earthquakeRadiusSlider->getTextField()->setFloatFormat(GLMotif::TextField::SMART);
	earthquakeRadiusSlider->setValueRange(0,200.0,1.0);
	earthquakeRadiusSlider->getSlider()->addNotch(earthquakeManager->getEarthquakeRadius());
	earthquakeRadiusSlider->setValue(earthquakeManager->getEarthquakeRadius());
	earthquakeRadiusSlider->getValueChangedCallbacks().add(this,&Sandbox::earthquakeRadiusSliderCallback);
	
	new GLMotif::Label("EarthquakeStrength",earthquakeControlDialog,"Strength");
	
	earthquakeStrengthSlider=new GLMotif::TextFieldSlider("earthquakeStrengthSlider",earthquakeControlDialog,8,ss.fontHeight*10.0f);
	earthquakeStrengthSlider->getTextField()->setFieldWidth(7);
	earthquakeStrengthSlider->getTextField()->setPrecision(4);
	earthquakeStrengthSlider->getTextField()->setFloatFormat(GLMotif::TextField::SMART);
	earthquakeStrengthSlider->setValueRange(0.0,100.0,0.5);
	earthquakeStrengthSlider->getSlider()->addNotch(earthquakeManager->getEarthquakePerturbation());
	earthquakeStrengthSlider->setValue(earthquakeManager->getEarthquakePerturbation());
	earthquakeStrengthSlider->getValueChangedCallbacks().add(this,&Sandbox::earthquakeStrengthSliderCallback);
	
	earthquakeControlDialog->manageChild();
	
	return earthquakeControlDialogPopup;
	}
	
GLMotif::PopupWindow* Sandbox::createDemControlDialog(void)
	{
	const GLMotif::StyleSheet& ss=*Vrui::getWidgetManager()->getStyleSheet();
	
	/* Create a popup window shell: */
	GLMotif::PopupWindow* demControlDialogPopup=new GLMotif::PopupWindow("DemControlDialogPopup",Vrui::getWidgetManager(),"Dem Simulation Control");
	demControlDialogPopup->setCloseButton(true);
	demControlDialogPopup->setResizableFlags(true,false);
	demControlDialogPopup->popDownOnClose();
	
	GLMotif::RowColumn* demControlDialog=new GLMotif::RowColumn("DemControlDialog",demControlDialogPopup,false);
	demControlDialog->setOrientation(GLMotif::RowColumn::VERTICAL);
	demControlDialog->setPacking(GLMotif::RowColumn::PACK_TIGHT);
	demControlDialog->setNumMinorWidgets(2);
	
	new GLMotif::Label("demVerticalScale",demControlDialog,"Vertical Scale");
	
	demVerticalScaleSlider=new GLMotif::TextFieldSlider("DemVerticalScaleSlider",demControlDialog,8,ss.fontHeight*10.0f);
	demVerticalScaleSlider->getTextField()->setFieldWidth(7);
	demVerticalScaleSlider->getTextField()->setPrecision(4);
	demVerticalScaleSlider->getTextField()->setFloatFormat(GLMotif::TextField::SMART);
	demVerticalScaleSlider->setValueRange(0.01,40.0,0.1);
	demVerticalScaleSlider->getSlider()->addNotch(1.0);
	demVerticalScaleSlider->setValue(1.0);
	demVerticalScaleSlider->getValueChangedCallbacks().add(this,&Sandbox::demVerticalScaleSliderCallback);
	
	new GLMotif::Label("demVerticalShift",demControlDialog,"Vertical Shift");
	
	demVerticalShiftSlider=new GLMotif::TextFieldSlider("DemVerticalShiftSlider",demControlDialog,8,ss.fontHeight*10.0f);
	demVerticalShiftSlider->getTextField()->setFieldWidth(7);
	demVerticalShiftSlider->getTextField()->setPrecision(4);
	demVerticalShiftSlider->getTextField()->setFloatFormat(GLMotif::TextField::SMART);
	demVerticalShiftSlider->setValueRange(-20.0,20.0,0.1);
	demVerticalShiftSlider->getSlider()->addNotch(defaultDemVerticalShift);
	demVerticalShiftSlider->setValue(defaultDemVerticalShift);
	demVerticalShiftSlider->getValueChangedCallbacks().add(this,&Sandbox::demVerticalShiftSliderCallback);
	
	demControlDialog->manageChild();
	
	return demControlDialogPopup;
	}

namespace {

/****************
Helper functions:
****************/

void printUsage(void)
	{
	std::cout<<"Usage: SARndbox [option 1] ... [option n]"<<std::endl;
	std::cout<<"  Options:"<<std::endl;
	std::cout<<"  -h"<<std::endl;
	std::cout<<"     Prints this help message"<<std::endl;
	std::cout<<"  -c <camera index>"<<std::endl;
	std::cout<<"     Selects the local 3D camera of the given index (0: first camera"<<std::endl;
	std::cout<<"     on USB bus)"<<std::endl;
	std::cout<<"     Default: 0"<<std::endl;
	std::cout<<"  -f <frame file name prefix>"<<std::endl;
	std::cout<<"     Reads a pre-recorded 3D video stream from a pair of color/depth"<<std::endl;
	std::cout<<"     files of the given file name prefix"<<std::endl;
	std::cout<<"  -s <scale factor>"<<std::endl;
	std::cout<<"     Scale factor from real sandbox to simulated terrain"<<std::endl;
	std::cout<<"     Default: 100.0 (1:100 scale, 1cm in sandbox is 1m in terrain"<<std::endl;
	std::cout<<"  -slf <sandbox layout file name>"<<std::endl;
	std::cout<<"     Loads the sandbox layout file of the given name"<<std::endl;
	std::cout<<"     Default: "<<CONFIG_CONFIGDIR<<'/'<<CONFIG_DEFAULTBOXLAYOUTFILENAME<<std::endl;
	std::cout<<"  -er <min elevation> <max elevation>"<<std::endl;
	std::cout<<"     Sets the range of valid sand surface elevations relative to the"<<std::endl;
	std::cout<<"     ground plane in cm"<<std::endl;
	std::cout<<"     Default: Range of elevation color map"<<std::endl;
	std::cout<<"  -hmp <x> <y> <z> <offset>"<<std::endl;
	std::cout<<"     Sets an explicit base plane equation to use for height color mapping"<<std::endl;
	std::cout<<"  -nas <num averaging slots>"<<std::endl;
	std::cout<<"     Sets the number of averaging slots in the frame filter; latency is"<<std::endl;
	std::cout<<"     <num averaging slots> * 1/30 s"<<std::endl;
	std::cout<<"     Default: 30"<<std::endl;
	std::cout<<"  -sp <min num samples> <max variance>"<<std::endl;
	std::cout<<"     Sets the frame filter parameters minimum number of valid samples"<<std::endl;
	std::cout<<"     and maximum sample variance before convergence"<<std::endl;
	std::cout<<"     Default: 10 2"<<std::endl;
	std::cout<<"  -he <hysteresis envelope>"<<std::endl;
	std::cout<<"     Sets the size of the hysteresis envelope used for jitter removal"<<std::endl;
	std::cout<<"     Default: 0.1"<<std::endl;
	std::cout<<"  -wts <water grid width> <water grid height>"<<std::endl;
	std::cout<<"     Sets the width and height of the water flow simulation grid"<<std::endl;
	std::cout<<"     Default: 640 480"<<std::endl;
	std::cout<<"  -ws <water speed> <water max steps>"<<std::endl;
	std::cout<<"     Sets the relative speed of the water simulation and the maximum"<<std::endl;
	std::cout<<"     number of simulation steps per frame"<<std::endl;
	std::cout<<"     Default: 1.0 30"<<std::endl;
	std::cout<<"  -rer <min rain elevation> <max rain elevation>"<<std::endl;
	std::cout<<"     Sets the elevation range of the rain cloud level relative to the"<<std::endl;
	std::cout<<"     ground plane in cm"<<std::endl;
	std::cout<<"     Default: Above range of elevation color map"<<std::endl;
	std::cout<<"  -rs <rain strength>"<<std::endl;
	std::cout<<"     Sets the strength of global or local rainfall in cm/s"<<std::endl;
	std::cout<<"     Default: 0.25"<<std::endl;
	std::cout<<"  -evr <evaporation rate>"<<std::endl;
	std::cout<<"     Water evaporation rate in cm/s"<<std::endl;
	std::cout<<"     Default: 0.0"<<std::endl;
	std::cout<<"  -dds <DEM distance scale>"<<std::endl;
	std::cout<<"     DEM matching distance scale factor in cm"<<std::endl;
	std::cout<<"     Default: 1.0"<<std::endl;
	std::cout<<"  -wi <window index>"<<std::endl;
	std::cout<<"     Sets the zero-based index of the display window to which the"<<std::endl;
	std::cout<<"     following rendering settings are applied"<<std::endl;
	std::cout<<"     Default: 0"<<std::endl;
	std::cout<<"  -fpv [projector transform file name]"<<std::endl;
	std::cout<<"     Fixes the navigation transformation so that Kinect camera and"<<std::endl;
	std::cout<<"     projector are aligned, as defined by the projector transform file"<<std::endl;
	std::cout<<"     of the given name"<<std::endl;
	std::cout<<"     Default projector transform file name: "<<CONFIG_CONFIGDIR<<'/'<<CONFIG_DEFAULTPROJECTIONMATRIXFILENAME<<std::endl;
	std::cout<<"  -nhs"<<std::endl;
	std::cout<<"     Disables hill shading"<<std::endl;
	std::cout<<"  -uhs"<<std::endl;
	std::cout<<"     Enables hill shading"<<std::endl;
	std::cout<<"  -ns"<<std::endl;
	std::cout<<"     Disables shadows"<<std::endl;
	std::cout<<"  -us"<<std::endl;
	std::cout<<"     Enables shadows"<<std::endl;
	std::cout<<"  -nhm"<<std::endl;
	std::cout<<"     Disables elevation color mapping"<<std::endl;
	std::cout<<"  -uhm [elevation color map file name]"<<std::endl;
	std::cout<<"     Enables elevation color mapping and loads the elevation color map from"<<std::endl;
	std::cout<<"     the file of the given name"<<std::endl;
	std::cout<<"     Default elevation color  map file name: "<<CONFIG_CONFIGDIR<<'/'<<CONFIG_DEFAULTHEIGHTCOLORMAPFILENAME<<std::endl;
	std::cout<<"  -ncl"<<std::endl;
	std::cout<<"     Disables topographic contour lines"<<std::endl;
	std::cout<<"  -ucl [contour line spacing]"<<std::endl;
	std::cout<<"     Enables topographic contour lines and sets the elevation distance between"<<std::endl;
	std::cout<<"     adjacent contour lines to the given value in cm"<<std::endl;
	std::cout<<"     Default contour line spacing: 0.75"<<std::endl;
	std::cout<<"  -rws"<<std::endl;
	std::cout<<"     Renders water surface as geometric surface"<<std::endl;
	std::cout<<"  -rwt"<<std::endl;
	std::cout<<"     Renders water surface as texture"<<std::endl;
	std::cout<<"  -wo <water opacity>"<<std::endl;
	std::cout<<"     Sets the water depth at which water appears opaque in cm"<<std::endl;
	std::cout<<"     Default: 2.0"<<std::endl;
	std::cout<<"  -cp <control pipe name>"<<std::endl;
	std::cout<<"     Sets the name of a named POSIX pipe from which to read control commands"<<std::endl;
	}

}

Sandbox::Sandbox(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 camera(0),pixelDepthCorrection(0),
	 frameFilter(0),pauseUpdates(false),
	 depthImageRenderer(0),
	 waterTable(0),
	 handExtractor(0),addWaterFunction(0),addWaterFunctionRegistered(false),
	 sun(0),
	 activeDem(0), 
	 activeImage(0),
	 mainMenu(0),pauseUpdatesToggle(0),waterControlDialog(0), 
	 waterSpeedSlider(0),waterMaxStepsSlider(0),frameRateTextField(0),waterAttenuationSlider(0), 
	 baseWaterLevelSlider(0),
	 earthquakeControlDialog(0), earthquakeRadiusSlider(0), earthquakeStrengthSlider(0),
	 demControlDialog(0), demVerticalShiftSlider(0), demVerticalScaleSlider(0),
	 controlPipeFd(-1)
	{
	/* Read the sandbox's default configuration parameters: */
	std::string sandboxConfigFileName=CONFIG_CONFIGDIR;
	sandboxConfigFileName.push_back('/');
	sandboxConfigFileName.append(CONFIG_DEFAULTCONFIGFILENAME);
	Misc::ConfigurationFile sandboxConfigFile(sandboxConfigFileName.c_str());
	Misc::ConfigurationFileSection cfg=sandboxConfigFile.getSection("/SARndbox");
	unsigned int cameraIndex=cfg.retrieveValue<int>("./cameraIndex",0);
	std::string cameraConfiguration=cfg.retrieveString("./cameraConfiguration","Camera");
	double scale=cfg.retrieveValue<double>("./scaleFactor",100.0);
	std::string sandboxLayoutFileName=CONFIG_CONFIGDIR;
	sandboxLayoutFileName.push_back('/');
	sandboxLayoutFileName.append(CONFIG_DEFAULTBOXLAYOUTFILENAME);
	sandboxLayoutFileName=cfg.retrieveString("./sandboxLayoutFileName",sandboxLayoutFileName);
	Math::Interval<double> elevationRange=cfg.retrieveValue<Math::Interval<double> >("./elevationRange",Math::Interval<double>(-1000.0,1000.0));
	bool haveHeightMapPlane=cfg.hasTag("./heightMapPlane");
	Plane heightMapPlane;
	if(haveHeightMapPlane)
		heightMapPlane=cfg.retrieveValue<Plane>("./heightMapPlane");
	unsigned int numAveragingSlots=cfg.retrieveValue<unsigned int>("./numAveragingSlots",30);
	unsigned int minNumSamples=cfg.retrieveValue<unsigned int>("./minNumSamples",10);
	unsigned int maxVariance=cfg.retrieveValue<unsigned int>("./maxVariance",2);
	float hysteresis=cfg.retrieveValue<float>("./hysteresis",0.1f);
	Misc::FixedArray<unsigned int,2> wtSize;
	wtSize[0]=640;
	wtSize[1]=480;
	wtSize=cfg.retrieveValue<Misc::FixedArray<unsigned int,2> >("./waterTableSize",wtSize);
	waterSpeed=cfg.retrieveValue<double>("./waterSpeed",1.0);
	waterMaxSteps=cfg.retrieveValue<unsigned int>("./waterMaxSteps",30U);
	Math::Interval<double> rainElevationRange=cfg.retrieveValue<Math::Interval<double> >("./rainElevationRange",Math::Interval<double>(-1000.0,1000.0));
	rainStrength=cfg.retrieveValue<GLfloat>("./rainStrength",0.25f);
	double evaporationRate=cfg.retrieveValue<double>("./evaporationRate",0.0);
	baseWaterLevel=cfg.retrieveValue<GLfloat>("./baseWaterLevel",-2.0f);
	float demDistScale=cfg.retrieveValue<float>("./demDistScale",1.0f);
	defaultDemVerticalShift=cfg.retrieveValue<float>("./defaultDemVerticalShift",-3.5f);
	std::string controlPipeName=cfg.retrieveString("./controlPipeName","");
	
	/* Process command line parameters: */
	bool printHelp=false;
	const char* frameFilePrefix=0;
	int windowIndex=0;
	renderSettings.push_back(RenderSettings());
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"h")==0)
				printHelp=true;
			else if(strcasecmp(argv[i]+1,"c")==0)
				{
				++i;
				cameraIndex=atoi(argv[i]);
				}
			else if(strcasecmp(argv[i]+1,"f")==0)
				{
				++i;
				frameFilePrefix=argv[i];
				}
			else if(strcasecmp(argv[i]+1,"s")==0)
				{
				++i;
				scale=atof(argv[i]);
				}
			else if(strcasecmp(argv[i]+1,"slf")==0)
				{
				++i;
				sandboxLayoutFileName=argv[i];
				}
			else if(strcasecmp(argv[i]+1,"er")==0)
				{
				++i;
				double elevationMin=atof(argv[i]);
				++i;
				double elevationMax=atof(argv[i]);
				elevationRange=Math::Interval<double>(elevationMin,elevationMax);
				}
			else if(strcasecmp(argv[i]+1,"hmp")==0)
				{
				/* Read height mapping plane coefficients: */
				haveHeightMapPlane=true;
				double hmp[4];
				for(int j=0;j<4;++j)
					{
					++i;
					hmp[j]=atof(argv[i]);
					}
				heightMapPlane=Plane(Plane::Vector(hmp),hmp[3]);
				heightMapPlane.normalize();
				}
			else if(strcasecmp(argv[i]+1,"nas")==0)
				{
				++i;
				numAveragingSlots=atoi(argv[i]);
				}
			else if(strcasecmp(argv[i]+1,"sp")==0)
				{
				++i;
				minNumSamples=atoi(argv[i]);
				++i;
				maxVariance=atoi(argv[i]);
				}
			else if(strcasecmp(argv[i]+1,"he")==0)
				{
				++i;
				hysteresis=float(atof(argv[i]));
				}
			else if(strcasecmp(argv[i]+1,"wts")==0)
				{
				for(int j=0;j<2;++j)
					{
					++i;
					wtSize[j]=(unsigned int)(atoi(argv[i]));
					}
				}
			else if(strcasecmp(argv[i]+1,"ws")==0)
				{
				++i;
				waterSpeed=atof(argv[i]);
				++i;
				waterMaxSteps=atoi(argv[i]);
				}
			else if(strcasecmp(argv[i]+1,"bwl")==0)
				{
				++i;
				baseWaterLevel=GLfloat(atof(argv[i]));
				}
			else if(strcasecmp(argv[i]+1,"rer")==0)
				{
				++i;
				double rainElevationMin=atof(argv[i]);
				++i;
				double rainElevationMax=atof(argv[i]);
				rainElevationRange=Math::Interval<double>(rainElevationMin,rainElevationMax);
				}
			else if(strcasecmp(argv[i]+1,"rs")==0)
				{
				++i;
				rainStrength=GLfloat(atof(argv[i]));
				}
			else if(strcasecmp(argv[i]+1,"evr")==0)
				{
				++i;
				evaporationRate=atof(argv[i]);
				}
			else if(strcasecmp(argv[i]+1,"dds")==0)
				{
				++i;
				demDistScale=float(atof(argv[i]));
				}
			else if(strcasecmp(argv[i]+1,"dvs")==0)
				{
				++i;
				defaultDemVerticalShift=float(atof(argv[i]));
				}
			else if(strcasecmp(argv[i]+1,"wi")==0)
				{
				++i;
				windowIndex=atoi(argv[i]);
				
				/* Extend the list of render settings if an index beyond the end is selected: */
				while(int(renderSettings.size())<=windowIndex)
					renderSettings.push_back(renderSettings.back());
				
				/* Disable fixed projector view on the new render settings: */
				renderSettings.back().fixProjectorView=false;
				}
			else if(strcasecmp(argv[i]+1,"fpv")==0)
				{
				renderSettings.back().fixProjectorView=true;
				if(i+1<argc&&argv[i+1][0]!='-')
					{
					/* Load the projector transformation file specified in the next argument: */
					++i;
					renderSettings.back().loadProjectorTransform(argv[i]);
					}
				}
			else if(strcasecmp(argv[i]+1,"nhs")==0)
				renderSettings.back().hillshade=false;
			else if(strcasecmp(argv[i]+1,"uhs")==0)
				renderSettings.back().hillshade=true;
			else if(strcasecmp(argv[i]+1,"ns")==0)
				renderSettings.back().useShadows=false;
			else if(strcasecmp(argv[i]+1,"us")==0)
				renderSettings.back().useShadows=true;
			else if(strcasecmp(argv[i]+1,"nhm")==0)
				{
				delete renderSettings.back().elevationColorMap;
				renderSettings.back().elevationColorMap=0;
				}
			else if(strcasecmp(argv[i]+1,"uhm")==0)
				{
				if(i+1<argc&&argv[i+1][0]!='-')
					{
					/* Load the height color map file specified in the next argument: */
					++i;
					renderSettings.back().loadHeightMap(argv[i]);
					}
				else
					{
					/* Load the default height color map: */
					renderSettings.back().loadHeightMap(CONFIG_DEFAULTHEIGHTCOLORMAPFILENAME);
					renderSettings.back().loadSlopeMap(CONFIG_DEFAULTSLOPECOLORMAPFILENAME);
					}
				}
			else if(strcasecmp(argv[i]+1,"usm")==0)
				{
				if(i+1<argc&&argv[i+1][0]!='-')
					{
					/* Load the slope color map file specified in the next argument: */
					++i;
					renderSettings.back().loadSlopeMap(argv[i]);
					}
				}
			else if(strcasecmp(argv[i]+1,"ncl")==0)
				renderSettings.back().useContourLines=false;
			else if(strcasecmp(argv[i]+1,"ucl")==0)
				{
				renderSettings.back().useContourLines=true;
				if(i+1<argc&&argv[i+1][0]!='-')
					{
					/* Read the contour line spacing: */
					++i;
					renderSettings.back().contourLineSpacing=GLfloat(atof(argv[i]));
					}
				}
			else if(strcasecmp(argv[i]+1,"rws")==0)
				renderSettings.back().renderWaterSurface=true;
			else if(strcasecmp(argv[i]+1,"rwt")==0)
				renderSettings.back().renderWaterSurface=false;
			else if(strcasecmp(argv[i]+1,"wo")==0)
				{
				++i;
				renderSettings.back().waterOpacity=GLfloat(atof(argv[i]));
				}
			else if(strcasecmp(argv[i]+1,"veg")==0)
				{
				renderSettings.back().useVegetation=true;
				}
			else if(strcasecmp(argv[i]+1,"slp")==0)
				{
				renderSettings.back().showSlope=true;
				}
			else if(strcasecmp(argv[i]+1,"cp")==0)
				{
				++i;
				controlPipeName=argv[i];
				}
			else
				std::cerr<<"Ignoring unrecognized command line switch "<<argv[i]<<std::endl;
			}
		}
	
	/* Print usage help if requested: */
	if(printHelp)
		printUsage();
	
	if(frameFilePrefix!=0)
		{
		/* Open the selected pre-recorded 3D video files: */
		std::string colorFileName=frameFilePrefix;
		colorFileName.append(".color");
		std::string depthFileName=frameFilePrefix;
		depthFileName.append(".depth");
		camera=new Kinect::FileFrameSource(Vrui::openFile(colorFileName.c_str()),Vrui::openFile(depthFileName.c_str()));
		}
	else
		{
		/* Open the 3D camera device of the selected index: */
		Kinect::DirectFrameSource* realCamera=Kinect::openDirectFrameSource(cameraIndex);
		Misc::ConfigurationFileSection cameraConfigurationSection=cfg.getSection(cameraConfiguration.c_str());
		realCamera->configure(cameraConfigurationSection);
		camera=realCamera;
		}
	for(int i=0;i<2;++i)
		frameSize[i]=camera->getActualFrameSize(Kinect::FrameSource::DEPTH)[i];
	
	/* Get the camera's per-pixel depth correction parameters and evaluate it on the depth frame's pixel grid: */
	Kinect::FrameSource::DepthCorrection* depthCorrection=camera->getDepthCorrectionParameters();
	if(depthCorrection!=0)
		{
		pixelDepthCorrection=depthCorrection->getPixelCorrection(frameSize);
		delete depthCorrection;
		}
	else
		{
		/* Create dummy per-pixel depth correction parameters: */
		pixelDepthCorrection=new PixelDepthCorrection[frameSize[1]*frameSize[0]];
		PixelDepthCorrection* pdcPtr=pixelDepthCorrection;
		for(unsigned int y=0;y<frameSize[1];++y)
			for(unsigned int x=0;x<frameSize[0];++x,++pdcPtr)
				{
				pdcPtr->scale=1.0f;
				pdcPtr->offset=0.0f;
				}
		}
	
	/* Get the camera's intrinsic parameters: */
	cameraIps=camera->getIntrinsicParameters();
	
	/* Read the sandbox layout file: */
	Geometry::Plane<double,3> basePlane;
	Geometry::Point<double,3> basePlaneCorners[4];
	{
	IO::ValueSource layoutSource(Vrui::openFile(sandboxLayoutFileName.c_str()));
	layoutSource.skipWs();
	std::string s=layoutSource.readLine();
	basePlane=Misc::ValueCoder<Geometry::Plane<double,3> >::decode(s.c_str(),s.c_str()+s.length());
	basePlane.normalize();
	for(int i=0;i<4;++i)
		{
		layoutSource.skipWs();
		s=layoutSource.readLine();
		basePlaneCorners[i]=Misc::ValueCoder<Geometry::Point<double,3> >::decode(s.c_str(),s.c_str()+s.length());
		}
	}
	
	/* Limit the valid elevation range to the intersection of the extents of all height color maps: */
	for(std::vector<RenderSettings>::iterator rsIt=renderSettings.begin();rsIt!=renderSettings.end();++rsIt)
		if(rsIt->elevationColorMap!=0)
			{
			Math::Interval<double> mapRange(rsIt->elevationColorMap->getScalarRangeMin(),rsIt->elevationColorMap->getScalarRangeMax());
			elevationRange.intersectInterval(mapRange);
			}
	
	/* Scale all sizes by the given scale factor: */
	double sf=scale/100.0; // Scale factor from cm to final units
	for(int i=0;i<3;++i)
		for(int j=0;j<4;++j)
			cameraIps.depthProjection.getMatrix()(i,j)*=sf;
	basePlane=Geometry::Plane<double,3>(basePlane.getNormal(),basePlane.getOffset()*sf);
	for(int i=0;i<4;++i)
		for(int j=0;j<3;++j)
			basePlaneCorners[i][j]*=sf;
	if(elevationRange!=Math::Interval<double>::full)
		elevationRange*=sf;
	if(rainElevationRange!=Math::Interval<double>::full)
		rainElevationRange*=sf;
	for(std::vector<RenderSettings>::iterator rsIt=renderSettings.begin();rsIt!=renderSettings.end();++rsIt)
		{
		if(rsIt->elevationColorMap!=0)
			rsIt->elevationColorMap->setScalarRange(rsIt->elevationColorMap->getScalarRangeMin()*sf,rsIt->elevationColorMap->getScalarRangeMax()*sf);
		rsIt->contourLineSpacing*=sf;
		rsIt->waterOpacity/=sf;
		for(int i=0;i<4;++i)
			rsIt->projectorTransform.getMatrix()(i,3)*=sf;
		}
	rainStrength*=sf;
	evaporationRate*=sf;
	demDistScale*=sf;
	
	/* Create the frame filter object: */
	frameFilter=new FrameFilter(frameSize,numAveragingSlots,pixelDepthCorrection,cameraIps.depthProjection,basePlane);
	frameFilter->setValidElevationInterval(cameraIps.depthProjection,basePlane,elevationRange.getMin(),elevationRange.getMax());
	frameFilter->setStableParameters(minNumSamples,maxVariance);
	frameFilter->setHysteresis(hysteresis);
	frameFilter->setSpatialFilter(true);
	frameFilter->setOutputFrameFunction(Misc::createFunctionCall(this,&Sandbox::receiveFilteredFrame));
	
	if(waterSpeed>0.0)
		{
		/* Create the hand extractor object: */
		handExtractor=new HandExtractor(frameSize,pixelDepthCorrection,cameraIps.depthProjection);
		}
	
	/* Start streaming depth frames: */
	camera->startStreaming(0,Misc::createFunctionCall(this,&Sandbox::rawDepthFrameDispatcher));
	
	/* Create the depth image renderer: */
	depthImageRenderer=new DepthImageRenderer(frameSize);
	depthImageRenderer->setDepthProjection(cameraIps.depthProjection);
	depthImageRenderer->setBasePlane(basePlane);
	
	/* Calculate the transformation from camera space to sandbox space: */
	{
	ONTransform::Vector z=basePlane.getNormal();
	ONTransform::Vector x=(basePlaneCorners[1]-basePlaneCorners[0])+(basePlaneCorners[3]-basePlaneCorners[2]);
	ONTransform::Vector y=z^x;
	boxTransform=ONTransform::rotate(Geometry::invert(ONTransform::Rotation::fromBaseVectors(x,y)));
	ONTransform::Point center=Geometry::mid(Geometry::mid(basePlaneCorners[0],basePlaneCorners[1]),Geometry::mid(basePlaneCorners[2],basePlaneCorners[3]));
	boxTransform*=ONTransform::translateToOriginFrom(basePlane.project(center));
	}
	
	/* Calculate a bounding box around all potential surfaces: */
	bbox=Box::empty;
	for(int i=0;i<4;++i)
		{
		bbox.addPoint(basePlane.project(basePlaneCorners[i])+basePlane.getNormal()*elevationRange.getMin());
		bbox.addPoint(basePlane.project(basePlaneCorners[i])+basePlane.getNormal()*elevationRange.getMax());
		}
	
	if(waterSpeed>0.0)
		{
		/* Initialize the water flow simulator: */
		waterTable=new WaterTable2(wtSize[0],wtSize[1],depthImageRenderer,basePlaneCorners);
		waterTable->setElevationRange(elevationRange.getMin(),rainElevationRange.getMax());
		waterTable->setWaterDeposit(evaporationRate);

		/* Set up base water level */
		if(baseWaterLevelSlider!=0)
			baseWaterLevelSlider->setValue(baseWaterLevel);
		
		/* Register a render function with the water table: */
		addWaterFunction=Misc::createFunctionCall(this,&Sandbox::addWater);
		waterTable->addRenderFunction(addWaterFunction);
		addWaterFunctionRegistered=true;
		
		earthquakeManager = new EarthquakeManager();
		}
	
	/* Initialize all surface renderers: */
	for(std::vector<RenderSettings>::iterator rsIt=renderSettings.begin();rsIt!=renderSettings.end();++rsIt)
		{
		/* Calculate the texture mapping plane for this renderer's height map: */
		if(rsIt->elevationColorMap!=0)
			{
			if(haveHeightMapPlane)
				rsIt->elevationColorMap->calcTexturePlane(heightMapPlane);
			else
				rsIt->elevationColorMap->calcTexturePlane(depthImageRenderer);
			}
		
		/* Initialize the surface renderer: */
		rsIt->surfaceRenderer=new SurfaceRenderer(depthImageRenderer);
		rsIt->surfaceRenderer->setDrawContourLines(rsIt->useContourLines);
		rsIt->surfaceRenderer->setContourLineDistance(rsIt->contourLineSpacing);
		rsIt->surfaceRenderer->setElevationColorMap(rsIt->elevationColorMap);
		rsIt->surfaceRenderer->setSlopeColorMap(rsIt->slopeColorMap);
		rsIt->surfaceRenderer->setIlluminate(rsIt->hillshade);
		rsIt->surfaceRenderer->setVegetation(rsIt->useVegetation);
		rsIt->surfaceRenderer->setShowSlope(rsIt->showSlope);
		if(waterTable!=0)
			{
			if(rsIt->renderWaterSurface)
				{
				/* Create a water renderer: */
				rsIt->waterRenderer=new WaterRenderer(waterTable);
				rsIt->surfaceRenderer->setWaterTable(waterTable);
				rsIt->surfaceRenderer->setAdvectWaterTexture(true);
				rsIt->surfaceRenderer->setWaterOpacity(rsIt->waterOpacity);
				}
			else
				{
				rsIt->surfaceRenderer->setWaterTable(waterTable);
				rsIt->surfaceRenderer->setAdvectWaterTexture(true);
				rsIt->surfaceRenderer->setWaterOpacity(rsIt->waterOpacity);
				}
			}
		rsIt->surfaceRenderer->setDemDistScale(demDistScale);
		}
	
	#if 0
	/* Create a fixed-position light source: */
	sun=Vrui::getLightsourceManager()->createLightsource(true);
	for(int i=0;i<Vrui::getNumViewers();++i)
		Vrui::getViewer(i)->setHeadlightState(false);
	sun->enable();
	sun->getLight().position=GLLight::Position(1,0,1,0);
	#endif
	
	/* Create the GUI: */
	mainMenu=createMainMenu();
	Vrui::setMainMenu(mainMenu);
	if(waterTable!=0)
		{
		waterControlDialog=createWaterControlDialog();
		earthquakeControlDialog=createEarthquakeControlDialog();
		demControlDialog=createDemControlDialog();
		}
	
	/* Initialize the custom tool classes: */
	DEMTool::initClass(*Vrui::getToolManager());
	ImageTool::initClass(*Vrui::getToolManager());
	SlopeTool::initClass(*Vrui::getToolManager());
	if(waterTable!=0)
		{
		BathymetrySaverTool::initClass(waterTable,*Vrui::getToolManager());
		EarthquakeTool::initClass(waterTable, earthquakeManager, *Vrui::getToolManager());
		GlobalWaterTool::initClass(*Vrui::getToolManager());
		LocalWaterTool::initClass(*Vrui::getToolManager());
		WaterLevelTool::initClass(*Vrui::getToolManager());
		AddVegetationTool::initClass(*Vrui::getToolManager());
		}
	addEventTool("Pause Topography",0,0);
	
	if(!controlPipeName.empty())
		{
		/* Open the control pipe in non-blocking mode: */
		controlPipeFd=open(controlPipeName.c_str(),O_RDONLY|O_NONBLOCK);
		if(controlPipeFd<0)
			std::cerr<<"Unable to open control pipe "<<controlPipeName<<"; ignoring"<<std::endl;
		}
	
	/* Inhibit the screen saver: */
	Vrui::inhibitScreenSaver();
	
	/* Set the linear unit to support proper scaling: */
	Vrui::getCoordinateManager()->setUnit(Geometry::LinearUnit(Geometry::LinearUnit::METER,scale/100.0));
	
	/* Initialize the navigation transformation: */
	Vrui::Point::AffineCombiner cc;
	for(int i=0;i<4;++i)
		cc.addPoint(Vrui::Point(basePlane.project(basePlaneCorners[i])));
	navCenter=cc.getPoint();
	navSize=Vrui::Scalar(0);
	for(int i=0;i<4;++i)
		{
		Vrui::Scalar dist=Geometry::dist(Vrui::Point(basePlane.project(basePlaneCorners[i])),navCenter);
		if(navSize<dist)
			navSize=dist;
		}
	navUp=Geometry::normal(Vrui::Vector(basePlane.getNormal()));
	}

Sandbox::~Sandbox(void)
	{
	/* Stop streaming depth frames: */
	camera->stopStreaming();
	delete camera;
	delete frameFilter;
	
	/* Delete helper objects: */
	if (waterTable != 0)
		delete earthquakeManager;
	delete waterTable;
	delete depthImageRenderer;
	delete handExtractor;
	delete addWaterFunction;
	delete[] pixelDepthCorrection;
	
	delete mainMenu;
	delete waterControlDialog;
	delete earthquakeControlDialog;
	delete demControlDialog;
	
	close(controlPipeFd);
	}

void Sandbox::toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData)
	{
	/* Check if the destroyed tool is the active DEM tool: */
	if(activeDem==dynamic_cast<DEM*>(cbData->tool))
		{
		/* Deactivate the active DEM tool: */
		activeDem=0;
		}
	}

void Sandbox::frame(void)
	{
	/* Check if the filtered frame has been updated: */
	if(filteredFrames.lockNewValue())
		{
		/* Update the depth image renderer's depth image: */
		depthImageRenderer->setDepthImage(filteredFrames.getLockedValue());
		}
	
	if(handExtractor!=0)
		{
		/* Lock the most recent extracted hand list: */
		handExtractor->lockNewExtractedHands();
		
		#if 0
		
		/* Register/unregister the rain rendering function based on whether hands have been detected: */
		bool registerWaterFunction=!handExtractor->getLockedExtractedHands().empty();
		if(addWaterFunctionRegistered!=registerWaterFunction)
			{
			if(registerWaterFunction)
				waterTable->addRenderFunction(addWaterFunction);
			else
				waterTable->removeRenderFunction(addWaterFunction);
			addWaterFunctionRegistered=registerWaterFunction;
			}
		
		#endif
		}
	
	/* Update all surface renderers: */
	for(std::vector<RenderSettings>::iterator rsIt=renderSettings.begin();rsIt!=renderSettings.end();++rsIt)
		rsIt->surfaceRenderer->setAnimationTime(Vrui::getApplicationTime());

	/* Update all color maps: */
	for(std::vector<RenderSettings>::iterator rsIt=renderSettings.begin();rsIt!=renderSettings.end();++rsIt)
		{
	      if(rsIt->elevationColorMap!=0) rsIt->elevationColorMap->load(CONFIG_DEFAULTHEIGHTCOLORMAPFILENAME);
	      if(rsIt->slopeColorMap!=0) rsIt->slopeColorMap->load(CONFIG_DEFAULTSLOPECOLORMAPFILENAME);
		}
		
	/* Check if there is a control command on the control pipe: */
	if(controlPipeFd>=0)
		{
		/* Try reading a chunk of data (will fail with EAGAIN if no data due to non-blocking access): */
		char command[1024];
		ssize_t readResult=read(controlPipeFd,command,sizeof(command)-1);
		if(readResult>0)
			{
			command[readResult]='\0';
			
			/* Extract the command: */
			char* cPtr;
			for(cPtr=command;*cPtr!='\0'&&!isspace(*cPtr);++cPtr)
				;
			char* commandEnd=cPtr;
			
			/* Find the beginning of an optional command parameter: */
			while(*cPtr!='\0'&&isspace(*cPtr))
				++cPtr;
			char* parameter=cPtr;
			
			/* Find the end of the optional parameter list: */
			while(*cPtr!='\0')
				++cPtr;
			while(cPtr>parameter&&isspace(cPtr[-1]))
				--cPtr;
			*cPtr='\0';
			
			/* Parse the command: */
			*commandEnd='\0';
			if(strcasecmp(command,"waterSpeed")==0)
				{
				waterSpeed=atof(parameter);
				if(waterSpeedSlider!=0)
					waterSpeedSlider->setValue(waterSpeed);
				}
			else if(strcasecmp(command,"waterMaxSteps")==0)
				{
				waterMaxSteps=atoi(parameter);
				if(waterMaxStepsSlider!=0)
					waterMaxStepsSlider->setValue(waterMaxSteps);
				}
			else if(strcasecmp(command,"waterAttenuation")==0)
				{
				double attenuation=atof(parameter);
				if(waterTable!=0)
					waterTable->setAttenuation(GLfloat(1.0-attenuation));
				if(waterAttenuationSlider!=0)
					waterAttenuationSlider->setValue(attenuation);
				}
			else if(strcasecmp(command,"baseWaterLevel")==0)
				{
				double baseWaterLevel=atof(parameter);
				if(waterTable!=0)
					waterTable->setBaseWaterLevel(GLfloat(baseWaterLevel));
				if(baseWaterLevelSlider!=0)
					baseWaterLevelSlider->setValue(baseWaterLevel);
				}
			else if(strcasecmp(command,"colorMap")==0)
				{
				try
					{
					/* Update all height color maps: */
					for(std::vector<RenderSettings>::iterator rsIt=renderSettings.begin();rsIt!=renderSettings.end();++rsIt)
						if(rsIt->elevationColorMap!=0)
							rsIt->elevationColorMap->load(parameter);
					}
				catch(std::runtime_error err)
					{
					std::cerr<<"Cannot read height color map "<<parameter<<" due to exception "<<err.what()<<std::endl;
					}
				}
			else if(strcasecmp(command,"heightMapPlane")==0)
				{
				/* Read the height map plane equation: */
				double hmp[4];
				char* endPtr=parameter;
				for(int i=0;i<4;++i)
					hmp[i]=strtod(endPtr,&endPtr);
				Plane heightMapPlane=Plane(Plane::Vector(hmp),hmp[3]);
				heightMapPlane.normalize();
				
				/* Override the height mapping planes of all elevation color maps: */
				for(std::vector<RenderSettings>::iterator rsIt=renderSettings.begin();rsIt!=renderSettings.end();++rsIt)
					if(rsIt->elevationColorMap!=0)
						rsIt->elevationColorMap->calcTexturePlane(heightMapPlane);
				}
			}
		}
	
	if(frameRateTextField!=0&&Vrui::getWidgetManager()->isVisible(waterControlDialog))
		{
		/* Update the frame rate display: */
		frameRateTextField->setValue(1.0/Vrui::getCurrentFrameTime());
		}
	
	if(pauseUpdates)
		Vrui::scheduleUpdate(Vrui::getApplicationTime()+1.0/30.0);
	}

void Sandbox::display(GLContextData& contextData) const
	{
	/* Get the data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Get the rendering settings for this window: */
	const Vrui::DisplayState& ds=Vrui::getDisplayState(contextData);
	const Vrui::VRWindow* window=ds.window;
	int windowIndex;
	for(windowIndex=0;windowIndex<Vrui::getNumWindows()&&window!=Vrui::getWindow(windowIndex);++windowIndex)
		;
	const RenderSettings& rs=windowIndex<int(renderSettings.size())?renderSettings[windowIndex]:renderSettings.back();
	
	/* Check if the water simulation state needs to be updated: */
	if(waterTable!=0&&dataItem->waterTableTime!=Vrui::getApplicationTime())
		{
		/* Update the water table's bathymetry grid: */
		if (earthquakeManager->hasBathymetryGrid()) 
			{
			GLfloat *bathymetryGrid = earthquakeManager->getBathymetryGrid();
			waterTable->updateBathymetry((const GLfloat *) bathymetryGrid, contextData);
			earthquakeManager->setBathymetryGrid(NULL);
			}
		else {
			waterTable->updateBathymetry(contextData);
		
		}
		
		/* Run the water flow simulation's main pass: */
		GLfloat totalTimeStep=GLfloat(Vrui::getFrameTime()*waterSpeed);
		unsigned int numSteps=0;
		while(numSteps<waterMaxSteps-1U&&totalTimeStep>1.0e-8f)
			{
			/* Run with a self-determined time step to maintain stability: */
			waterTable->setMaxStepSize(totalTimeStep);
			GLfloat timeStep=waterTable->runSimulationStep(false,contextData);
			totalTimeStep-=timeStep;
			++numSteps;
			}
			
		if(rs.useVegetation)
			{
			/* Run vegetation simulation */
			waterTable->runVegetationSimulation(contextData);
			}
		
		/* Mark the water simulation state as up-to-date for this frame: */
		dataItem->waterTableTime=Vrui::getApplicationTime();
		
		if (waterTable->bathymetryIsInitialized())
			waterTable->setBaseWaterLevel(baseWaterLevel);
		}
	
	/* Calculate the projection matrix: */
	PTransform projection=ds.projection;
	if(rs.fixProjectorView&&rs.projectorTransformValid)
		{
		/* Use the projector transformation instead: */
		projection=rs.projectorTransform;
		
		/* Multiply with the inverse modelview transformation so that lighting still works as usual: */
		projection*=Geometry::invert(ds.modelviewNavigational);
		}
	
	if(rs.hillshade)
		{
		/* Set the surface material: */
		glMaterial(GLMaterialEnums::FRONT,rs.surfaceMaterial);
		}
	
	{
	/* Render the surface in a single pass: */
	rs.surfaceRenderer->renderSinglePass(ds.viewport,projection,ds.modelviewNavigational,contextData);
	}
	
	if(rs.waterRenderer!=0)
		{
		/* Draw the water surface: */
		glMaterialAmbientAndDiffuse(GLMaterialEnums::FRONT,GLColor<GLfloat,4>(0.0f,0.5f,0.8f));
		glMaterialSpecular(GLMaterialEnums::FRONT,GLColor<GLfloat,4>(1.0f,1.0f,1.0f));
		glMaterialShininess(GLMaterialEnums::FRONT,64.0f);
		rs.waterRenderer->render(projection,ds.modelviewNavigational,contextData);
		}
	}

void Sandbox::resetNavigation(void)
	{
	/* Set the navigation transformation from the previously computed parameters: */
	Vrui::setNavigationTransformation(navCenter,navSize,navUp);
	}

void Sandbox::eventCallback(Vrui::Application::EventID eventId,Vrui::InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState)
		{
		switch(eventId)
			{
			case 0:
				/* Invert the current pause setting: */
				pauseUpdates=!pauseUpdates;
				
				/* Update the main menu toggle: */
				pauseUpdatesToggle->setToggle(pauseUpdates);
				
				break;
			}
		}
	}

void Sandbox::initContext(GLContextData& contextData) const
	{
	/* Create a data item and add it to the context: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	{
	/* Save the currently bound frame buffer: */
	GLint currentFrameBuffer;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT,&currentFrameBuffer);
	
	/* Set the default shadow buffer size: */
	dataItem->shadowBufferSize[0]=1024;
	dataItem->shadowBufferSize[1]=1024;
	
	/* Generate the shadow rendering frame buffer: */
	glGenFramebuffersEXT(1,&dataItem->shadowFramebufferObject);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,dataItem->shadowFramebufferObject);
	
	/* Generate a depth texture for shadow rendering: */
	glGenTextures(1,&dataItem->shadowDepthTextureObject);
	glBindTexture(GL_TEXTURE_2D,dataItem->shadowDepthTextureObject);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_MODE_ARB,GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_FUNC_ARB,GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D,GL_DEPTH_TEXTURE_MODE_ARB,GL_INTENSITY);
	glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24_ARB,dataItem->shadowBufferSize[0],dataItem->shadowBufferSize[1],0,GL_DEPTH_COMPONENT,GL_UNSIGNED_BYTE,0);
	glBindTexture(GL_TEXTURE_2D,0);
	
	/* Attach the depth texture to the frame buffer object: */
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,dataItem->shadowDepthTextureObject,0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,currentFrameBuffer);
	} 
	}

VRUI_APPLICATION_RUN(Sandbox)
