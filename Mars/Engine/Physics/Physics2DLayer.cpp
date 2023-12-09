#include "Physics2DLayer.h"
#include "Core/Application.h"

#include "Physics.h"
#include "ECS/Components/Components.h"

#include "Core/Time.h"
#include "Algorithm.h"

namespace mrs
{
	void Physics2DLayer::OnAttach()
	{
		_name = "Physics2DLayer";
		_scene = Application::Instance().GetScene();

		// Connect to RigidBody2D Component signals
		_scene = Application::Instance().GetScene();
		_scene->Registry()->on_construct<RigidBody2D>().connect<&Physics2DLayer::OnRigidBody2DCreated>(this);
		_scene->Registry()->on_destroy<RigidBody2D>().connect<&Physics2DLayer::OnRigidBody2DDestroyed>(this);
	}

	void Physics2DLayer::OnDetatch()
	{
		delete _physics_world;
		delete _contact_listener;
	}

	void Physics2DLayer::OnUpdate(float dt)
	{
		uint32_t velocity_iterations = 6;
		uint32_t position_iterations = 2;

		float timeStep = Time::FixedDeltaTime();
		_physics_world->Step(dt, velocity_iterations, position_iterations);

		FlushQueues();

		auto view = _scene->Registry()->view<Transform, RigidBody2D>();
		for (auto entity : view)
		{
			Entity e(entity, _scene);
			RigidBody2D& rb = e.GetComponent<RigidBody2D>();

			// Register body if not yet added
			if (rb.body == nullptr)
			{
				AddBody(e);
				continue;
			}

			Transform& transform = e.GetComponent<Transform>();

			// Get physics transform components
			b2Vec2 new_pos = rb.body->GetPosition();
			float rot_z = glm::degrees(rb.body->GetAngle());

			// Update transform
			transform.position.x = new_pos.x;
			transform.position.y = new_pos.y;
			transform.rotation.z = rot_z;
		}
	}

	void Physics2DLayer::OnFixedUpdate(float fixed_dt)
	{
	}

	void Physics2DLayer::OnImGuiRender()
	{
	}

	void Physics2DLayer::OnEnable()
	{
		InitWorld();
	}

	void Physics2DLayer::OnDisable()
	{
		ShutdownWorld();
	}

	void Physics2DLayer::AddBody(Entity entity)
	{
		Transform& transform = entity.GetComponent<Transform>();
		RigidBody2D& rb = entity.GetComponent<RigidBody2D>();

		if (rb.type == BodyType::STATIC)
		{
			b2BodyDef static_body_def = {};
			static_body_def.position.Set(transform.position.x, transform.position.y);
			static_body_def.angle = glm::radians(transform.rotation.z);

			rb.body = _physics_world->CreateBody(&static_body_def);
			rb.body->GetUserData().pointer = entity.Id();

			b2MassData mass_data = {};
			mass_data.mass = rb.mass;
			rb.body->SetMassData(&mass_data);

			CreateFixture(entity, BodyType::STATIC);
		}
		else if (rb.type == BodyType::DYNAMIC)
		{
			b2BodyDef dynamic_body_def = {};
			dynamic_body_def.type = b2_dynamicBody;
			dynamic_body_def.position = b2Vec2(transform.position.x, transform.position.y);
			dynamic_body_def.angle = glm::radians(transform.rotation.z);

			rb.body = _physics_world->CreateBody(&dynamic_body_def);
			rb.body->GetUserData().pointer = entity.Id();

			b2MassData mass_data = {};
			mass_data.mass = rb.mass;
			rb.body->SetMassData(&mass_data);

			if (!rb.use_gravity)
			{
				rb.body->SetGravityScale(0.0f);
			}

			CreateFixture(entity, BodyType::DYNAMIC);
		}
		else if (rb.type == BodyType::KINEMATIC)
		{
			b2BodyDef kinematic_body_def = {};
			kinematic_body_def.type = b2_kinematicBody;
			kinematic_body_def.position = b2Vec2(transform.position.x, transform.position.y);
			kinematic_body_def.angle = glm::radians(transform.rotation.z);

			rb.body = _physics_world->CreateBody(&kinematic_body_def);
			rb.body->GetUserData().pointer = entity.Id();

			b2MassData mass_data = {};
			mass_data.mass = rb.mass;
			rb.body->SetMassData(&mass_data);

			if (!rb.use_gravity)
			{
				rb.body->SetGravityScale(0.0f);
			}

			CreateFixture(entity, BodyType::KINEMATIC);
		}
		else
		{
			MRS_ERROR("Uknown rigidbody type!");
		}

		rb.body->SetLinearVelocity({ rb.start_velocity.x, rb.start_velocity.y });
		rb.body->SetAngularVelocity(rb.start_angular_velocity);
	}

