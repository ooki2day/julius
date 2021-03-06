#include "utilitymanagement.h"

#include "grid.h"
#include "routing.h"
#include "terrain.h"
#include "terraingraphics.h"

#include "data/building.hpp"
#include "data/cityinfo.hpp"
#include "data/constants.hpp"
#include "data/grid.hpp"
#include "data/scenario.hpp"
#include "data/settings.hpp"

#include "building/list.h"
#include "graphics/image.h"

#include <string.h>

static const int adjacentOffsets[] = {-162, 1, 162, -1};
static short queue[1000];
static int qHead;
static int qTail;

void UtilityManagement::updateHouseWaterAccess()
{
    building_list_small_clear();
    for (int i = 1; i < MAX_BUILDINGS; i++)
    {
        if (!BuildingIsInUse(i))
        {
            continue;
        }
        if (Data_Buildings[i].type == BUILDING_WELL)
        {
            building_list_small_add(i);
        }
        else if (Data_Buildings[i].houseSize)
        {
            Data_Buildings[i].hasWaterAccess = 0;
            Data_Buildings[i].hasWellAccess = 0;
            if (Terrain_existsTileWithinAreaWithType(
                        Data_Buildings[i].x, Data_Buildings[i].y,
                        Data_Buildings[i].size, Terrain_FountainRange))
            {
                Data_Buildings[i].hasWaterAccess = 1;
            }
        }
    }
    int total_wells = building_list_small_size();
    const int *wells = building_list_small_items();
    for (int i = 0; i < total_wells; i++)
    {
        Terrain_markBuildingsWithinWellRadius(wells[i], 2);
    }
}


static void setAllAqueductsToNoWater()
{
    int graphicId = image_group(ID_Graphic_Aqueduct) + 15;
    int gridOffset = Data_Settings_Map.gridStartOffset;
    for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize)
    {
        for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++)
        {
            if (Data_Grid_terrain[gridOffset] & Terrain_Aqueduct)
            {
                Data_Grid_aqueducts[gridOffset] = 0;
                if (Data_Grid_graphicIds[gridOffset] < graphicId)
                {
                    Data_Grid_graphicIds[gridOffset] += 15;
                }
            }
        }
    }
}

static void fillAqueductsFromOffset(int gridOffset)
{
    if (!(Data_Grid_terrain[gridOffset] & Terrain_Aqueduct))
    {
        return;
    }
    memset(queue, 0, sizeof(queue));
    qHead = qTail = 0;
    int guard = 0;
    int nextOffset;
    int graphicWithoutWater = image_group(ID_Graphic_Aqueduct) + 15;
    do
    {
        if (++guard >= GRID_SIZE * GRID_SIZE)
        {
            break;
        }
        Data_Grid_aqueducts[gridOffset] = 1;
        if (Data_Grid_graphicIds[gridOffset] >= graphicWithoutWater)
        {
            Data_Grid_graphicIds[gridOffset] -= 15;
        }
        nextOffset = -1;
        for (int i = 0; i < 4; i++)
        {
            int newOffset = gridOffset + adjacentOffsets[i];
            int buildingId = Data_Grid_buildingIds[newOffset];
            if (buildingId && Data_Buildings[buildingId].type == BUILDING_RESERVOIR)
            {
                // check if aqueduct connects to reservoir --> doesn't connect to corner
                int xy = Data_Grid_edge[newOffset] & Edge_MaskXY;
                if (xy != Edge_X0Y0 && xy != Edge_X2Y0 && xy != Edge_X0Y2 && xy != Edge_X2Y2)
                {
                    if (!Data_Buildings[buildingId].hasWaterAccess)
                    {
                        Data_Buildings[buildingId].hasWaterAccess = 2;
                    }
                }
            }
            else if (Data_Grid_terrain[newOffset] & Terrain_Aqueduct)
            {
                if (!Data_Grid_aqueducts[newOffset])
                {
                    if (nextOffset == -1)
                    {
                        nextOffset = newOffset;
                    }
                    else
                    {
                        queue[qTail++] = newOffset;
                        if (qTail >= 1000)
                        {
                            qTail = 0;
                        }
                    }
                }
            }
        }
        if (nextOffset == -1)
        {
            if (qHead == qTail)
            {
                return;
            }
            nextOffset = queue[qHead++];
            if (qHead >= 1000)
            {
                qHead = 0;
            }
        }
        gridOffset = nextOffset;
    }
    while (nextOffset > -1);
}

