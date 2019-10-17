#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "EliteAI/EliteBehaviorTree/EBehaviorTree.h"
#include <Exam_HelperStructs.h>
#include <IExamInterface.h>
#include "EliteMath/EVector2.h"

using namespace Elite;

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------
inline bool TooMuchFood(Blackboard* pBlackboard) // Checks for abundant food
{
	std::vector<int>* items;
	IExamInterface* interface { nullptr };
	int foodAmount = 0;

	auto dataAvailable = pBlackboard->GetData("Items", items)
		&& pBlackboard->GetData("Interface", interface);

	if (!dataAvailable)
		return false;

	if (!interface)
		return Failure;

	for (int i = 0; i < 5; ++i) // This loop checks how much food is in the inventory
	{
		if (items->at(i) == 2)
			++foodAmount;
	}

	return foodAmount > 3;
}

inline bool TooManyGuns(Blackboard* pBlackboard) // Check for overall too many guns
{
	std::vector<int>* items;
	int gunAmount = 0;
	int emptySpace = 0;

	auto dataAvailable = pBlackboard->GetData("Items", items);

	if (!dataAvailable)
		return false;

	for (int i = 0; i < 5; ++i) // This loop checks how much space guns are taking up
	{
		if (items->at(i) == 0)
			++gunAmount;
		else if (items->at(i) < 0)
			++emptySpace;
	}

	return (gunAmount > 2 || (gunAmount > 1 && emptySpace < 1)); // Consider too many when there's more than 2 guns or when there's no free space left but more than 1 gun in inv
}

inline BehaviorState DropGun(Blackboard* pBlackboard)
{
	std::vector<int>* items;
	IExamInterface* interface { nullptr };

	auto dataAvailable = pBlackboard->GetData("Items", items)
		&& pBlackboard->GetData("Interface", interface);
	
	if (!dataAvailable)
		return Failure;

	if (!interface)
		return Failure;

	for (int i = 0; i < 5; ++i) // Remove first gun it finds
	{
		if (items->at(i) == 0)
		{
			interface->Inventory_RemoveItem(i);
			items->at(i) = -1; // Set inventory space to appear empty
			pBlackboard->ChangeData("Items", items);
			return Success;
		}
	}
	return Failure;
}

inline bool TooManyMedkits(Blackboard* pBlackboard) // Checks for abundant medkits
{
	std::vector<int>* items;
	IExamInterface* interface { nullptr };
	int medkitAmount = 0;

	auto dataAvailable = pBlackboard->GetData("Items", items)
		&& pBlackboard->GetData("Interface", interface);

	if (!dataAvailable)
		return false;

	if (!interface)
		return Failure;

	for (int i = 0; i < 5; ++i) // This loop checks how many medkits are in the inventory
	{
		if (items->at(i) == 1)
			++medkitAmount;
	}

	return medkitAmount > 3;
}

inline bool MedkitInInv(Blackboard* pBlackboard) // Checks for any medkits
{
	std::vector<int>* items;
	IExamInterface* interface { nullptr };

	auto dataAvailable = pBlackboard->GetData("Items", items)
		&& pBlackboard->GetData("Interface", interface);

	if (!dataAvailable)
		return false;

	if (!interface)
		return false;

	for (int i = 0; i < 5; ++i) // This loop checks if a medkit is present
	{
		if (items->at(i) == 1)
			return true;
	}

	return false;
}

inline bool HealthLow(Blackboard* pBlackboard)
{
	AgentInfo agent;
	auto dataAvailable = pBlackboard->GetData("Agent", agent);

	if (!dataAvailable)
		return false;

	return agent.Health <= 1.f;
}

inline BehaviorState UseMedkit(Blackboard* pBlackboard)
{
	std::vector<int>* items;
	IExamInterface* interface { nullptr };

	auto dataAvailable = pBlackboard->GetData("Items", items)
		&& pBlackboard->GetData("Interface", interface);

	if (!dataAvailable)
		return Failure;

	if (!interface)
		return Failure;

	int random = rand() & 1; // Either 0 or 1
	for (int i = random * 4; i != int(random == 0) * 4; i += -1+int(random == 0)*2) // This loop uses first medkit it finds (randomized to increase the chance of using the oldest one first because older ones tend to heal less and if always using the first medkit it will never reach the last)
	{
		if (items->at(i) == 1)
		{
			interface->Inventory_UseItem(i); // Use and drop used medkit
			interface->Inventory_RemoveItem(i);
			items->at(i) = -1; // Set inventory space to appear empty
			pBlackboard->ChangeData("Items", items);
			return Success;
		}
	}

	return Failure;
}

