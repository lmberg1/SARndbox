/***********************************************************************
EarthquakeManager - Class to set and store variables related to earthquake
simulation

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

#include "EarthquakeManager.h"

#include <GL/gl.h>

/**********************************
Methods of class EarthquakeManager:
**********************************/

EarthquakeManager::EarthquakeManager(void)
	:bathymetryGrid(NULL),
	radius(10), perturbation(10.0)
	{
	}

EarthquakeManager::~EarthquakeManager(void)
	{
	}
	
bool EarthquakeManager::hasBathymetryGrid() const 
	{
	return (bathymetryGrid != NULL);
	}

void EarthquakeManager::setBathymetryGrid(GLfloat* newBathymetryGrid) 
	{
	bathymetryGrid=newBathymetryGrid;
	}

void EarthquakeManager::setEarthquakeRadius(unsigned int newRadius)
	{
	radius=newRadius;
	}
	
void EarthquakeManager::setEarthquakePerturbation(double newPerturbation)
	{
	perturbation=newPerturbation;
	}
	
GLfloat* EarthquakeManager::getBathymetryGrid() 
	{
	return bathymetryGrid;
	}

unsigned int EarthquakeManager::getEarthquakeRadius()
	{
	return radius;
	}

double EarthquakeManager::getEarthquakePerturbation()
	{
	return perturbation;
	}
	
