#include "CityBuildings.h"
#include "CityBuildings_private.h"

#include "../Graphics.h"

#include "../Data/Building.h"
#include "../Data/Constants.h"
#include "../Data/Figure.h"
#include "../Data/Formation.h"
#include "../Data/Settings.h"
#include "../Data/State.h"

static int showOnOverlay(struct Data_Walker *w)
{
	switch (Data_State.currentOverlay) {
		case Overlay_Water:
		case Overlay_Desirability:
			return 0;
		case Overlay_Native:
			return w->type == Figure_IndigenousNative || w->type == Figure_Missionary;
		case Overlay_Fire:
			return w->type == Figure_Prefect;
		case Overlay_Damage:
			return w->type == Figure_Engineer;
		case Overlay_TaxIncome:
			return w->type == Figure_TaxCollector;
		case Overlay_Crime:
			return w->type == Figure_Prefect || w->type == Figure_Protester ||
				w->type == Figure_Criminal || w->type == Figure_Rioter;
		case Overlay_Entertainment:
			return w->type == Figure_Actor || w->type == Figure_Gladiator ||
				w->type == Figure_LionTamer || w->type == Figure_Charioteer;
		case Overlay_Education:
			return w->type == Figure_SchoolChild || w->type == Figure_Librarian ||
				w->type == Figure_Teacher;
		case Overlay_Theater:
			if (w->type == Figure_Actor) {
				if (w->actionState == FigureActionState_94_EntertainerRoaming ||
					w->actionState == FigureActionState_95_EntertainerReturning) {
					return Data_Buildings[w->buildingId].type == Building_Theater;
				} else {
					return Data_Buildings[w->destinationBuildingId].type == Building_Theater;
				}
			}
			return 0;
		case Overlay_Amphitheater:
			if (w->type == Figure_Actor || w->type == Figure_Gladiator) {
				if (w->actionState == FigureActionState_94_EntertainerRoaming ||
					w->actionState == FigureActionState_95_EntertainerReturning) {
					return Data_Buildings[w->buildingId].type == Building_Amphitheater;
				} else {
					return Data_Buildings[w->destinationBuildingId].type == Building_Amphitheater;
				}
			}
			return 0;
		case Overlay_Colosseum:
			if (w->type == Figure_Gladiator) {
				if (w->actionState == FigureActionState_94_EntertainerRoaming ||
					w->actionState == FigureActionState_95_EntertainerReturning) {
					return Data_Buildings[w->buildingId].type == Building_Colosseum;
				} else {
					return Data_Buildings[w->destinationBuildingId].type == Building_Colosseum;
				}
			} else if (w->type == Figure_LionTamer) {
				return 1;
			}
			return 0;
		case Overlay_Hippodrome:
			return w->type == Figure_Charioteer;
		case Overlay_Religion:
			return w->type == Figure_Priest;
		case Overlay_School:
			return w->type == Figure_SchoolChild;
		case Overlay_Library:
			return w->type == Figure_Librarian;
		case Overlay_Academy:
			return w->type == Figure_Teacher;
		case Overlay_Barber:
			return w->type == Figure_Barber;
		case Overlay_Bathhouse:
			return w->type == Figure_BathhouseWorker;
		case Overlay_Clinic:
			return w->type == Figure_Doctor;
		case Overlay_Hospital:
			return w->type == Figure_Surgeon;
		case Overlay_FoodStocks:
			if (w->type == Figure_MarketBuyer || w->type == Figure_MarketTrader ||
				w->type == Figure_DeliveryBoy || w->type == Figure_FishingBoat) {
				return 1;
			} else if (w->type == Figure_CartPusher) {
				return w->resourceId == Resource_Wheat || w->resourceId == Resource_Vegetables ||
					w->resourceId == Resource_Fruit || w->resourceId == Resource_Meat;
			}
			return 0;
		case Overlay_Problems:
			if (w->type == Figure_LaborSeeker) {
				return Data_Buildings[w->buildingId].showOnProblemOverlay;
			} else if (w->type == Figure_CartPusher) {
				return w->actionState == FigureActionState_20_CartpusherInitial || w->minMaxSeen;
			}
			return 0;
	}
	return 1;
}