inline bool FoodInInv(Blackboard* pBlackboard)
{
	std::vector<int>* items;

	auto dataAvailable = pBlackboard->GetData("Items", items);

	if (!dataAvailable)
		return false;

	for (int i = 0; i < 5; ++i)
	{
		if (items->at(i) == 2) // if Food
			return true;
	}

	return false;
}

inline bool EnergyLow(Blackboard* pBlackboard)
{
	AgentInfo agent;
	auto dataAvailable = pBlackboard->GetData("Agent", agent);

	if (!dataAvailable)
		return false;

	return agent.Energy <= 1.f;
}

inline BehaviorState EatFood(Blackboard* pBlackboard)
{
	std::vector<int>* items;
	IExamInterface* interface { nullptr };

	auto dataAvailable = pBlackboard->GetData("Items", items)
		&& pBlackboard->GetData("Interface", interface);

	if (!dataAvailable)
		return Failure;

	if (!interface)
		return Failure;

	for (int i = 0; i < 5; ++i)
	{
		if (items->at(i) == 2) // If food
		{
			interface->Inventory_UseItem(i); // Use and drop used food
			interface->Inventory_RemoveItem(i);
			items->at(i) = -1; // Set inventory space to appear empty
			pBlackboard->ChangeData("Items", items);
			return Success;
		}
	}

	return Failure;
}

inline bool ZombieInFOV(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> entities;

	auto dataAvailable = pBlackboard->GetData("EntitiesInFOV", entities);

	if (!dataAvailable)
		return Failure;

	for (auto entity : entities)
	{
		if (entity.Type == eEntityType::ENEMY)
			return true;
	}

	return false;
}

inline bool GunInInv(Blackboard* pBlackboard)
{
	std::vector<int>* items;

	auto dataAvailable = pBlackboard->GetData("Items", items);

	if (!dataAvailable)
		return Failure;

	for (int i = 0; i < 5; ++i)
	{
		if (items->at(i) == 0) // if gun
			return true;
	}

	return false;
}

inline BehaviorState AimShoot(Blackboard* pBlackboard)
{
	std::vector<int>* items;
	IExamInterface* interface { nullptr };
	std::vector<EntityInfo> entities;
	AgentInfo agent;
	Vector2 target;

	auto dataAvailable = pBlackboard->GetData("Items", items)
		&& pBlackboard->GetData("Interface", interface)
		&& pBlackboard->GetData("EntitiesInFOV", entities)
		&& pBlackboard->GetData("Agent", agent)
		&& pBlackboard->GetData("Target", target);

	if (!dataAvailable)
		return Failure;

	if (!interface)
		return Failure;

	float shortestDistance = 100.f;
	bool foundEnemy = false;

	// Check all entities in FOV for enemies
	for (auto entity : entities)
	{
		if (entity.Type == eEntityType::ENEMY)
		{
			// Check distance between player and enemy
			const float distance = sqrt(pow(entity.Location.x - agent.Position.x, 2) + pow(entity.Location.y - agent.Position.y, 2));

			// Make most nearby enemy the target and check if it's right in front of the agent
			if (distance < shortestDistance)
			{
				shortestDistance = distance;
				interface->Draw_Circle(entity.Location, 1.5f, {1,0,0}, 0.f);

				// Instead of default rotating look at enemy by first retrieving the orientation you look at
				float rotDegAgent = agent.Orientation * (180.f / M_PI) + 270.f; // Because it starts whilst looking downwards which is 270 degrees
				while(rotDegAgent >= 360.f)
					rotDegAgent -= 360.f;
				while(rotDegAgent < 0.f)
					rotDegAgent += 360.f;

				// And then retrieve the orientation of looking at the enemy
				auto normVector = (entity.Location - agent.Position).GetNormalized();
				float rotDegEnemy = atan2(normVector.y, normVector.x) * (180.f / M_PI);
				while(rotDegEnemy >= 360.f)
					rotDegEnemy -= 360.f;
				while(rotDegEnemy < 0.f)
					rotDegEnemy += 360.f;

				// Compare both orientations (both being 0-360 degrees)
				float diffAngle = rotDegEnemy - rotDegAgent;
	
				if (diffAngle <= -180)
				    diffAngle += 360;
				else if (diffAngle >= 180)
				    diffAngle -= 360;

				agent.AngularVelocity = diffAngle; // Even though max velocity is 1, it will still put it to a number outside that range which I'm okay with for items
				pBlackboard->ChangeData("Agent", agent);

				if (abs(diffAngle) < 3.f)
				{
					for (int i = 0; i < 5; ++i)
					{
						if (items->at(i) == 0) // If it's a gun
						{
							interface->Inventory_UseItem(i); // Use gun
							interface->Inventory_RemoveItem(i); // Remove empty gun
							items->at(i) = -1; // Set inventory space to appear empty
							pBlackboard->ChangeData("Items", items);
							
							return Success;
						}
					}				
				}

				return Success;
			}
		}
	}

	return Failure;
}

