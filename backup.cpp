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

    // Keep track of the IDs of units that have already acted this turn
    // set<int> actedUnitIds;

    // we iterate over all our units and do something with them
    for (int i = 0; i < player.units.size(); i++)
    {
      Unit unit = player.units[i];

      // // Check if this unit has already acted in this turn
      // if (actedUnitIds.find(unit.id) != actedUnitIds.end())
      // {
      //   continue;
      // }

      if (unit.isWorker() && unit.canAct())
      {
        if (unit.getCargoSpaceLeft() > 0)
        {
          // if the unit is a worker and we have space in cargo, lets find the nearest resource tile and try to mine it
          Cell *closestResourceTile = nullptr;
          float closestDist = 9999999;

          for (auto it = resourceTiles.begin(); it != resourceTiles.end(); it++)
          {
            auto cell = *it;
            if (cell->resource.type != ResourceType::wood && !player.researchedCoal() && !player.researchedUranium())
              continue;
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
          // if unit is a worker and there is no cargo space left, and we have cities, lets return to them
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
    }



    

    // you can add debug annotations using the methods of the Annotate class.
    // actions.push_back(Annotate::circle(0, 0));

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
