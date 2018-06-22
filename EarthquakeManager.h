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

#ifndef EARTHQUAKEMANAGER_INCLUDED
#define EARTHQUAKEMANAGER_INCLUDED

#include <GL/gl.h>

class EarthquakeManager
	{
	private:
	/* Elements: */
	GLfloat* bathymetryGrid; // bathymetry grid of grid size - 1
	unsigned int radius; // radius of earthquake in pixels
	double perturbation; // amount of perturbation in cm
	
	/* Constructors and destructors: */
	public:
	EarthquakeManager(void);
	virtual ~EarthquakeManager(void);
	
	/* New Methods */
	bool hasBathymetryGrid() const; // returns if a bathymetry grid is set
	void setBathymetryGrid(GLfloat* newBathymetryGrid); // sets a new bathymetry grid
	void setEarthquakePerturbation(double newPerturbation); // sets a new perturbation
	void setEarthquakeRadius(unsigned int newRadius); // sets a new earthquake radius in pixels
	GLfloat *getBathymetryGrid(); // returns the current bathymetry grid
	unsigned int getEarthquakeRadius(); // returns the current earthquake radius
	double getEarthquakePerturbation(); // returns the current perturbation
	};

#endif
