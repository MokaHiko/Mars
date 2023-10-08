#include "Collider.h"
#include "Algorithm.h"

mrs::Collider::Collider()
{
}

mrs::Collider::~Collider()
{
}

mrs::CollisionPoints mrs::SphereCollider::TestRayCollision(const Transform* transform, const Ray* ray) const
{
	return algo::FindRaySphereCollision(ray, this, transform);
}

mrs::CollisionPoints mrs::SphereCollider::TestCollision(const Transform* transform, const Collider* other, const Transform* other_transform) const
{
	// Double dispatch
	return other->TestCollision(other_transform, this, transform);
}

mrs::CollisionPoints mrs::SphereCollider::TestCollision(const Transform* transform, const SphereCollider* other, const Transform* other_transform) const
{
	return CollisionPoints();
}

mrs::CollisionPoints mrs::SphereCollider::TestCollision(const Transform* transform, const PlaneCollider* other, const Transform* other_transform) const
{
	return CollisionPoints();
}

mrs::CollisionPoints mrs::PlaneCollider::TestRayCollision(const Transform* transform, const Ray* ray) const
{
	return algo::FindRayPlaneCollision(ray, this, transform);
}

mrs::CollisionPoints mrs::PlaneCollider::TestCollision(const Transform* transform, const Collider* other, const Transform* other_transform) const
{
	return other->TestCollision(other_transform, this, transform);
}

mrs::CollisionPoints mrs::PlaneCollider::TestCollision(const Transform* transform, const SphereCollider* other, const Transform* other_transform) const
{
	return CollisionPoints();
}

mrs::CollisionPoints mrs::PlaneCollider::TestCollision(const Transform* transform, const PlaneCollider* other, const Transform* other_transform) const
{
	return CollisionPoints();
}
