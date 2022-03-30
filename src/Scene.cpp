
#include "Scene.h"

Scene::Scene(
	ID3D11Device* dxdevice,
	ID3D11DeviceContext* dxdevice_context,
	int window_width,
	int window_height) :
	dxdevice(dxdevice),
	dxdevice_context(dxdevice_context),
	window_width(window_width),
	window_height(window_height)
{ }

void Scene::WindowResize(
	int window_width,
	int window_height)
{
	this->window_width = window_width;
	this->window_height = window_height;
}

OurTestScene::OurTestScene(
	ID3D11Device* dxdevice,
	ID3D11DeviceContext* dxdevice_context,
	int window_width,
	int window_height) :
	Scene(dxdevice, dxdevice_context, window_width, window_height)
{ 
	InitTransformationBuffer();
	// + init other CBuffers
	InitLightBuffer();
	CreateSamplerState();
}

//
// Called once at initialization
//
void OurTestScene::Init()
{
	camera = new Camera(
		45.0f * fTO_RAD,		// field-of-view (radians)
		(float)window_width / window_height,	// aspect ratio
		1.0f,					// z-near plane (everything closer will be clipped/removed)
		500.0f);				// z-far plane (everything further will be clipped/removed)

	// Move camera to (0,0,5)
	camera->moveTo({ 0, 0, 5 });

	lightDir = true;
	lightZ = 10;
	lightSource = { 0, 0, 10 };

	// Create objects
	quad = new QuadModel(dxdevice, dxdevice_context);
	cube = new Cube(dxdevice, dxdevice_context);
	child = new Cube(dxdevice, dxdevice_context);
	sponza = new OBJModel("assets/crytek-sponza/sponza.obj", dxdevice, dxdevice_context);
}

//
// Called every frame
// dt (seconds) is time elapsed since the previous frame
//
void OurTestScene::Update(
	float dt,
	InputHandler* input_handler)
{
	if(input_handler->IsKeyPressed(Keys::NUM_1))
		samplerStateIndex = 0;
	else if (input_handler->IsKeyPressed(Keys::NUM_2))
		samplerStateIndex = 1;
	else if (input_handler->IsKeyPressed(Keys::NUM_3))
		samplerStateIndex = 2;
	else if (input_handler->IsKeyPressed(Keys::NUM_4))
		samplerStateIndex = 3;
	else if (input_handler->IsKeyPressed(Keys::NUM_4))
		samplerStateIndex = 4;

	// Basic camera control
	if (input_handler->IsKeyPressed(Keys::Up) || input_handler->IsKeyPressed(Keys::W))
		camera->move({ 0.0f, 0.0f, -camera_vel * dt });
	if (input_handler->IsKeyPressed(Keys::Down) || input_handler->IsKeyPressed(Keys::S))
		camera->move({ 0.0f, 0.0f, camera_vel * dt });
	if (input_handler->IsKeyPressed(Keys::Right) || input_handler->IsKeyPressed(Keys::D))
		camera->move({ camera_vel * dt, 0.0f, 0.0f });
	if (input_handler->IsKeyPressed(Keys::Left) || input_handler->IsKeyPressed(Keys::A))
		camera->move({ -camera_vel * dt, 0.0f, 0.0f });

	long mousedx = input_handler->GetMouseDeltaX();
	long mousedy = input_handler->GetMouseDeltaY();

	if((GetKeyState(VK_LBUTTON) & 0x100) != 0)
		camera->rotate(input_handler);

	// Now set/update object transformations
	// This can be done using any sequence of transformation matrices,
	// but the T*R*S order is most common; i.e. scale, then rotate, and then translate.
	// If no transformation is desired, an identity matrix can be obtained 
	// via e.g. Mquad = linalg::mat4f_identity; 

	// Quad model-to-world transformation
	Mquad = mat4f::translation(0, 0, 0) *			// No translation
		mat4f::rotation(-angle, 0.0f, 1.0f, 0.0f) *	// Rotate continuously around the y-axis
		mat4f::scaling(1.5, 1.5, 1.5);				// Scale uniformly to 150%

	Mcube = mat4f::translation(lightSource) *			
		mat4f::rotation(angle / 2 , 0.0f, 1.0f, 0.0f) *
		mat4f::scaling(1, 1, 1);				

	Mchild = Mcube * mat4f::translation(2, 0, 0) *
		mat4f::rotation(angle, 0.0f, 1.0f, 0.0f) *
		mat4f::scaling(0.5, 0.5, 0.5);

	// Sponza model-to-world transformation
	Msponza = mat4f::translation(0, -5, 0) *		 // Move down 5 units
		mat4f::rotation(fPI / 2, 0.0f, 1.0f, 0.0f) * // Rotate pi/2 radians (90 degrees) around y
		mat4f::scaling(0.05f);						 // The scene is quite large so scale it down to 5%

	// Increment the rotation angle.
	angle += angle_vel * dt;

	lightSource = { 0 , 0, lightZ };

	/*if (lightDir) {
		lightZ+= 0.5f;
		if (lightZ > 20) {
			lightDir = false;
		}
	}
	else if (!lightDir) {
		lightZ-= 0.5f;
		if (lightZ < -20) {
			lightDir = true;
		}
	}*/
	

	// Print fps
	fps_cooldown -= dt;
	if (fps_cooldown < 0.0)
	{
		//std::cout << "fps " << (int)(1.0f / dt) << std::endl;
//		printf("fps %i\n", (int)(1.0f / dt));
		fps_cooldown = 2.0;
	}
}

