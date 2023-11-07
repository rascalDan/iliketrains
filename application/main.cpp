#include <array>
#include <assetFactory/assetFactory.h>
#include <chrono>
#include <collection.h>
#include <game/activities/go.h>
#include <game/activities/idle.h>
#include <game/activity.h>
#include <game/gamestate.h>
#include <game/geoData.h>
#include <game/network/link.h>
#include <game/network/rail.h>
#include <game/objective.h>
#include <game/objectives/goto.h>
#include <game/orders.h>
#include <game/scenary/foliage.h>
#include <game/scenary/plant.h>
#include <game/terrain.h>
#include <game/vehicles/railVehicle.h>
#include <game/vehicles/railVehicleClass.h>
#include <game/vehicles/train.h>
#include <game/worldobject.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp> // IWYU pragma: keep
#include <memory>
#include <special_members.h>
#include <ui/applicationBase.h>
#include <ui/gameMainWindow.h>
#include <ui/window.h>

static const int DISPLAY_WIDTH = 1280;
static const int DISPLAY_HEIGHT = 1024;

class MainApplication : public GameState, public ApplicationBase {
public:
	using Windows = Collection<Window>;

	int
	run()
	{
		geoData = std::make_shared<GeoData>(GeoData::Limits {{-120, -120}, {120, 120}}, 10.F);
		geoData->generateRandom();

		Windows windows;
		windows.create<GameMainWindow>(DISPLAY_WIDTH, DISPLAY_HEIGHT);

		world.create<Terrain>(geoData);
		assets = AssetFactory::loadAll("res");

		{
			auto rl = world.create<RailLinks>();
			const Position3D j {-1120, -1100, 3}, k {-1100, -1000, 15}, l {-1000, -800, 20}, m {-900, -600, 30},
					n {-600, -500, 32}, o {-500, -800, 30}, p {-600, -900, 25}, q {-1025, -1175, 10},
					r {-925, -1075, 10};
			const Position3D s {-1100, -500, 15}, t {-1100, -450, 15}, u {-1000, -400, 15};
			auto l3 = rl->addLinksBetween(j, k);
			rl->addLinksBetween(k, l);
			rl->addLinksBetween(l, m);
			rl->addLinksBetween(m, n);
			rl->addLinksBetween(n, o);
			rl->addLinksBetween(o, p);
			// branch 1
			rl->addLinksBetween(p, q);
			rl->addLinksBetween(q, j);
			// branch 2
			rl->addLinksBetween(p, r);
			rl->addLinksBetween(r, j);
			// early loop
			rl->addLinksBetween(s, t);
			rl->addLinksBetween(l, s);
			rl->addLinksBetween(t, u);
			rl->addLinksBetween(u, m);
			const std::shared_ptr<Train> train = world.create<Train>(l3);
			auto b47 = std::dynamic_pointer_cast<RailVehicleClass>(assets.at("brush-47"));
			for (int N = 0; N < 6; N++) {
				train->create<RailVehicle>(b47);
			}
			train->orders.removeAll();
			train->orders.create<GoTo>(&train->orders, l3->ends[1], l3->length, rl->findNodeAt({-1100, -450, 15}));
			train->currentActivity = train->orders.current()->createActivity();

			auto foliage = std::dynamic_pointer_cast<Foliage>(assets.at("Tree-01-1"));
			for (float x = 900; x < 1100; x += 3) {
				for (float y = 900; y < 1100; y += 3) {
					world.create<Plant>(foliage, Location {geoData->positionAt({-x, -y})});
				}
			}
		}

		auto t_start = std::chrono::high_resolution_clock::now();
		while (isRunning) {
			processInputs(windows);
			const auto t_end = std::chrono::high_resolution_clock::now();
			const auto t_passed = std::chrono::duration_cast<TickDuration>(t_end - t_start);

			world.apply(&WorldObject::tick, t_passed);
			windows.apply(&Window::tick, t_passed);
			windows.apply(&Window::refresh);

			t_start = t_end;
		}

		world.objects.clear();
		return 0;
	}

private:
	void
	processInputs(const Windows & windows)
	{
		for (SDL_Event e; SDL_PollEvent(&e);) {
			if (e.type == SDL_QUIT) {
				isRunning = false;
				return;
			}
			windows.applyOne(&Window::handleInput, e);
		}
	}

	bool isRunning {true};
};

int
main(int, char **)
{
	return std::make_shared<MainApplication>()->run();
}
