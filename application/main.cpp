#include "ui/mainApplication.h"
#include "ui/mainWindow.h"
#include <array>
#include <assetFactory/assetFactory.h>
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
#include <game/water.h>
#include <game/worldobject.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp> // IWYU pragma: keep
#include <memory>
#include <random>
#include <special_members.h>
#include <ui/applicationBase.h>
#include <ui/gameMainWindow.h>
#include <ui/window.h>

static const int DISPLAY_WIDTH = 1280;
static const int DISPLAY_HEIGHT = 1024;

class DummyMainApplication : public GameState, public MainApplication {
public:
	int
	run()
	{
		geoData = std::make_shared<GeoData>(GeoData::loadFromAsciiGrid("test/fixtures/height/SD19.asc"));

		windows.create<MainWindow>(DISPLAY_WIDTH, DISPLAY_HEIGHT)->setContent<GameMainWindow>();

		world.create<Terrain>(geoData);
		world.create<Water>(geoData);
		assets = AssetFactory::loadAll("res");

		{
			auto rl = world.create<RailLinks>();
			const GlobalPosition3D j {-1120000, -1100000, 3000}, k {-1100000, -1000000, 15000},
					l {-1000000, -800000, 20000}, m {-900000, -600000, 30000}, n {-600000, -500000, 32000},
					o {-500000, -800000, 30000}, p {-600000, -900000, 25000}, q {-1025000, -1175000, 10000},
					r {-925000, -1075000, 10000}, s {-1100000, -500000, 15000}, t {-1100000, -450000, 15000},
					u {-1000000, -400000, 15000};
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
			train->orders.create<GoTo>(
					&train->orders, l3->ends[1], l3->length, rl->findNodeAt({-1100000, -450000, 15000}));
			train->currentActivity = train->orders.current()->createActivity();

			std::random_device randomdev {};
			std::uniform_real_distribution<Angle> rotationDistribution {0, two_pi};
			std::uniform_int_distribution<GlobalDistance> positionOffsetDistribution {-1500, +1500};
			std::uniform_int_distribution<int> treeDistribution {1, 3};
			std::uniform_int_distribution<int> treeVariantDistribution {1, 4};
			for (auto x = 311000000; x < 311830000; x += 5000) {
				for (auto y = 491100000; y < 491130000; y += 5000) {
					world.create<Plant>(std::dynamic_pointer_cast<Foliage>(assets.at(std::format("Tree-{:#02}-{}",
												treeDistribution(randomdev), treeVariantDistribution(randomdev)))),
							Location {geoData->positionAt({{x + positionOffsetDistribution(randomdev),
											  y + positionOffsetDistribution(randomdev)}}),
									{0, rotationDistribution(randomdev), 0}});
				}
			}
		}

		mainLoop();

		world.objects.clear();
		return 0;
	}
};

int
main(int, char **)
{
	return std::make_shared<DummyMainApplication>()->run();
}