	void Physics2DLayer::CreateFixture(Entity entity, BodyType type)
	{
		Transform& transform = entity.GetComponent<Transform>();
		RigidBody2D& rb = entity.GetComponent<RigidBody2D>();

		if (type == BodyType::DYNAMIC)
		{
			b2PolygonShape dynamic_box = {};
			dynamic_box.SetAsBox(transform.scale.x, transform.scale.y);

			b2FixtureDef fixture_def = {};
			fixture_def.shape = &dynamic_box;
			fixture_def.density = 1.0f;
			fixture_def.friction = rb.friction;
			fixture_def.restitution = 0.2f;

			rb.body->CreateFixture(&fixture_def);
		}
		else if (type == BodyType::KINEMATIC)
		{
			b2PolygonShape kinematic_box = {};
			kinematic_box.SetAsBox(transform.scale.x, transform.scale.y);

			b2FixtureDef fixture_def = {};
			fixture_def.shape = &kinematic_box;
			fixture_def.density = 1.0f;
			fixture_def.friction = rb.friction;
			fixture_def.restitution = 0.2f;
			fixture_def.isSensor = true;

			rb.body->CreateFixture(&fixture_def);
		}
		else if (type == BodyType::STATIC)
		{
			b2PolygonShape static_box;
			static_box.SetAsBox(transform.scale.x, transform.scale.y);
			rb.body->CreateFixture(&static_box, 0.0f);

			// TODO: Add Box Collider 2D Component
			if (entity.HasComponent<BoxCollider2D>())
			{
			}
		}
	}

	void Physics2DLayer::OnRigidBody2DCreated(entt::basic_registry<entt::entity>&, entt::entity entity)
	{
		Entity e{ entity, _scene };
		_construction_queue.push_back(e);
	}

	void Physics2DLayer::OnRigidBody2DDestroyed(entt::basic_registry<entt::entity>&, entt::entity entity)
	{
		Entity e{ entity, _scene };

		// We push the the b2body* because the entity along with its RigidBody2D component is destroyed
		if (e.HasComponent<RigidBody2D>())
		{
			auto& rb = e.GetComponent<RigidBody2D>();

			MRS_ASSERT(rb.body != nullptr, std::to_string(e.Id()).c_str());
			_destruction_queue.push_back(rb.body);
		}
	}

	void Physics2DLayer::Raycast(const Ray& ray, float distance, RayCastListener& callback)
	{
		b2Vec2 p1 = b2Vec2(ray.origin.x, ray.origin.y);
		b2Vec2 p2 = p1 + b2Vec2(ray.direction.x * distance, ray.direction.y * distance);

		_physics_world->RayCast(&callback, p1, p2);
	}

	void Physics2DLayer::InitWorld()
	{
		// Prevent instantiate of world without shutdown
		if (_physics_world != nullptr)
		{
			ShutdownWorld();
		}

		// Init world
		b2Vec2 gravity = { 0.0f, -10.0f };
		_physics_world = MRS_NEW b2World(gravity);

		auto view = _scene->Registry()->view<Transform, RigidBody2D>();
		for (auto entity : view)
		{
			Entity e(entity, _scene);
			AddBody(e);
		}

		// Get scene handle
		_scene = Application::Instance().GetScene();

		// Init contact listeners
		_contact_listener = MRS_NEW ContactListener(_scene);
		_physics_world->SetContactListener(_contact_listener);
	}

	void Physics2DLayer::ShutdownWorld()
	{
		delete _contact_listener;
		_contact_listener = nullptr;

		delete _physics_world;
		_physics_world = nullptr;
	}

	void Physics2DLayer::FlushQueues()
	{
		for (auto& e : _construction_queue)
		{
			// Real time addition if layer is active
			if (_enabled)
			{
				AddBody(e);
			}
		}
		_construction_queue.clear();

		for (b2Body* body : _destruction_queue)
		{
			if (body)
			{
				body->GetWorld()->DestroyBody(body);
			}
		}
		_destruction_queue.clear();
	}

	void ContactListener::BeginContact(b2Contact* contact)
	{
		Entity entity_a = Entity((entt::entity)contact->GetFixtureA()->GetBody()->GetUserData().pointer, _scene);
		Entity entity_b = Entity((entt::entity)contact->GetFixtureB()->GetBody()->GetUserData().pointer, _scene);

		if (entity_a.HasComponent<Script>()) {
			ScriptableEntity* script = (entity_a.GetComponent<Script>().script);

			if (script != nullptr)
			{
				script->OnCollisionEnter2D(entity_b);
			}
		}

		if (entity_b.HasComponent<Script>()) {
			ScriptableEntity* script = (entity_b.GetComponent<Script>().script);

			if (script)
			{
				script->OnCollisionEnter2D(entity_a);
			}
		}
	}

	void ContactListener::EndContact(b2Contact* contact)
	{
	}

	float RayCastListener::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction)
	{
		Entity e = Entity((entt::entity)fixture->GetBody()->GetUserData().pointer, _scene);

		Collision col = {};
		col.collision_points.normal = Vector3(normal.x, normal.y, 0.0f);
		col.collision_points.a = Vector3(point.x, point.y, 0.0f);
		col.entity = e;

		if (_fn)
		{
			_fn(col);
		}

		return fraction;
	}
}