//
// Called every frame, after update
//
void OurTestScene::Render()
{
	
	dxdevice_context->PSSetSamplers(0, 1, &samplerStates[samplerStateIndex]);

	// Bind transformation_buffer to slot b0 of the VS
	dxdevice_context->VSSetConstantBuffers(0, 1, &transformation_buffer);

	dxdevice_context->PSSetConstantBuffers(2, 1, &light_buffer);

	// Obtain the matrices needed for rendering from the camera
	Mview = camera->get_WorldToViewMatrix();
	Mproj = camera->get_ProjectionMatrix();

	UpdateLightBuffer(lightSource.xyz0(), camera->position.xyz0());

	// Load matrices + the Quad's transformation to the device and render it
	UpdateTransformationBuffer(Mquad, Mview, Mproj);
	//quad->Render();

	// Load matrices + Sponza's transformation to the device and render it
	UpdateTransformationBuffer(Msponza, Mview, Mproj);
	sponza->Render();

	UpdateTransformationBuffer(Mcube, Mview, Mproj);
	cube->Render();

	UpdateTransformationBuffer(Mchild, Mview, Mproj);
	child->Render();

	
}

void OurTestScene::Release()
{
	SAFE_DELETE(quad);
	SAFE_DELETE(sponza);
	SAFE_DELETE(cube);
	SAFE_DELETE(child);
	SAFE_DELETE(camera);

	SAFE_RELEASE(transformation_buffer);
	SAFE_RELEASE(light_buffer);
	// + release other CBuffers
}

void OurTestScene::WindowResize(
	int window_width,
	int window_height)
{
	if (camera)
		camera->aspect = float(window_width) / window_height;

	Scene::WindowResize(window_width, window_height);
}

