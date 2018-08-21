/***********************************************************************
SurfaceAddVegetationColor - Shader fragment to add vegetation color
Simon Johansson 2015
Modified by Lauren von Berg 2018
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

#extension GL_ARB_texture_rectangle : enable

/**************************
Vegetation shading function
***************************/
uniform sampler2DRect vegetationSampler;
uniform sampler1D vegetationColorMapSampler;
varying vec2 waterTexCoord; // Texture coordinate for vegetation

void addVegetationColor(in vec2 fragCoord,inout vec4 baseColor)
	{
	float veg = texture2DRect(vegetationSampler, waterTexCoord).r; // Read vegetation value
	
	/* Set vegetation color if vegetation exists */
	if (veg > 0.0)
		{
		vec4 vegColor=texture1D(vegetationColorMapSampler,veg);
		baseColor=mix(baseColor, vegColor, 0.8);
		}
	}
