#include "Cube.h"

Cube::Cube(
	ID3D11Device* dxdevice,
	ID3D11DeviceContext* dxdevice_context)
	: Model(dxdevice, dxdevice_context)
{
	// Vertex and index arrays
	// Once their data is loaded to GPU buffers, they are not needed anymore
	std::vector<Vertex> vertices;
	std::vector<unsigned> indices;

	InitMaterialBuffer();

	// Populate the vertex array with 4 vertices
	Vertex v0, v1, v2, v3;
	float n = 1;
	for (float i = 0.0f; i < 2; i++) {

		//z
		v0.Pos = { 0, 0, i };
		v0.Normal = { 0, 0, n };
		v0.TexCoord = { 0, 0 };
		vertices.push_back(v0);

		v1.Pos = { 1, 0, i };
		v1.Normal = { 0, 0, n };
		v1.TexCoord = { 0, n };
		vertices.push_back(v1);

		v2.Pos = { 1, 1, i };
		v2.Normal = { 0, 0, n };
		v2.TexCoord = { n, n };
		vertices.push_back(v2);

		v3.Pos = { 0, 1, i };
		v3.Normal = { 0, 0, n };
		v3.TexCoord = { n, 0 };
		vertices.push_back(v3);

		//y
		v0.Pos = { 0, i, 0 };
		v0.Normal = { 0, 0, n };
		v0.TexCoord = { 0, 0 };
		vertices.push_back(v0);

		v1.Pos = { 1, i, 0 };
		v1.Normal = { 0, 0, n };
		v1.TexCoord = { 0, n };
		vertices.push_back(v1);

		v2.Pos = { 1, i, 1};
		v2.Normal = { 0, 0, n };
		v2.TexCoord = { n, n };
		vertices.push_back(v2);

		v3.Pos = { 0, i, 1 };
		v3.Normal = { 0, 0, n };
		v3.TexCoord = { n, 0 };
		vertices.push_back(v3);

		//x
		v0.Pos = { i, 0, 0 };
		v0.Normal = { 0, 0, n };
		v0.TexCoord = { 0, 0 };
		vertices.push_back(v0);

		v1.Pos = { i, 1, 0 };
		v1.Normal = { 0, 0, n };
		v1.TexCoord = { 0, n };
		vertices.push_back(v1);

		v2.Pos = { i, 1, 1 };
		v2.Normal = { 0, 0, n };
		v2.TexCoord = { n, n };
		vertices.push_back(v2);

		v3.Pos = { i, 0, 1 };
		v3.Normal = { 0, 0, n };
		v3.TexCoord = { n, 0 };
		vertices.push_back(v3);

		n = -n;
	}

	
	// z 1
	indices.push_back(3);
	indices.push_back(1);
	indices.push_back(0);
	
	indices.push_back(3);
	indices.push_back(2);
	indices.push_back(1);

	// y 1
	indices.push_back(4);
	indices.push_back(5);
	indices.push_back(7);

	indices.push_back(5);
	indices.push_back(6);
	indices.push_back(7);

	//x 1
	indices.push_back(11);
	indices.push_back(9);
	indices.push_back(8);
	
	indices.push_back(11);
	indices.push_back(10);
	indices.push_back(9);

	//z 2
	indices.push_back(12);
	indices.push_back(13);
	indices.push_back(15);

	indices.push_back(13);
	indices.push_back(14);
	indices.push_back(15);

	//y 2
	indices.push_back(19);
	indices.push_back(17);
	indices.push_back(16);
	
	indices.push_back(19);
	indices.push_back(18);
	indices.push_back(17);

	// x 2
	indices.push_back(20);
	indices.push_back(21);
	indices.push_back(23);

	indices.push_back(21);
	indices.push_back(22);
	indices.push_back(23);


	// Vertex array descriptor
	D3D11_BUFFER_DESC vbufferDesc = { 0 };
	vbufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbufferDesc.CPUAccessFlags = 0;
	vbufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vbufferDesc.MiscFlags = 0;
	vbufferDesc.ByteWidth = (UINT)(vertices.size() * sizeof(Vertex));
	// Data resource
	D3D11_SUBRESOURCE_DATA vdata;
	vdata.pSysMem = &vertices[0];
	// Create vertex buffer on device using descriptor & data
	const HRESULT vhr = dxdevice->CreateBuffer(&vbufferDesc, &vdata, &vertex_buffer);
	SETNAME(vertex_buffer, "VertexBuffer");

	//  Index array descriptor
	D3D11_BUFFER_DESC ibufferDesc = { 0 };
	ibufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibufferDesc.CPUAccessFlags = 0;
	ibufferDesc.Usage = D3D11_USAGE_DEFAULT;
	ibufferDesc.MiscFlags = 0;
	ibufferDesc.ByteWidth = (UINT)(indices.size() * sizeof(unsigned));
	// Data resource
	D3D11_SUBRESOURCE_DATA idata;
	idata.pSysMem = &indices[0];
	// Create index buffer on device using descriptor & data
	const HRESULT ihr = dxdevice->CreateBuffer(&ibufferDesc, &idata, &index_buffer);
	SETNAME(index_buffer, "IndexBuffer");

	nbr_indices = (unsigned int)indices.size();

	HRESULT hr;

	mtl.Kd_texture_filename = "assets/textures/earth_diffuse.jpg";
	if (mtl.Kd_texture_filename.size()) {

		hr = LoadTextureFromFile(
			dxdevice,
			mtl.Kd_texture_filename.c_str(),
			&mtl.diffuse_texture);
		std::cout << "\t" << mtl.Kd_texture_filename
			<< (SUCCEEDED(hr) ? " - OK" : "- FAILED") << std::endl;
	}
}


void Cube::Render() const
{
	dxdevice_context->PSSetConstantBuffers(1, 1, &material_buffer);

	dxdevice_context->PSSetShaderResources(0, 1, &mtl.diffuse_texture.texture_SRV);
	dxdevice_context->PSSetShaderResources(1, 1, &mtl.normal_texture.texture_SRV);

	// Bind our vertex buffer
	const UINT32 stride = sizeof(Vertex); //  sizeof(float) * 8;
	const UINT32 offset = 0;
	dxdevice_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);

	// Bind our index buffer
	dxdevice_context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);

	// Make the drawcall
	dxdevice_context->DrawIndexed(nbr_indices, 0, 0);

	UpdateMaterialBuffer(mtl);
}