static void drawWalkerWithCart(struct Data_Walker *w, int xOffset, int yOffset)
{
	if (w->yOffsetCart >= 0) {
		Graphics_drawImage(w->graphicId, xOffset, yOffset);
		Graphics_drawImage(w->cartGraphicId, xOffset + w->xOffsetCart, yOffset + w->yOffsetCart);
	} else {
		Graphics_drawImage(w->cartGraphicId, xOffset + w->xOffsetCart, yOffset + w->yOffsetCart);
		Graphics_drawImage(w->graphicId, xOffset, yOffset);
	}
}

static void drawHippodromeHorses(struct Data_Walker *w, int xOffset, int yOffset)
{
	int val = w->waitTicksMissile;
	switch (Data_Settings_Map.orientation) {
		case Dir_0_Top:
			xOffset += 10;
			if (val <= 10) {
				yOffset -= 2;
			} else if (val <= 11) {
				yOffset -= 10;
			} else if (val <= 12) {
				yOffset -= 18;
			} else if (val <= 13) {
				yOffset -= 16;
			} else if (val <= 20) {
				yOffset -= 14;
			} else if (val <= 21) {
				yOffset -= 10;
			} else {
				yOffset -= 2;
			}
			break;
		case Dir_2_Right:
			xOffset -= 10;
			if (val <= 9) {
				yOffset -= 12;
			} else if (val <= 10) {
				yOffset += 4;
			} else if (val <= 11) {
				xOffset -= 5;
				yOffset += 2;
			} else if (val <= 13) {
				xOffset -= 5;
			} else if (val <= 20) {
				yOffset -= 2;
			} else if (val <= 21) {
				yOffset -= 6;
			} else {
				yOffset -= 12;
			}
		case Dir_4_Bottom:
			xOffset += 20;
			if (val <= 9) {
				yOffset += 4;
			} else if (val <= 10) {
				xOffset += 10;
				yOffset += 4;
			} else if (val <= 11) {
				xOffset += 10;
				yOffset -= 4;
			} else if (val <= 13) {
				yOffset -= 6;
			} else if (val <= 20) {
				yOffset -= 12;
			} else if (val <= 21) {
				yOffset -= 10;
			} else {
				yOffset -= 2;
			}
			break;
		case Dir_6_Left:
			xOffset -= 10;
			if (val <= 9) {
				yOffset -= 12;
			} else if (val <= 10) {
				yOffset += 4;
			} else if (val <= 11) {
				yOffset += 2;
			} else if (val <= 13) {
				// no change
			} else if (val <= 20) {
				yOffset -= 2;
			} else if (val <= 21) {
				yOffset -= 6;
			} else {
				yOffset -= 12;
			}
			break;
	}
	drawWalkerWithCart(w, xOffset, yOffset);
}

static int tileOffsetToPixelOffsetX(int x, int y)
{
	int dir = Data_Settings_Map.orientation;
	if (dir == Dir_0_Top || dir == Dir_4_Bottom) {
		int base = 2 * x - 2 * y;
		return dir == Dir_0_Top ? base : -base;
	} else {
		int base = 2 * x + 2 * y;
		return dir == Dir_2_Right ? base : -base;
	}
}

static int tileOffsetToPixelOffsetY(int x, int y)
{
	int dir = Data_Settings_Map.orientation;
	if (dir == Dir_0_Top || dir == Dir_4_Bottom) {
		int base = x + y;
		return dir == Dir_0_Top ? base : -base;
	} else {
		int base = x - y;
		return dir == Dir_6_Left ? base : -base;
	}
}

static int tileProgressToPixelOffsetX(int direction, int progress)
{
	int offset = 0;
	if (direction == 0 || direction == 2) {
		offset = 2 * progress - 28;
	} else if (direction == 1) {
		offset = 4 * progress - 56;
	} else if (direction == 3 || direction == 7) {
		offset = 0;
	} else if (direction == 4 || direction == 6) {
		offset = 28 - 2 * progress;
	} else if (direction == 5) {
		offset = 56 - 4 * progress;
	}
	return offset;
}

static int tileProgressToPixelOffsetY(int direction, int progress)
{
	int offset = 0;
	if (direction == 0 || direction == 6) {
		offset = 14 - progress;
	} else if (direction == 1 || direction == 5) {
		offset = 0;
	} else if (direction == 2 || direction == 4) {
		offset = progress - 14;
	} else if (direction == 3) {
		offset = 2 * progress - 28;
	} else if (direction == 7) {
		offset = 28 - 2 * progress;
	}
	return offset;
}

