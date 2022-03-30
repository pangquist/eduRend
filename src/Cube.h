#pragma once
#include "Model.h"

class Cube : public Model
{
		unsigned nbr_indices = 0;

		Material mtl;

	public:

		Cube(
			ID3D11Device* dx3ddevice,
			ID3D11DeviceContext* dx3ddevice_context);

		virtual void Render() const;

		~Cube() { }
};