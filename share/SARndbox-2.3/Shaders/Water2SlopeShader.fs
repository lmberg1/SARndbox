/***********************************************************************
Water2SlopeShader - Shader to compute spatial partial derivatives of the
conserved quantities.
Copyright (c) 2012 Oliver Kreylos

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

uniform vec2 cellSize;
uniform float theta;
uniform sampler2DRect bathymetrySampler;
	
/* Calculate magnitude slope */
float slope(in float b0, in float b1, in float b2, in float cellSize)
	{
	/* Calculate the left, central, and right differences: */
	float d01=abs(b1-b0)*(theta/cellSize);
	float d02=abs(b2-b0)/(2.0*cellSize);
	float d12=abs(b2-b1)*(theta/cellSize);
	
	/* Calculate the component-wise intervals: */
	float dMin=min(min(d01,d02),d12);
	float dMax=max(max(d01,d02),d12);
	
	/* Assemble the result: */
	float result;
	result=dMin>0.0?dMin:dMax<0.0?dMax:0.0;
	
	return result;
	}

void main()
	{
	/* Get the bathymetry elevation at the cell's corners: */
	float b0=texture2DRect(bathymetrySampler,vec2(gl_FragCoord.x,gl_FragCoord.y-1.0)).r;
	float b1=texture2DRect(bathymetrySampler,vec2(gl_FragCoord.x-1.0,gl_FragCoord.y)).r;
	float b2=texture2DRect(bathymetrySampler,gl_FragCoord.xy).r;
	float b3=texture2DRect(bathymetrySampler,vec2(gl_FragCoord.x+1.0,gl_FragCoord.y)).r;
	float b4=texture2DRect(bathymetrySampler,vec2(gl_FragCoord.x,gl_FragCoord.y+1.0)).r;

	/* Calculate the x-direction slope: */
	float slopeX=slope(b1, b2, b3, cellSize.x);
	
	/* Calculate the y-direction slope: */
	float slopeY=slope(b0, b2, b4, cellSize.y);
	
	float maxSlope=max(slopeX, slopeY);
	
	gl_FragColor=vec4(maxSlope, 0.0, 0.0, 0.0);
	}
