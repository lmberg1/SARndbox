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
uniform sampler2DRect randomSampler;
uniform float growthRate;
uniform float color;

float distBetween(in float x1, in float y1, in float x2, in float y2)
	{
	float dist = sqrt(pow(x1-x2, 2.0) + pow(y1-y2, 2.0));
	return dist;
	}

void main()
	{
	float hydration=texture2DRect(hydrationSampler, gl_FragCoord.xy).r;
	float vegetation=texture2DRect(vegetationSampler, gl_FragCoord.xy).r;
	float random=texture2DRect(randomSampler, gl_FragCoord.xy).r;
	// Vegetation cannot grow if insufficient hydration or if there is already 
	// vegetation there
	if (hydration <= 0.01 || vegetation > 0.0) 
		gl_FragColor=vec4(vegetation,0.0,0.0,0.0);
	else
		{
		float x = gl_FragCoord.x;
		float y = gl_FragCoord.y;
	
		// minumum distance to another piece of vegetation to grow
		float range = 10.0; 
		float probability = 0.0;
		for (int i = -range; i < range; i++)
			for (int j = -range; j < range; j++)
			{
			float dist = distBetween(x, y, x+i, y+j);
			float v = texture2DRect(vegetationSampler, vec2(x+i, y+j)).r;
			if (dist <= range && v > 0.0)
				// probability of growth increases with proximity to 
				// other pieces of vegetation
				probability = max(probability, (range - dist)/range);
			}
	
		// vegetation is more likely to grow in areas of high hydration
		probability *= pow(hydration, 7.0);
		probability *= (growthRate);
		
		// vegetation grows if the random value generated for that pixel
		// falls under the probability of growth
		if (random < probability)
			gl_FragColor=vec4(color,0.0,0.0,0.0);
		else 
			gl_FragColor=vec4(vegetation,0.0,0.0,0.0);
		}
}
