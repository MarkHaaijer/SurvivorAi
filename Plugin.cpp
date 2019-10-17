#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include "Behaviors.h"

using namespace Elite;

// Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	// Retrieving the interface
	// This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	// Bit information about the plugin
	// Please fill this in!!
	info.BotName = "Bold Bastard";
	info.Student_FirstName = "Mark";
	info.Student_LastName = "Haaijer";
	info.Student_Class = "2DAE1";

	for (UINT i = 0; i < 5; ++i)
		m_Items->push_back(-1); // considered empty

	// Create blackboard
	auto pBlackboard = new Blackboard();
	pBlackboard->AddData("Interface", m_pInterface);
	pBlackboard->AddData("Agent", m_pInterface->Agent_GetInfo());
	pBlackboard->AddData("Target", m_Target);
	pBlackboard->AddData("Items", m_Items);
	pBlackboard->AddData("Recovering", m_Recovering);
	pBlackboard->AddData("EntitiesInFOV", GetEntitiesInFOV());
	pBlackboard->AddData("BuildingsInFOV", GetHousesInFOV());
	pBlackboard->AddData("LastBuildings", m_LastBuildings);
	pBlackboard->AddData("HasTargetBuilding", m_HasTargetBuilding);
	pBlackboard->AddData("TargetBuilding", m_TargetBuilding);
	pBlackboard->AddData("LastBuildingCounter", m_LastBuildingCounter);

	// Create behavior tree
	m_pBehaviorTree = new BehaviorTree(pBlackboard,
		new BehaviorSelector
		({
			new BehaviorSequence
			({
				new BehaviorConditional(TooMuchFood),
				new BehaviorAction(EatFood),
			}),
			new BehaviorSequence
			({
				new BehaviorConditional(TooManyGuns),
				new BehaviorAction(DropGun),
			}),
			new BehaviorSequence
			({
				new BehaviorConditional(TooManyMedkits),
				new BehaviorAction(UseMedkit),
			}),
			new BehaviorSequence
			({
				new BehaviorConditional(MedkitInInv),
				new BehaviorConditional(HealthLow),
				new BehaviorAction(UseMedkit)
			}),
			new BehaviorSequence
			({
				new BehaviorConditional(FoodInInv),
				new BehaviorConditional(EnergyLow),
				new BehaviorAction(EatFood)
			}),
			new BehaviorSequence
			({
				new BehaviorConditional(ZombieInFOV),
				new BehaviorSelector
				({
					new BehaviorSequence
					({
						new BehaviorConditional(GunInInv),
						new BehaviorAction(AimShoot)
					}),
					new BehaviorSequence
					({
					new BehaviorConditional(SuppliesNotLow),
					new BehaviorAction(Avoid)
					}),
				}),
			}),
			new BehaviorSequence
			({
				new BehaviorConditional(InvNotFull),
				new BehaviorConditional(ItemInFOV),
				new BehaviorAction(Pickup)
			}),
			new BehaviorSequence
			({
				new BehaviorConditional(SuppliesLow),
				new BehaviorSelector
				({
					new BehaviorSequence
					({
						new BehaviorConditional(HasTargetBuilding),
						new BehaviorAction(ScavengeBuilding)
					}),
					new BehaviorSequence
					({
						new BehaviorConditional(BuildingInFOV),
						new BehaviorConditional(NotRecentlyVisited),
						new BehaviorAction(SeekBuilding)
					}),
				}),
			}),
		}));
}

// Called only once
void Plugin::DllInit()
{
	// Called when the plugin is loaded
}

// Called only once
void Plugin::DllShutdown()
{
	// Called wheb the plugin gets unloaded
}

// Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be useful to inspect certain behaviours (Default = false)
							//params.LevelFile = "LevelTwo.gppl";
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	params.OverrideDifficulty = false;
	params.Difficulty = 2.f;
}

