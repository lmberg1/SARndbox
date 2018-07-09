/***********************************************************************
SurfaceAddSlopeColor - Shader fragment to add slope color
Simon Johansson 2015
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
Slope shading function
***************************/
uniform sampler2DRect slopeSampler;

varying vec2 waterTexCoord; // Texture coordinate for water level

void addSlopeColor(in vec2 fragCoord,inout vec4 baseColor)
	{
	float slope = texture2DRect(slopeSampler, waterTexCoord).r;

	vec4 color;
	if (slope < 0.2)
		color = vec4(255.0/255.0, 255.0/255.0, 255.0/255.0, 1.0);
	else if (slope < 0.4)
		color = vec4(255.0/255.0, 200.0/255.0, 200.0/255.0, 1.0);
	else if (slope < 0.6)
		color = vec4(255.0/255.0, 100.0/255.0, 150.0/255.0, 1.0);
	else if (slope < 0.8)
		color = vec4(255.0/255.0, 50.0/255.0, 100.0/255.0, 1.0);
	else if (slope < 1.0)
		color = vec4(255.0/255.0, 0.0/255.0, 0.0/255.0, 1.0);
	else
		color = vec4(150.0/255.0, 0.0/255.0, 0.0/255.0, 1.0);
		
	baseColor=mix(baseColor, color, 1.0);
	}