inline BehaviorState Avoid(Blackboard* pBlackboard)
{
	AgentInfo agent;
	auto dataAvailable = pBlackboard->GetData("Agent", agent);

	if (!dataAvailable)
		return Failure;

	if (agent.Stamina > 0.f)
		agent.RunMode = true;

	pBlackboard->ChangeData("Agent", agent);
	pBlackboard->ChangeData("Recovering", false);

	return Success;
}

inline bool InvNotFull(Blackboard* pBlackboard)
{
	std::vector<int>* items;

	auto dataAvailable = pBlackboard->GetData("Items", items);

	for (int i = 0; i < 5; ++i)
	{
		if (items->at(i) == -1)
			return true;
	}

	return false;
}

inline bool ItemInFOV(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> entities;

	auto dataAvailable = pBlackboard->GetData("EntitiesInFOV", entities);

	if (!dataAvailable)
		return false;

	for (auto entity : entities)
	{
		if (entity.Type == eEntityType::ITEM)
			return true;
	}

	return false;
}

inline BehaviorState Pickup(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> entities;
	std::vector<int>* items = nullptr;
	AgentInfo agent;
	IExamInterface* interface { nullptr };
	Vector2 target;

	auto dataAvailable = pBlackboard->GetData("EntitiesInFOV", entities)
		&& pBlackboard->GetData("Items", items)
		&& pBlackboard->GetData("Agent", agent)
		&& pBlackboard->GetData("Interface", interface)
		&& pBlackboard->GetData("Target", target);

	if (!dataAvailable || !interface)
		return Failure;

	float shortestDistance = 100.f;
	bool foundItem = false;

	for (auto entity : entities)
	{
		if (entity.Type == eEntityType::ITEM)
		{
			// Check distance between player and item
			const float distance = sqrt(pow(entity.Location.x - agent.Position.x, 2) + pow(entity.Location.y - agent.Position.y, 2));

			// Make most nearby item the target
			if (distance < shortestDistance)
			{
				shortestDistance = distance;
				target = entity.Location;
				pBlackboard->ChangeData("Target", target);
				interface->Draw_Circle(target, 3.f, {0,1,0}, 0.f);

				// Instead of default rotating look at pickup by first retrieving the orientation you look at (even though the agent already looks where he's going when indoors, he can still spot items when outside looking inwards)
				float rotDegAgent = agent.Orientation * (180.f / M_PI) + 270.f; // Because it starts whilst looking downwards which is 270 degrees
				while(rotDegAgent >= 360.f)
					rotDegAgent -= 360.f;
				while(rotDegAgent < 0.f)
					rotDegAgent += 360.f;

				// And then retrieve the orientation of the item
				auto normVector = (entity.Location - agent.Position).GetNormalized();
				float rotDegItem = atan2(normVector.y, normVector.x) * (180.f / M_PI);
				while(rotDegItem >= 360.f)
					rotDegItem -= 360.f;
				while(rotDegItem < 0.f)
					rotDegItem += 360.f;

				// Compare both orientations (both being 0-360 degrees)
				float diffAngle = rotDegItem - rotDegAgent;
	
				if (diffAngle <= -180)
				    diffAngle += 360;
				else if (diffAngle >= 180)
				    diffAngle -= 360;

				agent.AngularVelocity = diffAngle; // Even though max velocity is 1, it will still put it to a number outside that range which I'm okay with for items
				pBlackboard->ChangeData("Agent", agent);

				// If nearby enough grab it
				if (distance < agent.GrabRange)
				{
					// Choose a spot in m_Items to place the item (earliest spot with -1)
					for (int i = 0; i < 5; ++i)
					{
						if (items->at(i) < 0)
						{
							ItemInfo item;
							interface->Item_Grab(entity, item);
							items->at(i) = int(item.Type);
							interface->Inventory_AddItem(i, item);
							pBlackboard->ChangeData("Items", items);
							// Drop it immediately if it turns out to be garbage
							if (item.Type == eItemType::GARBAGE)
							{
								interface->Inventory_RemoveItem(i);
								items->at(i) = -1;
								pBlackboard->ChangeData("Items", items);
							}
							return Success;
						}
					}
				}
				foundItem = true;
			}
		}
	} // Only leave the entity search loop when it grabbed something, otherwise return result afterwards
	if (foundItem)
		return Success;

	return Failure;
}

