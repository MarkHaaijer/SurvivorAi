#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"
#include "EliteAI/EliteBehaviorTree/EBehaviorTree.h"

class IBaseInterface;
class IExamInterface;

class Plugin :public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void Update(float dt) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;
	vector<HouseInfo> GetHousesInFOV() const;
	vector<EntityInfo> GetEntitiesInFOV() const;

	Elite::BehaviorTree* m_pBehaviorTree = nullptr;
	Elite::Vector2 m_Target = {};
	Elite::Vector2 m_OldTarget = {};
	Elite::Vector2 m_TargetBuilding = {}; // Used to commit to a building once in scavenge mode
	vector<Elite::Vector2> m_LastTargets{{0,0}, {0,0}}; // Keeps track of the last 2 different targets
	vector<Elite::Vector2> m_LastBuildings{{0,0}, {0,0}, {0,0}, {0,0}}; // Keeps track of the last 4 different houses to prevent revisiting as it's likely empty
	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	bool m_Recovering = true;
	bool m_ChangeSecond = false; // Changes whether it changes information of first or second item in the last targets list 
	bool m_HasTargetBuilding = false; // Allows the agent to commit to a building when it sees another unvisited one and doesn't try cycle through both

	int m_StuckTimer = 0; // Will tell you how long you've been cycling between 2 targets
	int m_LastBuildingCounter = 0; // Will tell you which one of the 3 buildings need to be updated
	float m_AngSpeed = 0.f; //Demo purpose
	vector<int> m_Items[1]; // Keeps track of items
	/*
	PISTOL = 0
	MEDKIT = 1
	FOOD = 2
	GARBAGE = 3
	 */
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}