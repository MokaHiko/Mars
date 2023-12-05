#include "TargetLockEffect.h"

AnimateTargetLockProcess::AnimateTargetLockProcess(mrs::Entity target_lock_sprite, float duration)
{
	_target_lock_sprite = target_lock_sprite;
	_duration = duration;

	auto& transform = _target_lock_sprite.GetComponent<mrs::Transform>();

	_end_scale = glm::length(transform.scale);
	_end_rotation = transform.rotation.z;

	_start_scale = glm::length(transform.scale) * 5.0f;
	_start_rotation = transform.rotation.z + 270.0f;
}

AnimateTargetLockProcess::~AnimateTargetLockProcess()
{
}

void AnimateTargetLockProcess::OnUpdate(float dt)
{
	if(_time_elapsed >= _duration)
	{
		Succeed();
	}

	auto& transform = _target_lock_sprite.GetComponent<mrs::Transform>();

	// Rotate
	float rotation = mrs::Lerp(_start_rotation, _end_rotation, _time_elapsed / _duration);
	transform.rotation.z = rotation;

	// Scale Down
	float scale = mrs::Lerp(_start_scale, _end_scale, _time_elapsed / _duration);
	transform.scale = mrs::Vector3(1.0f) * scale;

	// if (time_elapsed >= blink_period)
	// {
	// 	auto& renderable = _target_sprite.GetComponent<mrs::Renderable>();
	// 	renderable.enabled = !renderable.enabled;
	// 	time_elapsed = 0.0f;
	// }
	// time_elapsed += dt;

	_time_elapsed += dt;
}

void TargetLockEffect::Init()
{
	auto& transform = GetComponent<mrs::Transform>();
	AddComponent<mrs::SpriteRenderer>().sprite = mrs::Sprite::Get("target_lock");
	AddComponent<mrs::Renderable>().material = mrs::Material::Get("default_ui");
	//targetting.AddComponent<mrs::SpriteAnimator>();

	// Remove from parent to avoid following rotation
	_target = transform.parent;
	_target.GetComponent<mrs::Transform>().RemoveChild(_game_object);

	// TODO Steam line going to root
	mrs::Application::Instance().GetScene()->Root().GetComponent<mrs::Transform>().AddChild(_game_object);

	auto target_anim_process = CreateRef<AnimateTargetLockProcess>(_game_object, 1.0f);
	StartProcess(target_anim_process);
}

void TargetLockEffect::OnUpdate(float dt)
{
	if(!_target.IsAlive())
	{
		return;
	}

	auto& transform = GetComponent<mrs::Transform>();
	const auto& target_transform = _target.GetComponent<mrs::Transform>();

	// Follow without rotation
	transform.position = target_transform.position;
}

