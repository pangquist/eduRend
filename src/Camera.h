
//
//  Camera.h
//
//	Basic camera class
//

#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include "vec\vec.h"
#include "vec\mat.h"
#include <iostream>

using namespace linalg;

class Camera
{
public:
	// Aperture attributes
	float vfov, aspect;	
	
	// Clip planes in view space coordinates
	// Evrything outside of [zNear, zFar] is clipped away on the GPU side
	// zNear should be > 0
	// zFar should depend on the size of the scene
	// This range should be kept as tight as possibly to improve
	// numerical precision in the z-buffer
	float zNear, zFar;	

	float dx, dy;
						
	vec3f position;
	mat4f rotation;

	Camera(
		float vfov,
		float aspect,
		float zNear,
		float zFar):
		vfov(vfov), aspect(aspect), zNear(zNear), zFar(zFar)
	{
		position = {0.0f, 0.0f, 0.0f};
		rotation = mat4f::rotation(0, 0, 0);
		dy = 0.0f;
		dx = 0.0f;
	}

	// Move to an absolute position
	//
	void moveTo(const vec3f& p)
	{
		position = p;
	}

	// Move relatively
	//
	void move(const vec3f& v)
	{
		position += ApplyMatrix(v, rotation);
	}

	void rotate(InputHandler* input_handler)
	{
		float mousedx = input_handler->GetMouseDeltaX() * (PI / 360);
		float mousedy = input_handler->GetMouseDeltaY() * (PI / 360);

		dx += mousedx;
		dy += mousedy;

		if (dx > PI * 2)
			dx = 0.0f;
		if (dy > PI * 2)
			dy = 0.0f;

		//std::cout << mousedx << ", " << mousedy << std::endl;

		if(dy < PI / 2 && dy > -PI / 2)
		rotation = mat4f::rotation(0, dx, dy);
	}

	// Return World-to-View matrix for this camera
	//
	mat4f get_WorldToViewMatrix()
	{
		// Assuming a camera's position and rotation is defined by matrices T(p) and R,
		// the View-to-World transform is T(p)*R (for a first-person style camera).
		//
		// World-to-View then is the inverse of T(p)*R;
		//		inverse(T(p)*R) = inverse(R)*inverse(T(p)) = transpose(R)*T(-p)
		// Since now there is no rotation, this matrix is simply T(-p)

		//return rotation.inverse() * mat4f::translation(position).inverse();

		//return mat4f::translation(-position);

		return (mat4f::translation(position) * rotation).inverse();
	}

	// Matrix transforming from View space to Clip space
	// In a performance sensitive situation this matrix should be precomputed
	// if possible
	//
	mat4f get_ProjectionMatrix()
	{
		return mat4f::projection(vfov, aspect, zNear, zFar);
	}

	vec3f ApplyMatrix(vec3f v, const mat4f& m)
	{
		vec3f res;

		res.x = m.array[0] * v.x + m.array[4] * v.y + m.array[8] * v.z + m.array[12];
		res.y = m.array[1] * v.x + m.array[5] * v.y + m.array[9] * v.z + m.array[13];
		res.z = m.array[2] * v.x + m.array[6] * v.y + m.array[10] * v.z + m.array[14];

		return res;
	}
};

#endif