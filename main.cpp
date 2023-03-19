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
    // wait for updates
    gameState.update();

    vector<string> actions = vector<string>();

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

    for (int i = 0; i < player.units.size(); i++)
    {
      Unit unit = player.units[i];

      if (unit.isWorker() && unit.canAct())
      {
        if (unit.getCargoSpaceLeft() > 0)
        {
          Cell *closestResourceTile = nullptr;
          float closestDist = 9999999;

          for (auto it = resourceTiles.begin(); it != resourceTiles.end(); it++)
          {
            auto cell = *it;

            if ((cell->resource.type == ResourceType::wood) ||
                (cell->resource.type == ResourceType::coal && player.researchPoints >= 50) ||
                (cell->resource.type == ResourceType::uranium && player.researchPoints >= 200))
            {
              if (unit.getCargoSpaceLeft() == 0)
                break;

              float dist = cell->pos.distanceTo(unit.pos);

              if (dist < closestDist)
              {
                closestDist = dist;
                closestResourceTile = cell;
              }
            }
            else
            {
              continue;
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
      }
    }



    for (auto &kv : player.cities)
    {
      auto &city = kv.second;
      for (CityTile &city_tile : city.citytiles)
      {
        if (city_tile.canAct())
        {
          // Modifiez le seuil pour augmenter la priorité de la recherche
          if (player.units.size() < player.cityTileCount)
          {
            actions.push_back(city_tile.buildWorker());
          }
          else
          {
            actions.push_back(city_tile.research());
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