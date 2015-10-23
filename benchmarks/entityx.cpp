#include "benchpress.hpp"
#include "entityx/entityx.h"
#include <random>

namespace entityx_benchmarks {
	const int NUM_ENTITIES = 1 << 15;

	using namespace entityx;

	struct ComponentA { float a; float b; float c; };
	struct ComponentB { float a; float b; float c; };
	struct ComponentC { float a; float b; float c; };
	struct ComponentD { float a; float b; float c; };
	struct ComponentE { float a; float b; float c; };

	struct IteratingSystemA : public System<IteratingSystemA> {
		void update(entityx::EntityManager &es, entityx::EventManager &events, TimeDelta dt) override {
			ComponentHandle<ComponentA> component;
			for (Entity entity : es.entities_with_components(component)) {
				component->a++;
				component->b++;
				component->c++;
			}
		};
	};

	struct IteratingSystemB : public System<IteratingSystemB> {
		void update(entityx::EntityManager &es, entityx::EventManager &events, TimeDelta dt) override {
			ComponentHandle<ComponentB> component;
			for (Entity entity : es.entities_with_components(component)) {
				component->a++;
				component->b++;
				component->c++;
			}
		};
	};

	struct IteratingSystemC : public System<IteratingSystemC> {
		void update(entityx::EntityManager &es, entityx::EventManager &events, TimeDelta dt) override {
			ComponentHandle<ComponentC> component;
			for (Entity entity : es.entities_with_components(component)) {
				component->a++;
				component->b++;
				component->c++;
			}
		};
	};

	struct IteratingSystemD : public System<IteratingSystemD> {
		void update(entityx::EntityManager &es, entityx::EventManager &events, TimeDelta dt) override {
			ComponentHandle<ComponentD> component;
			for (Entity entity : es.entities_with_components(component)) {
				component->a++;
				component->b++;
				component->c++;
			}
		};
	};

	struct IteratingSystemE : public System<IteratingSystemE> {
		void update(entityx::EntityManager &es, entityx::EventManager &events, TimeDelta dt) override {
			ComponentHandle<ComponentE> component;
			for (Entity entity : es.entities_with_components(component)) {
				component->a++;
				component->b++;
				component->c++;
			}
		};
	};

	class IteratingManager : public EntityX {
	public:
		explicit IteratingManager() {
			systems.add<IteratingSystemA>();
			systems.add<IteratingSystemB>();
			systems.add<IteratingSystemC>();
			systems.add<IteratingSystemD>();
			systems.add<IteratingSystemE>();
			systems.configure();

			std::vector<int> v;

			for (int i = 0; i < NUM_ENTITIES; i++) {
				v.push_back(i);
			}

			std::mt19937 g(0);
			std::shuffle(v.begin(), v.end(), g);

			for (int i = 0; i < NUM_ENTITIES; i++) {
				Entity entity = entities.create();
				if (v[i] & 1)  entity.assign<ComponentA>();
				if (v[i] & 2)  entity.assign<ComponentB>();
				if (v[i] & 4)  entity.assign<ComponentC>();
				if (v[i] & 8)  entity.assign<ComponentD>();
				if (v[i] & 16) entity.assign<ComponentE>();
			}
		}

		void update(TimeDelta dt) {
			systems.update_all(dt);
		}
	};

	class Benchmark {
	public:
		IteratingManager iteratingManager;

		Benchmark() {
		}

		void run(benchpress::context *ctx) {
			for (size_t i = 0; i < ctx->num_iterations(); ++i) {
				iteratingManager.update(42.0);
			}
		}
	};

	BENCHMARK(Benchmark, "entityx");
}