inline bool SuppliesLow(Blackboard* pBlackboard)
{
	AgentInfo agent;
	std::vector<int>* items = 0;
	auto dataAvailable = pBlackboard->GetData("Agent", agent)
	 && pBlackboard->GetData("Items", items);
	int Supplies = 0;

	for (int i = 0; i < 5; ++i) // This loop checks how many medkits/food items are in the inventory
	{
		if (items->at(i) == 1 || items->at(i) == 2)
			++Supplies;
	}
	bool suppliesLow = agent.Energy < 1.f || Supplies < 3;

	if (!suppliesLow)
		pBlackboard->ChangeData("HasTargetBuilding", false); // Disable building target when supplies are plentiful

	return suppliesLow;
}

inline bool SuppliesNotLow(Blackboard* pBlackboard)
{
	return !SuppliesLow(pBlackboard);
}

inline bool HasTargetBuilding(Blackboard* pBlackboard)
{
	bool hasTargetBuilding = false;

	auto dataAvailable = pBlackboard->GetData("HasTargetBuilding", hasTargetBuilding);

	if (!dataAvailable)
		return false;

	return hasTargetBuilding;
}

inline BehaviorState ScavengeBuilding(Blackboard* pBlackboard)
{
	AgentInfo agent;
	IExamInterface* interface { nullptr };
	Vector2 target;
	Vector2 targetBuilding;
	vector<Vector2> lastBuildings;
	int lastBuildingCounter;

	auto dataAvailable = pBlackboard->GetData("Agent", agent)
		&& pBlackboard->GetData("Interface", interface)
		&& pBlackboard->GetData("LastBuildings", lastBuildings)
		&& pBlackboard->GetData("TargetBuilding", targetBuilding)
		&& pBlackboard->GetData("LastBuildingCounter", lastBuildingCounter);

	if (!dataAvailable || !interface)
		return Failure;

	// Check distance between player and building center
	const float distance = sqrt(pow(targetBuilding.x - agent.Position.x, 2) + pow(targetBuilding.y - agent.Position.y, 2));

	// Use the navmesh to calculate the next point to the entrance of the building
	target = interface->NavMesh_GetClosestPathPoint(targetBuilding);
	pBlackboard->ChangeData("Target", target);
	interface->Draw_Circle(target, 3.f, {0,1,0}, 0.f);
	
	// Once it reached the center, add it to recently visited buildings so it doesn't stay there and no longer have target building
	if (distance <= 1.f)
	{
		lastBuildings.at(lastBuildingCounter) = target;
		++lastBuildingCounter;
		lastBuildingCounter = lastBuildingCounter%4;

		pBlackboard->ChangeData("LastBuildingCounter", lastBuildingCounter);
		pBlackboard->ChangeData("LastBuildings", lastBuildings);
		pBlackboard->ChangeData("HasTargetBuilding", false);
	}

	return Success;
}

inline bool BuildingInFOV(Blackboard* pBlackboard)
{
	std::vector<HouseInfo> buildings;

	auto dataAvailable = pBlackboard->GetData("BuildingsInFOV", buildings);

	if (!dataAvailable)
		return false;

	return buildings.size() > 0;
}

inline bool NotRecentlyVisited(Blackboard* pBlackboard)
{
	std::vector<HouseInfo> buildings;
	vector<Vector2> lastBuildings;

	auto dataAvailable = pBlackboard->GetData("BuildingsInFOV", buildings)
		&& pBlackboard->GetData("LastBuildings", lastBuildings);

	if (!dataAvailable)
		return false;

	for (auto lastbuilding : lastBuildings) // Check if the current house coincides with one of the 3 last visited ones
	{
		if (buildings.at(0).Center == lastbuilding)
			return false;
	}

	return true;
}

inline BehaviorState SeekBuilding(Blackboard* pBlackboard)
{
	std::vector<HouseInfo> buildings;

	auto dataAvailable = pBlackboard->GetData("BuildingsInFOV", buildings);

	if (!dataAvailable)
		return Failure;

	if (buildings.size() != 0)
	{
		// Make first building the target
		pBlackboard->ChangeData("TargetBuilding", buildings.at(0).Center);
		pBlackboard->ChangeData("HasTargetBuilding", true);

		return Success;
	}

	return Failure;
}
#endif