void OurTestScene::CreateSamplerState()
{
	HRESULT hr;
	D3D11_SAMPLER_DESC sd0 =
	{
	D3D11_FILTER_ANISOTROPIC,
	D3D11_TEXTURE_ADDRESS_WRAP,
	D3D11_TEXTURE_ADDRESS_WRAP,
	D3D11_TEXTURE_ADDRESS_WRAP,
	0.0f,
	16,
	D3D11_COMPARISON_NEVER,
		{1.0f, 1.0f, 1.0f, 1.0f},
		-FLT_MAX,
		FLT_MAX,
	};
	ASSERT(hr = dxdevice->CreateSamplerState(&sd0, &samplerState));
	samplerStates.push_back(samplerState);

	D3D11_SAMPLER_DESC sd1 =
	{
	D3D11_FILTER_ANISOTROPIC,
	D3D11_TEXTURE_ADDRESS_MIRROR,
	D3D11_TEXTURE_ADDRESS_MIRROR,
	D3D11_TEXTURE_ADDRESS_MIRROR,
	0.0f,
	16,
	D3D11_COMPARISON_NEVER,
		{1.0f, 1.0f, 1.0f, 1.0f},
		-FLT_MAX,
		FLT_MAX,
	};
	ASSERT(hr = dxdevice->CreateSamplerState(&sd1, &samplerState));
	samplerStates.push_back(samplerState);

	D3D11_SAMPLER_DESC sd2 =
	{
	D3D11_FILTER_ANISOTROPIC,
	D3D11_TEXTURE_ADDRESS_CLAMP,
	D3D11_TEXTURE_ADDRESS_CLAMP,
	D3D11_TEXTURE_ADDRESS_CLAMP,
	0.0f,
	16,
	D3D11_COMPARISON_NEVER,
		{1.0f, 1.0f, 1.0f, 1.0f},
		-FLT_MAX,
		FLT_MAX,
	};
	ASSERT(hr = dxdevice->CreateSamplerState(&sd2, &samplerState));
	samplerStates.push_back(samplerState);

	D3D11_SAMPLER_DESC sd3 =
	{
	D3D11_FILTER_MIN_MAG_MIP_POINT,
	D3D11_TEXTURE_ADDRESS_CLAMP,
	D3D11_TEXTURE_ADDRESS_CLAMP,
	D3D11_TEXTURE_ADDRESS_CLAMP,
	0.0f,
	16,
	D3D11_COMPARISON_NEVER,
		{1.0f, 1.0f, 1.0f, 1.0f},
		-FLT_MAX,
		FLT_MAX,
	};
	ASSERT(hr = dxdevice->CreateSamplerState(&sd3, &samplerState));
	samplerStates.push_back(samplerState);

	D3D11_SAMPLER_DESC sd4 =
	{
	D3D11_FILTER_MIN_MAG_MIP_LINEAR,
	D3D11_TEXTURE_ADDRESS_WRAP,
	D3D11_TEXTURE_ADDRESS_WRAP,
	D3D11_TEXTURE_ADDRESS_WRAP,
	0.0f,
	16,
	D3D11_COMPARISON_NEVER,
		{1.0f, 1.0f, 1.0f, 1.0f},
		-FLT_MAX,
		FLT_MAX,
	};
	ASSERT(hr = dxdevice->CreateSamplerState(&sd4, &samplerState));
	samplerStates.push_back(samplerState);

}

void OurTestScene::InitTransformationBuffer()
{
	HRESULT hr;
	D3D11_BUFFER_DESC MatrixBuffer_desc = { 0 };
	MatrixBuffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	MatrixBuffer_desc.ByteWidth = sizeof(TransformationBuffer);
	MatrixBuffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	MatrixBuffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	MatrixBuffer_desc.MiscFlags = 0;
	MatrixBuffer_desc.StructureByteStride = 0;
	ASSERT(hr = dxdevice->CreateBuffer(&MatrixBuffer_desc, nullptr, &transformation_buffer));
}

void OurTestScene::InitLightBuffer()
{
	HRESULT hr;
	D3D11_BUFFER_DESC VectorBuffer_desc = { 0 };
	VectorBuffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	VectorBuffer_desc.ByteWidth = sizeof(LightBuffer);
	VectorBuffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	VectorBuffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	VectorBuffer_desc.MiscFlags = 0;
	VectorBuffer_desc.StructureByteStride = 0;
	ASSERT(hr = dxdevice->CreateBuffer(&VectorBuffer_desc, nullptr, &light_buffer));
}

void OurTestScene::UpdateTransformationBuffer(
	mat4f ModelToWorldMatrix,
	mat4f WorldToViewMatrix,
	mat4f ProjectionMatrix)
{
	// Map the resource buffer, obtain a pointer and then write our matrices to it
	D3D11_MAPPED_SUBRESOURCE resource;
	dxdevice_context->Map(transformation_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	TransformationBuffer* matrix_buffer_ = (TransformationBuffer*)resource.pData;
	matrix_buffer_->ModelToWorldMatrix = ModelToWorldMatrix;
	matrix_buffer_->WorldToViewMatrix = WorldToViewMatrix;
	matrix_buffer_->ProjectionMatrix = ProjectionMatrix;
	dxdevice_context->Unmap(transformation_buffer, 0);
}

void OurTestScene::UpdateLightBuffer(vec4f LightSourcePosition, vec4f CameraPosition)
{
	// Map the resource buffer, obtain a pointer and then write our matrices to it
	D3D11_MAPPED_SUBRESOURCE resource;
	dxdevice_context->Map(light_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	LightBuffer* vector_buffer_ = (LightBuffer*)resource.pData;
	vector_buffer_->LightSourcePosition = LightSourcePosition;
	vector_buffer_->CameraPosition = CameraPosition;
	dxdevice_context->Unmap(light_buffer, 0);

	//std::cout << LightSourcePosition << std::endl;
}
