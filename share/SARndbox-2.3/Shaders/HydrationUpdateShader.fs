/***********************************************************************
HydrationUpdateShader - Shader to update the current hydration level
Copyright (c) 2012 Oliver Kreylos
Modified by Simon Johansson 2015
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

uniform sampler2DRect bathymetrySampler;
uniform float baseWaterLevel;

void main()
	{
	if (baseWaterLevel == -100.0)
		{
		/* Set hydration to 0.0 if the base water level hasn't been set */
		gl_FragColor=vec4(0.0, 0.0, 0.0, 0.0);
		}
	else
		{
		/* Calculate the bathymetry elevation at the center of this cell: */
		float b=(texture2DRect(bathymetrySampler,vec2(gl_FragCoord.x-1.0,gl_FragCoord.y-1.0)).r+
	         texture2DRect(bathymetrySampler,vec2(gl_FragCoord.x,gl_FragCoord.y-1.0)).r+
	         texture2DRect(bathymetrySampler,vec2(gl_FragCoord.x-1.0,gl_FragCoord.y)).r+
	         texture2DRect(bathymetrySampler,vec2(gl_FragCoord.xy)).r)*0.25;
		
		/* Hydration is proportional to the inverse square of height above sea level */
		float elevation = max(0.0, b - baseWaterLevel);
		float hydration = pow(1.0/(elevation+1.1), 2.0);
		
		gl_FragColor = vec4(hydration, 0.0, 0.0, 0.0);
		}	
}