void UI_CityBuildings_drawWalker(int walkerId, int xOffset, int yOffset, int selectedWalkerId, struct UI_CityPixelCoordinate *coord)
{
	struct Data_Walker *w = &Data_Walkers[walkerId];

	// determining x/y offset on tile
	int xTileOffset = 0;
	int yTileOffset = 0;
	if (w->useCrossCountry) {
		xTileOffset = tileOffsetToPixelOffsetX(w->crossCountryX % 15, w->crossCountryY % 15);
		yTileOffset = tileOffsetToPixelOffsetY(w->crossCountryX % 15, w->crossCountryY % 15);
		yTileOffset -= w->missileDamage;
	} else {
		int direction = (8 + w->direction - Data_Settings_Map.orientation) % 8;
		xTileOffset = tileProgressToPixelOffsetX(direction, w->progressOnTile);
		yTileOffset = tileProgressToPixelOffsetY(direction, w->progressOnTile);
		yTileOffset -= w->currentHeight;
		if (w->numPreviousWalkersOnSameTile && w->type != Figure_Ballista) {
			static const int xOffsets[] = {
				0, 8, 8, -8, -8, 0, 16, 0, -16, 8, -8, 16, -16, 16, -16, 8, -8, 0, 24, 0, -24, 0, 0, 0
			};
			static const int yOffsets[] = {
				0, 0, 8, 8, -8, -16, 0, 16, 0, -16, 16, 8, -8, -8, 8, 16, -16, -24, 0, 24, 0, 0, 0, 0
			};
			xTileOffset += xOffsets[w->numPreviousWalkersOnSameTile];
			yTileOffset += yOffsets[w->numPreviousWalkersOnSameTile];
		}
	}

	xTileOffset += 29;
	yTileOffset += 15;
	if (w->isEnemyGraphic) {
		xOffset += xTileOffset - GraphicEnemySpriteOffsetX(w->graphicId);
		yOffset += yTileOffset - GraphicEnemySpriteOffsetY(w->graphicId);
	} else {
		xOffset += xTileOffset - GraphicSpriteOffsetX(w->graphicId);
		yOffset += yTileOffset - GraphicSpriteOffsetY(w->graphicId);
	}

	// excluding walkers
	if (selectedWalkerId == 9999) {
		if (!showOnOverlay(w)) {
			return;
		}
	} else if (selectedWalkerId) {
		if (walkerId != selectedWalkerId) {
			return;
		}
		if (coord) {
			coord->x = xOffset;
			coord->y = yOffset;
		}
	}

	// actual drawing
	if (w->cartGraphicId) {
		switch (w->type) {
			case Figure_CartPusher:
			case Figure_Warehouseman:
			case Figure_LionTamer:
			case Figure_Dockman:
			case Figure_NativeTrader:
			case Figure_Immigrant:
			case Figure_Emigrant:
				drawWalkerWithCart(w, xOffset, yOffset);
				break;
			case Figure_HippodromeMiniHorses:
				drawHippodromeHorses(w, xOffset, yOffset);
				break;
			case Figure_FortStandard:
				if (!Data_Formations[w->formationId].inDistantBattle) {
					// base
					Graphics_drawImage(w->graphicId, xOffset, yOffset);
					// flag
					Graphics_drawImage(w->cartGraphicId,
						xOffset, yOffset - GraphicHeight(w->cartGraphicId));
					// top icon
					int iconGraphicId = GraphicId(ID_Graphic_FortStandardIcons) + w->formationId - 1;
					Graphics_drawImage(iconGraphicId,
						xOffset, yOffset - GraphicHeight(iconGraphicId) - GraphicHeight(w->cartGraphicId));
				}
				break;
			default:
				Graphics_drawImage(w->graphicId, xOffset, yOffset);
				break;
		}
	} else {
		if (w->isEnemyGraphic) {
			Graphics_drawEnemyImage(w->graphicId, xOffset, yOffset);
		} else {
			Graphics_drawImage(w->graphicId, xOffset, yOffset);
		}
	}
}
