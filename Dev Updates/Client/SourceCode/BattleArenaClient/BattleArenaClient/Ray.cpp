#include "Ray.h"
using namespace DirectX;

CRay CRay::RayAtViewSpace(
	const D3D12_VIEWPORT& ViewPort, const DirectX::XMFLOAT4X4& ProjectionTransform,
	int nScreenX, int nScreenY)
{
	CRay ray;

	ray.OriginalPos = { 0.0f, 0.0f, 0.0f };
	ray.Direction.x = ( (((2.0f * (nScreenX - ViewPort.TopLeftX)) / ViewPort.Width)  - 1.0f) - ProjectionTransform._31);
	ray.Direction.y = (-(((2.0f * (nScreenY - ViewPort.TopLeftY)) / ViewPort.Height) - 1.0f) - ProjectionTransform._32);
	ray.Direction.x /= ProjectionTransform._11;
	ray.Direction.y /= ProjectionTransform._22;
	ray.Direction.z = 1.0f;
	ray.RaySpace = RaySpaceType::VIEW;

	return ray;
}

CRay CRay::RayAtWorldSpace(
	const D3D12_VIEWPORT& ViewPort, const DirectX::XMFLOAT4X4& ProjectionTransform, const DirectX::XMFLOAT4X4& ViewTransform,
	int nScreenX, int nScreenY)
{
	CRay ray = CRay::RayAtViewSpace(ViewPort, ProjectionTransform, nScreenX, nScreenY);

	XMMATRIX VIEW = XMLoadFloat4x4(&ViewTransform);
	XMMATRIX INV_VIEW = XMMatrixInverse(&XMMatrixDeterminant(VIEW), VIEW);
	XMVECTOR ORIGIN_POS = XMVector3TransformCoord(XMLoadFloat3(&ray.OriginalPos), INV_VIEW);
	XMVECTOR DIRECTON = XMVector3TransformNormal(XMLoadFloat3(&ray.Direction), INV_VIEW);
	DIRECTON = XMVector3Normalize(DIRECTON);

	XMStoreFloat3(&ray.OriginalPos, ORIGIN_POS);
	XMStoreFloat3(&ray.Direction, DIRECTON);

	ray.RaySpace = RaySpaceType::WORLD;

	return ray;
}

bool CRay::RayAABBIntersect(const CRay& ray, const DirectX::BoundingBox& AABB, DirectX::XMFLOAT3& intersectPoint)
{
	XMVECTOR ORIGIN_POS = XMLoadFloat3(&ray.OriginalPos);
	XMVECTOR DIRECTION = XMLoadFloat3(&ray.Direction);

	float t = 0.0f;
	bool Isintersected = AABB.Intersects(ORIGIN_POS, DIRECTION, t);

	XMVECTOR INTERSECT_POS = ORIGIN_POS + (t * DIRECTION);

	XMStoreFloat3(&intersectPoint, INTERSECT_POS);

	return Isintersected;
}