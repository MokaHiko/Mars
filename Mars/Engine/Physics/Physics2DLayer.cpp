#include "Physics2DLayer.h"
#include "Core/Application.h"

#include "Physics.h"
#include "ECS/Components/Components.h"

namespace mrs
{
	Physics2DLayer::Physics2DLayer()
	{
	}

	Physics2DLayer::~Physics2DLayer()
	{
	}

	void Physics2DLayer::OnAttach()
	{
		// Get scene handle
		_scene = Application::GetInstance().GetScene();

		// Init world
		b2Vec2 gravity = {0.0f, -10.0f};
		_physics_world = new b2World(gravity);

		auto view = _scene->Registry()->view<Transform, RigidBody2D>();
		for (auto entity : view)
		{
			Entity e(entity, _scene);
			AddBody(e);
		}

		_contact_listener = new ContactListener(_scene);
		_physics_world->SetContactListener(_contact_listener);
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

			b2Vec2 new_pos = rb.body->GetPosition();
			float rot_z = rb.body->GetAngle();
			transform.position = glm::vec3(new_pos.x, new_pos.y, 0.0f);
			transform.rotation = glm::vec3(0, 0, rot_z);
		}
	}

	void Physics2DLayer::OnImGuiRender()
	{
	}

	void Physics2DLayer::AddBody(Entity entity)
	{
		Transform &transform = entity.GetComponent<Transform>();
		RigidBody2D &rb = entity.GetComponent<RigidBody2D>();

		if (rb.type == BodyType::STATIC)
		{
			b2BodyDef static_body_def = {};
			static_body_def.position.Set(transform.position.x, transform.position.y);
			rb.body = _physics_world->CreateBody(&static_body_def);
			rb.body->GetUserData().pointer = entity.Id();

			CreateFixture(entity, BodyType::STATIC);
		}

		else if (rb.type == BodyType::DYNAMIC)
		{
			b2BodyDef dynamic_body_def = {};
			dynamic_body_def.type = b2_dynamicBody;
			dynamic_body_def.position = b2Vec2(transform.position.x, transform.position.y);
			rb.body = _physics_world->CreateBody(&dynamic_body_def);
			rb.body->GetUserData().pointer = entity.Id();

			CreateFixture(entity, BodyType::DYNAMIC);
		}
		else
		{
			throw std::runtime_error("Uknown Rigidbody type!");
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
			rb.body->CreateFixture(&fixture_def);
		}
		else if (type == BodyType::STATIC)
		{
			if (entity.HasComponent<BoxCollider2D>())
			{
				b2PolygonShape static_box;
				static_box.SetAsBox(transform.scale.x, transform.scale.y);
				rb.body->CreateFixture(&static_box, 0.0f);
			}
		}
	}

	void ContactListener::BeginContact(b2Contact *contact)
	{
		Entity entity_a = Entity((entt::entity)contact->GetFixtureA()->GetBody()->GetUserData().pointer, _scene);
		Entity entity_b = Entity((entt::entity)contact->GetFixtureB()->GetBody()->GetUserData().pointer, _scene);

		if(entity_a.HasComponent<Script>()) {
		entity_a.GetComponent<Script>().script->OnCollisionEnter(entity_b);
		}

		if(entity_b.HasComponent<Script>()) {
		entity_b.GetComponent<Script>().script->OnCollisionEnter(entity_a);
		}
	}
	void ContactListener::EndContact(b2Contact *contact)
	{
	}
}