void UtilityManagement::updateReservoirFountain()
{
    Grid_andShortGrid(Data_Grid_terrain, ~(Terrain_FountainRange | Terrain_ReservoirRange));
    // reservoirs
    setAllAqueductsToNoWater();
    building_list_large_clear(1);
    // mark reservoirs next to water
    for (int i = 1; i < MAX_BUILDINGS; i++)
    {
        if (BuildingIsInUse(i) && Data_Buildings[i].type == BUILDING_RESERVOIR)
        {
            building_list_large_add(i);
            if (Terrain_existsTileWithinAreaWithType(
                        Data_Buildings[i].x - 1, Data_Buildings[i].y - 1, 5, Terrain_Water))
            {
                Data_Buildings[i].hasWaterAccess = 2;
            }
            else
            {
                Data_Buildings[i].hasWaterAccess = 0;
            }
        }
    }
    int total_reservoirs = building_list_large_size();
    const int *reservoirs = building_list_large_items();
    // fill reservoirs from full ones
    int changed = 1;
    static const int connectorOffsets[] = {-161, 165, 487, 161};
    while (changed == 1)
    {
        changed = 0;
        for (int i = 0; i < total_reservoirs; i++)
        {
            int buildingId = reservoirs[i];
            if (Data_Buildings[buildingId].hasWaterAccess == 2)
            {
                Data_Buildings[buildingId].hasWaterAccess = 1;
                changed = 1;
                for (int d = 0; d < 4; d++)
                {
                    fillAqueductsFromOffset(Data_Buildings[buildingId].gridOffset + connectorOffsets[d]);
                }
            }
        }
    }
    // mark reservoir ranges
    for (int i = 0; i < total_reservoirs; i++)
    {
        int buildingId = reservoirs[i];
        if (Data_Buildings[buildingId].hasWaterAccess)
        {
            Terrain_setWithRadius(
                Data_Buildings[buildingId].x, Data_Buildings[buildingId].y,
                3, 10, Terrain_ReservoirRange);
        }
    }
    // fountains
    for (int i = 1; i < MAX_BUILDINGS; i++)
    {
        struct Data_Building *b = &Data_Buildings[i];
        if (!BuildingIsInUse(i) || b->type != BUILDING_FOUNTAIN)
        {
            continue;
        }
        int des = Data_Grid_desirability[b->gridOffset];
        int graphicId;
        if (des > 60)
        {
            graphicId = image_group(ID_Graphic_Fountain4);
        }
        else if (des > 40)
        {
            graphicId = image_group(ID_Graphic_Fountain3);
        }
        else if (des > 20)
        {
            graphicId = image_group(ID_Graphic_Fountain2);
        }
        else
        {
            graphicId = image_group(ID_Graphic_Fountain1);
        }
        Terrain_addBuildingToGrids(i, b->x, b->y, 1, graphicId, Terrain_Building);
        if ((Data_Grid_terrain[b->gridOffset] & Terrain_ReservoirRange) && b->numWorkers)
        {
            b->hasWaterAccess = 1;
            Terrain_setWithRadius(b->x, b->y, 1,
                                  Data_Scenario.climate == Climate_Desert ? 3 : 4,
                                  Terrain_FountainRange);
        }
        else
        {
            b->hasWaterAccess = 0;
        }
    }
}

static int markRoadNetwork(int gridOffset, unsigned char roadNetworkId)
{
    memset(queue, 0, sizeof(queue));
    qHead = qTail = 0;
    int guard = 0;
    int nextOffset;
    int size = 1;
    do
    {
        if (++guard >= GRID_SIZE * GRID_SIZE)
        {
            break;
        }
        Data_Grid_roadNetworks[gridOffset] = roadNetworkId;
        nextOffset = -1;
        for (int i = 0; i < 4; i++)
        {
            int newOffset = gridOffset + adjacentOffsets[i];
            if (Data_Grid_routingLandCitizen[newOffset] >= Routing_Citizen_0_Road &&
                    Data_Grid_routingLandCitizen[newOffset] <= Routing_Citizen_2_PassableTerrain &&
                    !Data_Grid_roadNetworks[newOffset])
            {
                if (Data_Grid_routingLandCitizen[newOffset] != Routing_Citizen_2_PassableTerrain ||
                        Data_Grid_terrain[newOffset] & Terrain_AccessRamp)
                {
                    Data_Grid_roadNetworks[newOffset] = roadNetworkId;
                    size++;
                    if (nextOffset == -1)
                    {
                        nextOffset = newOffset;
                    }
                    else
                    {
                        queue[qTail++] = newOffset;
                        if (qTail >= 1000)
                        {
                            qTail = 0;
                        }
                    }
                }
            }
        }
        if (nextOffset == -1)
        {
            if (qHead == qTail)
            {
                return size;
            }
            nextOffset = queue[qHead++];
            if (qHead >= 1000)
            {
                qHead = 0;
            }
        }
        gridOffset = nextOffset;
    }
    while (nextOffset > -1);
    return size;
}

void UtilityManagement::determineRoadNetworks()
{
    for (int i = 0; i < 10; i++)
    {
        Data_CityInfo.largestRoadNetworks[i].id = 0;
        Data_CityInfo.largestRoadNetworks[i].size = 0;
    }
    Grid_clearUByteGrid(Data_Grid_roadNetworks);
    int roadNetworkId = 1;
    int gridOffset = Data_Settings_Map.gridStartOffset;
    for (int y = 0; y < Data_Settings_Map.height; y++, gridOffset += Data_Settings_Map.gridBorderSize)
    {
        for (int x = 0; x < Data_Settings_Map.width; x++, gridOffset++)
        {
            if (Data_Grid_terrain[gridOffset] & Terrain_Road && !Data_Grid_roadNetworks[gridOffset])
            {
                int size = markRoadNetwork(gridOffset, roadNetworkId);
                for (int n = 0; n < 10; n++)
                {
                    if (size > Data_CityInfo.largestRoadNetworks[n].size)
                    {
                        // move everyone down
                        for (int m = 9; m > n; m--)
                        {
                            Data_CityInfo.largestRoadNetworks[m].id = Data_CityInfo.largestRoadNetworks[m-1].id;
                            Data_CityInfo.largestRoadNetworks[m].size = Data_CityInfo.largestRoadNetworks[m-1].size;
                        }
                        Data_CityInfo.largestRoadNetworks[n].id = roadNetworkId;
                        Data_CityInfo.largestRoadNetworks[n].size = size;
                        break;
                    }
                }
                roadNetworkId++;
            }
        }
    }
}