// Only Active in DEBUG Mode
// (=Use only for Debug Purposes)
void Plugin::Update(float dt)
{
	// Demo Event Code
	// In the end your AI should be able to walk around without external input
	if (m_pInterface->Input_IsMouseButtonUp(InputMouseButton::eLeft))
	{
		// Update target based on input
		MouseData mouseData = m_pInterface->Input_GetMouseData(InputType::eMouseButton, InputMouseButton::eLeft);
		const Vector2 pos = Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
		m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(eScancode_Space))
	{
		m_CanRun = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(eScancode_Left))
	{
		m_AngSpeed -= ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(eScancode_Right))
	{
		m_AngSpeed += ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(eScancode_G))
	{
		m_GrabItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(eScancode_U))
	{
		m_UseItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(eScancode_R))
	{
		m_RemoveItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyUp(eScancode_Space))
	{
		m_CanRun = false;
	}
}

// Update
// This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	auto steering = SteeringPlugin_Output();

	// Use the Interface (IAssignmentInterface) to 'interface' with the AI_Framework
	auto agentInfo = m_pInterface->Agent_GetInfo();

	// Retrieve the current location of our CheckPoint
	auto checkpointLocation = m_pInterface->World_GetCheckpointLocation();

	// Use the navmesh to calculate the next navmesh point
	auto nextTargetPos = m_pInterface->NavMesh_GetClosestPathPoint(checkpointLocation);

	// OR, Use the mouse target
	//auto nextTargetPos = m_Target; //Uncomment this to use mouse position as guidance

	auto vHousesInFOV = GetHousesInFOV();//uses m_pInterface->Fov_GetHouseByIndex(...)
	auto vEntitiesInFOV = GetEntitiesInFOV(); //uses m_pInterface->Fov_GetEntityByIndex(...)

	// I also always want the agent to scan the area by default
	agentInfo.AngularVelocity = 1.f;

	// Update behavior tree variables
	auto pBlackboard = m_pBehaviorTree->GetBlackboard();
	if (pBlackboard)
	{
		pBlackboard->ChangeData("Target", nextTargetPos);
		m_Target = nextTargetPos; // This is for steeringbehavior
		pBlackboard->ChangeData("Items", m_Items);
		pBlackboard->ChangeData("Agent", agentInfo);
		pBlackboard->ChangeData("Interface", m_pInterface);
		pBlackboard->ChangeData("EntitiesInFOV", GetEntitiesInFOV());
		pBlackboard->ChangeData("BuildingsInFOV", GetHousesInFOV());
	}

	// Recieve updated recovery state
	pBlackboard->GetData("Recovering", m_Recovering);

	// Because I want the agent to always sprint, it's not going to be part of a specific branch in the tree (unless indoors when avoiding zombies or recover when there's none nearby)
	if (!agentInfo.IsInHouse)
	{
		if (agentInfo.Stamina >= 9.9f && m_Recovering)
		{
			agentInfo.RunMode = true;
			m_Recovering = false;
		}
		else if (agentInfo.Stamina <= 0.1f && !m_Recovering)
		{
			agentInfo.RunMode = false;
			m_Recovering = true;
		}
	}
	else
	{
		agentInfo.RunMode = false;
		m_Recovering = true;		
	}

	pBlackboard->ChangeData("Recovering", m_Recovering);
	pBlackboard->ChangeData("Agent", agentInfo);

	// Another global setting I want is to look where you're going when indoors (no reason to look behind you when entering a building and potentially miss items)
	if (agentInfo.IsInHouse)
	{
		// Instead of default rotating look at pickup by first retrieving the orientation you look at
		float rotDegAgent = agentInfo.Orientation * (180.f / M_PI);
		while(rotDegAgent >= 360.f)
			rotDegAgent -= 360.f;
		while(rotDegAgent < 0.f)
			rotDegAgent += 360.f;

		// And then retrieve the orientation of the velocity
		float rotDegVel = GetOrientationFromVelocity(agentInfo.LinearVelocity) * (180.f / M_PI);
		while(rotDegVel >= 360.f)
			rotDegVel -= 360.f;
		while(rotDegVel < 0.f)
			rotDegVel += 360.f;

		// Compare both orientations (both being 0-360 degrees)
		float diffAngle = rotDegVel - rotDegAgent;
	
		if (diffAngle <= -180)
		    diffAngle += 360;
		else if (diffAngle >= 180)
		    diffAngle -= 360;

		agentInfo.AngularVelocity = diffAngle; // Even though max velocity is 1, it will still put it to a number outside that range which I'm okay with
		pBlackboard->ChangeData("Agent", agentInfo);
	}

	// Last but not least, stop getting stuck in that one spot where it can't decide what exit to take by checking if it's cycling between 2 targets a lot
	// Check whenever target point is changed (unless confirmed stuck)
	if (m_StuckTimer < 10)
	{
		if (m_Target != m_OldTarget)
		{
			m_OldTarget = m_Target;
			// Check if it re-adds the same position AKA switches to an older target which means the agent is stuck
			if (m_LastTargets.at(m_ChangeSecond) == m_OldTarget)
				++m_StuckTimer;
			else
				m_StuckTimer = 0;

			m_LastTargets.at(m_ChangeSecond) = m_OldTarget;
			m_ChangeSecond = !m_ChangeSecond;
		}
	}
	else // If confirmed stuck pick one of the targets and stick to it until it reached the chosen target
	{
		m_Target = m_pInterface->NavMesh_GetClosestPathPoint(m_LastTargets.at(0));
		pBlackboard->ChangeData("Target", m_Target);
		m_pInterface->Draw_Circle({agentInfo.Position}, 4.f, {1,0,0}, 0.f);
	}

	// Update tree
	m_pBehaviorTree->Update();

	// Return to normal behaviour after reaching the target it's stuck to or when the behaviour tree found a new target
	pBlackboard->GetData("Target", m_Target);
	const float distance = sqrt(pow(m_Target.x - agentInfo.Position.x, 2) + pow(m_Target.y - agentInfo.Position.y, 2));
	if (distance < 5.f)
		m_StuckTimer = 0;

	// INVENTORY USAGE DEMO
	//********************
	if (m_GrabItem)
	{
		ItemInfo item;
		// Item_Grab > When DebugParams.AutoGrabClosestItem is TRUE, the Item_Grab function returns the closest item in range
		// Keep in mind that DebugParams are only used for debugging purposes, by default this flag is FALSE
		// Otherwise, use GetEntitiesInFOV() to retrieve a vector of all entities in the FOV (EntityInfo)
		// Item_Grab gives you the ItemInfo back, based on the passed EntityHash (retrieved by GetEntitiesInFOV)
		if (m_pInterface->Item_Grab({}, item))
		{
			// Once grabbed, you can add it to a specific inventory slot
			// Slot must be empty
			m_pInterface->Inventory_AddItem(0, item);
		}
	}

	if (m_UseItem)
	{
		// Use an item (make sure there is an item at the given inventory slot)
		m_pInterface->Inventory_UseItem(0);
	}

	if (m_RemoveItem)
	{
		// Remove an item from a inventory slot
		m_pInterface->Inventory_RemoveItem(0);
	}

	// Simple Seek Behaviour (towards Target)
	pBlackboard->GetData("Agent", agentInfo);
	pBlackboard->GetData("Target", nextTargetPos);
	steering.LinearVelocity = nextTargetPos - agentInfo.Position; //Desired Velocity
	steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed

	steering.AngularVelocity = agentInfo.AngularVelocity; // Rotate your character to inspect the world while walking
	steering.AutoOrientate = false; // Setting AutoOrientate to TRue overrides the AngularVelocity

	steering.RunMode = agentInfo.RunMode; // If RunMode is True > MaxLinSpd is increased for a limited time (till your stamina runs out)

								 // SteeringPlugin_Output is works the exact same way a SteeringBehaviour output

								 // @End (Demo Purposes)
	m_GrabItem = false; // Reset State
	m_UseItem = false;
	m_RemoveItem = false;

	return steering;
}

// This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	// This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}