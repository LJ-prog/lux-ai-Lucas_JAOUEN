#include "lux/kit.hpp"
#include "lux/define.cpp"
#include <string.h>
#include <vector>
#include <set>
#include <stdio.h>

using namespace std;
using namespace lux;

int main()
{
  kit::Agent gameState = kit::Agent();
  // initialize
  gameState.initialize();

  while (true)
  {
    /** Do not edit! **/
    // wait for updates
    gameState.update();

    vector<string> actions = vector<string>();

    /** AI Code Goes Below! **/

    Player &player = gameState.players[gameState.id];
    Player &opponent = gameState.players[(gameState.id + 1) % 2];

    GameMap &gameMap = gameState.map;

    vector<Cell *> resourceTiles = vector<Cell *>();
    for (int y = 0; y < gameMap.height; y++)
    {
      for (int x = 0; x < gameMap.width; x++)
      {
        Cell *cell = gameMap.getCell(x, y);
        if (cell->hasResource())
        {
          resourceTiles.push_back(cell);
        }
      }
    }

    // Iterate over all our units and do something with them
    for (int i = 0; i < player.units.size(); i++)
    {
      Unit unit = player.units[i];

      if (unit.isWorker() && unit.canAct())
      {
        if (unit.getCargoSpaceLeft() > 0)
        {
          // If the unit is a worker and we have space in cargo, let's find the nearest resource tile and try to mine it
          Cell *closestResourceTile = nullptr;
          float closestDist = 9999999;

          for (auto it = resourceTiles.begin(); it != resourceTiles.end(); it++)
          {
            auto cell = *it;
            if (cell->resource.type == ResourceType::wood) {
              // We can always gather wood
            }
            else if (cell->resource.type == ResourceType::coal && !player.researchedCoal()) {
              // Skip coal if we haven't researched it yet
              continue;
            }
            else if (cell->resource.type == ResourceType::uranium && !player.researchedUranium()) {
              // Skip uranium if we haven't researched it yet
              continue;
            }
            if (unit.getCargoSpaceLeft() == 0)
              break;
            float dist = cell->pos.distanceTo(unit.pos);

            if (dist < closestDist)
            {
              closestDist = dist;
              closestResourceTile = cell;
            }
          }

          if (closestResourceTile != nullptr)
          {
            auto dir = unit.pos.directionTo(closestResourceTile->pos);
            actions.push_back(unit.move(dir));
          }
        }
        else
        {
          // If unit is a worker and there is no cargo space left, and we have cities, let's return to them
          if (player.cities.size() > 0)
          {
            auto city_iter = player.cities.begin();
            auto &city = city_iter->second;

            float closestDist = 999999;
            CityTile *closestCityTile = nullptr;

            for (auto &citytile : city.citytiles)
            {
              float dist = citytile.pos.distanceTo(unit.pos);

              if (dist < closestDist)
              {
                closestCityTile = &citytile;
                closestDist = dist;
              }
            }

            if (closestCityTile != nullptr)
            {
              auto dir = unit.pos.directionTo(closestCityTile->pos);
              actions.push_back(unit.move(dir));
            }
          }
        }

    // Create a vector of directions
    std::vector<DIRECTIONS> directions = {DIRECTIONS::NORTH, DIRECTIONS::EAST, DIRECTIONS::SOUTH, DIRECTIONS::WEST};
    // Check if there is an adjacent cell without a CityTile and build one
    for (const auto &direction : directions)
    {
      auto targetPos = unit.pos.translate(direction, 1);
      bool inMap = targetPos.x >= 0 && targetPos.x < gameMap.width && targetPos.y >= 0 && targetPos.y < gameMap.height;
      if (inMap)
      {
        auto targetCell = gameMap.getCell(targetPos.x, targetPos.y);
        // Check if the target cell is empty and doesn't have a CityTile already
        if (targetCell->citytile == nullptr && !targetCell->hasResource())
        {
          // Check if the worker has enough resources to build a CityTile
          if (unit.cargo.wood >= 100 &&
              unit.cargo.coal >= 100 &&
              unit.cargo.uranium >= 100)
          {
            actions.push_back(unit.buildCity());
            break;
          }
        }
      }
    }
  }


  // Create a vector of directions for creating new city tiles
  std::vector<DIRECTIONS> cityDirections = {DIRECTIONS::NORTH, DIRECTIONS::EAST, DIRECTIONS::SOUTH, DIRECTIONS::WEST};

  // Check if there is an adjacent cell to a city tile without a CityTile and build one
  for (auto &city : player.cities) {
    auto& cityTiles = city.second.citytiles;
    for (auto &citytile : cityTiles) {
      for (const auto &direction : cityDirections) {
        auto targetPos = citytile.pos.translate(direction, 1);
        bool inMap = targetPos.x >= 0 && targetPos.x < gameMap.width && targetPos.y >= 0 && targetPos.y < gameMap.height;
        if (inMap)
        {
          auto targetCell = gameMap.getCell(targetPos.x, targetPos.y);
          // Check if the target cell is empty and doesn't have a CityTile already
          if (targetCell->citytile == nullptr && !targetCell->hasResource())
          {
            // Check if the player has enough resources to build a CityTile
            if (player.researchedUranium() && unit.getCargoSpaceLeft() >= 100 &&
                city.second.fuel >= 250 && city.second.getLightUpkeep() <= city.second.fuel - 10) {
              // Use multiple workers from the same city to build city tiles in different directions
              int numWorkers = std::min(3, static_cast<int>(cityTiles.size())); // limit to 3 workers per city
              for (int i = 0; i < numWorkers; i++) {
                CityTile& cityTile = cityTiles[i];
                if (cityTile.canAct()) {
                  auto dir = cityTile.pos.directionTo(targetPos);
                  actions.push_back(unit.buildCity());
                }
              }
            }

          }
        }
      }
    }
  }

}

// // Attack with units that can attack
// for (int i = 0; i < player.units.size(); i++) {
// Unit unit = player.units[i];
// if (unit.isMilitary() && unit.canAct()) {
// for (auto& cell : gameMap.getNeighbors(unit.pos.x, unit.pos.y)) {
// if (cell->citytile && cell->citytile->team != gameState.id) {
// auto dir = unit.pos.directionTo(cell->pos);
// actions.push_back(unit.(dir));
// break;
// }
// if (cell->unit && cell->unit->team != gameState.id) {
// auto dir = unit.pos.directionTo(cell->pos);
// actions.push_back(unit.attack(dir));
// break;
// }
// }
// }
// }

// Spawn new workers and knights from cities that have enough resources
for (auto& city : player.cities) {
  if (city.second.fuel >= 50 && player.units.size() <= 30) {
    auto& cityTiles = city.second.citytiles;
    if (!cityTiles.empty()) {
      int randIndex = rand() % cityTiles.size();
      CityTile& cityTile = cityTiles[randIndex];
      if (cityTile.canAct()) {
        actions.push_back(cityTile.buildWorker());
      }
    }
  }
  else if (city.second.fuel >= 60) {
    auto& cityTiles = city.second.citytiles;
    if (!cityTiles.empty()) {
      int randIndex = rand() % cityTiles.size();
      CityTile& cityTile = cityTiles[randIndex];
      // if (cityTile.canAct()) {
      //   actions.push_back(cityTile.buildKnight());
      // }
    }
  }
}

/** AI Code Goes Above! **/

/** Do not edit! **/
for (int i = 0; i < actions.size(); i++)
{
  if (i != 0)
  cout << ",";
  cout << actions[i];
}
cout << endl;
    // end turn
    gameState.end_turn();
    }

  return 0;
}
