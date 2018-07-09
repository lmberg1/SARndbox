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
uniform sampler2DRect quantitySampler;
uniform sampler2DRect prevHydrationSampler;
uniform float hydrationRange;
uniform float detectionThreshold;
uniform float hydrationVelocity;
uniform float hydrationStepSize;

void main()
	{
	// Search the surronding pixels for water
	float n = 0.0;
	float maxHydration = 0.0;
	float range = hydrationRange;
	float start = -1.0*(range/2.0);
	float end = range/2.0;
	float step = hydrationStepSize;
	
	for(float i=start; i<end; i+=step){
		for(float j=start; j<end; j+=step){
			float x = gl_FragCoord.x + i;
			float y = gl_FragCoord.y + j;
			
			if (x <= 0 || x >= 640 || y <= 0 || y >= 480) continue;
			
			n++;
			
			/* Calculate the bathymetry elevation at the center of this cell: */
			float b=(texture2DRect(bathymetrySampler,vec2(x-1.0,y-1.0)).r+
				   texture2DRect(bathymetrySampler,vec2(x,y-1.0)).r+
				   texture2DRect(bathymetrySampler,vec2(x-1.0,y)).r+
				   texture2DRect(bathymetrySampler,vec2(x, y)).r)*0.25;
	
			/* Get the old quantity at the cell center: */
			vec3 q=texture2DRect(quantitySampler,vec2(x, y)).rgb;
	
			/* Calculate the water column height: */
			float h=q.x-b;
				
			// Water detection
			if (h > detectionThreshold){
				maxHydration += 1.0;
			}
		}
	}
	maxHydration = maxHydration/n; // The average water coverage
	float previousHydration = texture2DRect(prevHydrationSampler, gl_FragCoord.xy).r;	
	float velocity = hydrationVelocity;
	float newHydration = previousHydration + (maxHydration - previousHydration)*velocity;
	
	gl_FragColor=vec4(newHydration, 0.0, 0.0, 0.0);
}
