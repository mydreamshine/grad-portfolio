#include "Ray.h"
using namespace DirectX;

CRay CRay::RayAtViewSpace(
	const D3D12_VIEWPORT& ViewPort, const DirectX::XMFLOAT4X4& ProjectionTransform,
	int nScreenX, int nScreenY)
{
	CRay ray;
	ray.Direction.x = ( (((2.0f * (nScreenX - ViewPort.TopLeftX)) / ViewPort.Width)  - 1.0f) - ProjectionTransform._31) / ProjectionTransform._11;
	ray.Direction.y = (-(((2.0f * (nScreenY - ViewPort.TopLeftY)) / ViewPort.Height) - 1.0f) - ProjectionTransform._32) / ProjectionTransform._22;
	ray.Direction.z = 1.0f;
	ray.RaySpace = RaySpaceType::VIEW;

	return ray;
}

CRay CRay::RayAtWorldSpace(
	const D3D12_VIEWPORT& ViewPort, const DirectX::XMFLOAT4X4& ProjectionTransform, const DirectX::XMFLOAT4X4& ViewTransform,
	int nScreenX, int nScreenY)
{
	CRay ray = CRay::RayAtViewSpace(ViewPort, ProjectionTransform, nScreenX, nScreenY);

	XMMATRIX INV_VIEW = XMMatrixInverse(nullptr, XMLoadFloat4x4(&ViewTransform));
	XMVECTOR ORIGIN_POS = XMVector3TransformCoord(XMLoadFloat3(&ray.OriginalPos), INV_VIEW);
	XMVECTOR DIRECTON = XMVector3TransformNormal(XMLoadFloat3(&ray.Direction), INV_VIEW);
	DIRECTON = XMVector3Normalize(DIRECTON);

	XMStoreFloat3(&ray.OriginalPos, ORIGIN_POS);
	XMStoreFloat3(&ray.Direction, DIRECTON);

	/*XMFLOAT4X4 inv_View; XMStoreFloat4x4(&inv_View, INV_VIEW);
	XMFLOAT3 v = ray.Direction;
	ray.Direction.x = v.x * inv_View._11 + v.y * inv_View._21 + v.z * inv_View._31;
	ray.Direction.y = v.x * inv_View._12 + v.y * inv_View._22 + v.z * inv_View._32;
	ray.Direction.z = v.x * inv_View._13 + v.y * inv_View._23 + v.z * inv_View._33;
	XMStoreFloat3(&ray.Direction, XMVector3Normalize(XMLoadFloat3(&ray.Direction)));
	ray.OriginalPos.x = inv_View._41;
	ray.OriginalPos.y = inv_View._42;
	ray.OriginalPos.z = inv_View._43;*/

	ray.RaySpace = RaySpaceType::WORLD;

	return ray;
}

bool CRay::RayAABBIntersect(const CRay& ray, const DirectX::BoundingBox& AABB, DirectX::XMFLOAT3& intersectPoint)
{
	// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
	XMFLOAT3 lb = { AABB.Center.x - AABB.Extents.x , AABB.Center.y - AABB.Extents.y, AABB.Center.z - AABB.Extents.z };
	XMFLOAT3 rt = { AABB.Center.x + AABB.Extents.x , AABB.Center.y + AABB.Extents.y, AABB.Center.z + AABB.Extents.z };
	XMFLOAT3 dirfrac;

	dirfrac.x = 1.0f / ray.Direction.x;
	dirfrac.y = 1.0f / ray.Direction.y;
	dirfrac.z = 1.0f / ray.Direction.z;


	float t1 = (lb.x - ray.OriginalPos.x) * dirfrac.x;
	float t2 = (rt.x - ray.OriginalPos.x) * dirfrac.x;
	float t3 = (lb.y - ray.OriginalPos.y) * dirfrac.y;
	float t4 = (rt.y - ray.OriginalPos.y) * dirfrac.y;
	float t5 = (lb.z - ray.OriginalPos.z) * dirfrac.z;
	float t6 = (rt.z - ray.OriginalPos.z) * dirfrac.z;

	float tmin = fmaxf(fmaxf(fminf(t1, t2), fminf(t3, t4)), fminf(t5, t6));
	float tmax = fminf(fminf(fmaxf(t1, t2), fmaxf(t3, t4)), fmaxf(t5, t6));

	float t;
	// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
	if (tmax < 0)
	{
		t = tmax;
		return false;
	}

	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax)
	{
		t = tmax;
		return false;
	}

	t = tmin;

	XMVECTOR ORIGIN_POS = XMLoadFloat3(&ray.OriginalPos);
	XMVECTOR DIRECTION = XMLoadFloat3(&ray.Direction);
	XMVECTOR INTERSECT_POS = ORIGIN_POS + (t * DIRECTION);

	XMStoreFloat3(&intersectPoint, INTERSECT_POS);

	return true;
}