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
#include <game/objectives/freeroam.h>
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
#include <ranges>
#include <special_members.h>
#include <stream_support.h>
#include <ui/applicationBase.h>
#include <ui/builders/freeExtend.h>
#include <ui/builders/join.h>
#include <ui/builders/straight.h>
#include <ui/gameMainWindow.h>
#include <ui/window.h>

static const int DISPLAY_WIDTH = 1280;
static const int DISPLAY_HEIGHT = 1024;

class DummyMainApplication : public GameState, public MainApplication {
public:
	int
	run()
	{
		windows.create<MainWindow>(DISPLAY_WIDTH, DISPLAY_HEIGHT)->setContent<GameMainWindow>();

		terrain = world.create<Terrain>(GeoData::loadFromAsciiGrid("test/fixtures/height/SD19.asc"));
		world.create<Water>(terrain);
		assets = AssetFactory::loadAll("res");

		{
			auto rl = world.create<RailLinks>();
			const auto nodes = materializeRange(std::vector<GlobalPosition2D> {
														{315103000, 491067000},
														{315977000, 490777000},
														{316312000, 490557000},
														{316885000, 491330000},
														{316510934, 491255979},
														{316129566, 490893054},
														{315825622, 490833929},
														{315106182, 491073714},
												}
					| std::views::transform([this](const auto n) {
						  return terrain->positionAt(n);
					  }));
			auto l3 = BuilderStraight {}.create(rl.get(), terrain.get(), *nodes.begin(), *++nodes.begin()).front();
			for (const auto & [from, to] : nodes | std::views::drop(1) | std::views::pairwise) {
				const auto links = BuilderFreeExtend {}.createExtend(rl.get(), terrain.get(), from, to);
			}
			for (const auto & [from, to] : std::initializer_list<std::pair<GlobalPosition2D, GlobalPosition2D>> {
						 {{315103000, 491067000}, {315003434, 491076253}},
						 {{315103000, 491067000}, {315016495, 491019224}},
						 {{315016495, 491019224}, {314955393, 490999023}},
				 }) {
				const auto links = BuilderFreeExtend {}.createExtend(
						rl.get(), terrain.get(), terrain->positionAt(from), terrain->positionAt(to));
			}
			for (const auto & [from, to] : std::initializer_list<std::pair<GlobalPosition2D, GlobalPosition2D>> {
						 {{315106182, 491073714}, {314955393, 490999023}},
				 }) {
				auto p1 = rl->intersectRayNodes({from || 0, up})->pos;
				auto p2 = rl->intersectRayNodes({to || 0, up})->pos;
				const auto links = BuilderFreeExtend {}.createJoin(rl.get(), terrain.get(), p1, p2);
			}

			const std::shared_ptr<Train> train = world.create<Train>(l3, 800000);
			auto b47 = std::dynamic_pointer_cast<RailVehicleClass>(assets.at("brush-47"));
			for (int N = 0; N < 6; N++) {
				train->create<RailVehicle>(b47);
			}
			train->orders.removeAll();
			train->orders.create<FreeRoam>(&train->orders);
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
							Location {terrain->positionAt({{x + positionOffsetDistribution(randomdev),
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
