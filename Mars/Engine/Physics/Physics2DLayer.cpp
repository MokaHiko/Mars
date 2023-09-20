#include "Physics2DLayer.h"
#include "Core/Application.h"

#include "Physics.h"
#include "ECS/Components/Components.h"

namespace mrs
{
	void Physics2DLayer::OnAttach()
	{
		_name = "Physics2DLayer";
		_scene = Application::Instance().GetScene();
		
		// Connect to RigidBody2D Component signals
		Scene* scene = Application::Instance().GetScene();
		scene->Registry()->on_destroy<RigidBody2D>().connect<&Physics2DLayer::OnRigidBody2DDestroyed>(this);
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

		auto view = _scene->Registry()->view<Transform, RigidBody2D>();
		float timeStep = dt;
		_physics_world->Step(timeStep, velocity_iterations, position_iterations);
		for (auto entity : view)
		{
			Entity e(entity, _scene);
			RigidBody2D &rb = e.GetComponent<RigidBody2D>();

			// Register body if not yet added
			if (rb.body == nullptr)
			{
				AddBody(e);
				continue;
			}

			Transform &transform = e.GetComponent<Transform>();

			// Get physics transform components
			b2Vec2 new_pos = rb.body->GetPosition();
			float rot_z = glm::degrees(rb.body->GetAngle());

			// Update transform
			transform.position.x = new_pos.x;
			transform.position.y = new_pos.y;
			transform.rotation.z = rot_z;
		}
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
		Transform &transform = entity.GetComponent<Transform>();
		RigidBody2D &rb = entity.GetComponent<RigidBody2D>();

		if (rb.type == BodyType::STATIC)
		{
			b2BodyDef static_body_def = {};
			static_body_def.position.Set(transform.position.x, transform.position.y);
			static_body_def.angle = glm::radians(transform.rotation.z);

			rb.body = _physics_world->CreateBody(&static_body_def);
			rb.body->GetUserData().pointer = entity.Id();

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

			if (!rb.use_gravity)
			{
				rb.body->SetGravityScale(0.0f);
			}

			CreateFixture(entity, BodyType::DYNAMIC);
		}
		else
		{
			MRS_ERROR("Uknown rigidbody type!");
		}
	}

	void Physics2DLayer::CreateFixture(Entity entity, BodyType type)
	{
		Transform &transform = entity.GetComponent<Transform>();
		RigidBody2D &rb = entity.GetComponent<RigidBody2D>();

		if (type == BodyType::DYNAMIC)
		{
			b2PolygonShape dynamic_box = {};
			dynamic_box.SetAsBox(transform.scale.x, transform.scale.y);

			b2FixtureDef fixture_def = {};
			fixture_def.shape = &dynamic_box;
			fixture_def.density = 1.0f;
			fixture_def.friction = 0.3f;
			fixture_def.restitution = 0.2f;
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
		Entity e{entity, _scene};
		
        // Real time addition if layer is active
		if(_enabled)
		{
            AddBody(e);
		}
	}

void Physics2DLayer::OnRigidBody2DDestroyed(entt::basic_registry<entt::entity>&, entt::entity entity)
	{
		Entity e{entity, _scene};
		if (e.HasComponent<RigidBody2D>())
		{
			auto &rb = e.GetComponent<RigidBody2D>();

			MRS_ASSERT(rb.body != nullptr, std::to_string(e.Id()).c_str());
			
			if (rb.body)
			{
				rb.body->GetWorld()->DestroyBody(rb.body);
			}
		}
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

	void ContactListener::BeginContact(b2Contact *contact)
	{
		Entity entity_a = Entity((entt::entity)contact->GetFixtureA()->GetBody()->GetUserData().pointer, _scene);
		Entity entity_b = Entity((entt::entity)contact->GetFixtureB()->GetBody()->GetUserData().pointer, _scene);

		if (entity_a.HasComponent<Script>()) {
			ScriptableEntity* script = (entity_a.GetComponent<Script>().script);

			if(script != nullptr)
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
	void ContactListener::EndContact(b2Contact *contact)
	{
	}
}