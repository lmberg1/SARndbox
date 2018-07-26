/***********************************************************************
VegetationUpdateShader - Shader to update the current vegetation level
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

uniform sampler2DRect vegetationSampler;
uniform sampler2DRect hydrationSampler;
uniform sampler2DRect quantitySampler;
uniform sampler2DRect bathymetrySampler;
uniform sampler2DRect randomSampler;
uniform float vegetationRange;
uniform float growthRate;
uniform float hydrationThreshold;
uniform float color;
uniform bool clearVeg;

float distBetween(in float x1, in float y1, in float x2, in float y2)
	{
	float dist = sqrt(pow(x1-x2, 2.0) + pow(y1-y2, 2.0));
	return dist;
	}

void main()
	{
	float hydration=texture2DRect(hydrationSampler, gl_FragCoord.xy).r;
	float vegetation=texture2DRect(vegetationSampler, gl_FragCoord.xy).r;
	
	/* Calculate the bathymetry elevation at the center of this cell: */
	float b=(texture2DRect(bathymetrySampler,vec2(gl_FragCoord.x-1.0,gl_FragCoord.y-1.0)).r+
	         texture2DRect(bathymetrySampler,vec2(gl_FragCoord.x,gl_FragCoord.y-1.0)).r+
	         texture2DRect(bathymetrySampler,vec2(gl_FragCoord.x-1.0,gl_FragCoord.y)).r+
	         texture2DRect(bathymetrySampler,vec2(gl_FragCoord.xy)).r)*0.25;
	
	/* Get the old quantity at the cell center: */
	vec3 q=texture2DRect(quantitySampler,gl_FragCoord.xy).rgb;
	
	/* Calculate the water column height: */
	float waterHeight=q.x-b;
	
	/* Vegetation cannot grow if insufficient hydration or if it's underwater
	   Set vegetation to zero if clearVeg flag is true */
	if (hydration <= hydrationThreshold || waterHeight > 0.01 || clearVeg) 
		gl_FragColor=vec4(0.0,0.0,0.0,0.0);
	
	/* Don't change vegetation if it already exists */
	else if (vegetation > 0.0)
		gl_FragColor=vec4(vegetation, 0.0, 0.0, 0.0);
	
	/* Grow vegetation based on probability of growth */
	else
		{
		/* Calculate proximity to other pieces of vegetation */
		float proximity = 0.0;
		float x = gl_FragCoord.x;
		float y = gl_FragCoord.y;
		for (int i = -vegetationRange; i < vegetationRange; i++)
			for (int j = -vegetationRange; j < vegetationRange; j++)
			{
			float dist = distBetween(x, y, x+i, y+j);
			float v = texture2DRect(vegetationSampler, vec2(x+i, y+j)).r;
			
			if (dist <= vegetationRange && v > 0.0)
				proximity = max(proximity, (vegetationRange - dist)/vegetationRange);
			}
		
		/* probability increases with closer proximity to other vegetation */
		float proximityWeight = 6.0;
		float probability = pow(proximity, proximityWeight);
	
		/* probability increases with higher level of hydration */
		float hydrationWeight = 5.0;
		probability *= pow(hydration, hydrationWeight);
		
		/* probability increases with a higher growth rate */
		probability *= growthRate;
		
		/* vegetation grows if the random value generated for that pixel
		   falls under the probability of growth */
		float random=texture2DRect(randomSampler, gl_FragCoord.xy).r;
		if (random < probability)
			gl_FragColor=vec4(color,0.0,0.0,0.0);
		}
}
