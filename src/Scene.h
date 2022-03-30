
#pragma once
#ifndef SCENE_H
#define SCENE_H

#include "stdafx.h"
#include "InputHandler.h"
#include "Camera.h"
#include "Model.h"
#include "Texture.h"
#include "Cube.h"

// New files
// Material
// Texture <- stb

// TEMP


class Scene
{
protected:
	ID3D11Device*			dxdevice;
	ID3D11DeviceContext*	dxdevice_context;
	int						window_width;
	int						window_height;

public:

	Scene(
		ID3D11Device* dxdevice,
		ID3D11DeviceContext* dxdevice_context,
		int window_width,
		int window_height);

	virtual void Init() = 0;

	virtual void Update(
		float dt,
		InputHandler* input_handler) = 0;
	
	virtual void Render() = 0;
	
	virtual void Release() = 0;

	virtual void WindowResize(
		int window_width,
		int window_height);
};

class OurTestScene : public Scene
{
	//
	// Constant buffers (CBuffers) for data that is sent to shaders
	//

	// CBuffer for transformation matrices
	ID3D11Buffer* transformation_buffer = nullptr;
	// + other CBuffers
	ID3D11Buffer* light_buffer = nullptr;
	// 
	// CBuffer client-side definitions
	// These must match the corresponding shader definitions 
	//
	ID3D11SamplerState* samplerState = nullptr;
	std::vector<ID3D11SamplerState*> samplerStates;
	int samplerStateIndex = 0;

	struct TransformationBuffer
	{
		mat4f ModelToWorldMatrix;
		mat4f WorldToViewMatrix;
		mat4f ProjectionMatrix;
	};

	struct LightBuffer
	{
		vec4f LightSourcePosition;
		vec4f CameraPosition;
	};

	//
	// Scene content
	//
	Camera* camera;

	QuadModel* quad;
	OBJModel* sponza;
	Cube* cube;
	Cube* child;

	vec3f lightSource;
	float lightZ;
	bool lightDir;

	// Model-to-world transformation matrices
	mat4f Msponza;
	mat4f Mquad;
	mat4f Mcube;
	mat4f Mchild;

	// World-to-view matrix
	mat4f Mview;
	// Projection matrix
	mat4f Mproj;

	// Misc
	float angle = 0;			// A per-frame updated rotation angle (radians)...
	float angle_vel = fPI / 2;	// ...and its velocity (radians/sec)
	float camera_vel = 10.0f;	// Camera movement velocity in units/s
	float fps_cooldown = 0;

	void InitTransformationBuffer();

	void InitLightBuffer();

	void UpdateTransformationBuffer(
		mat4f ModelToWorldMatrix,
		mat4f WorldToViewMatrix,
		mat4f ProjectionMatrix);

	void UpdateLightBuffer(
		vec4f LightSourcePosition,
		vec4f CameraPosition);

public:
	OurTestScene(
		ID3D11Device* dxdevice,
		ID3D11DeviceContext* dxdevice_context,
		int window_width,
		int window_height);

	void Init() override;

	void Update(
		float dt,
		InputHandler* input_handler) override;

	void Render() override;

	void Release() override;

	void WindowResize(
		int window_width,
		int window_height) override;

	void CreateSamplerState();
};

#endif