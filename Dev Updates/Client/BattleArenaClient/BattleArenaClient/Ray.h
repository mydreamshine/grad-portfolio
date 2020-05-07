#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>

class CRay
{
public:
	enum class RaySpaceType
	{
		NONE, VIEW, WORLD, LOCAL
	};

	DirectX::XMFLOAT3 OriginalPos = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 Direction = { 0.0f, 0.0f, 1.0f };
	RaySpaceType       RaySpace = RaySpaceType::NONE;

	CRay RayAtViewSpace(
		const D3D12_VIEWPORT& ViewPort, const DirectX::XMFLOAT4X4& ProjectionTransform,
		int nScreenX, int nScreenY);

	CRay RayAtWorldSpace(
		const D3D12_VIEWPORT& ViewPort, const DirectX::XMFLOAT4X4& ProjectionTransform, const DirectX::XMFLOAT4X4& ViewTransform,
		int nScreenX, int nScreenY);

	bool RayAABBIntersect(const CRay& ray, const DirectX::BoundingBox& AABB, DirectX::XMFLOAT3& intersectPoint);
};