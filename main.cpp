#include "main.h"

bool isPluginInitialized = false;
int skinID = 0;

// changes skin without any checks
bool rpc_process(int skin_id) {
	BitStream bs;
	bs.Write<UINT32>(SAMP::pSAMP->getPlayers()->sLocalPlayerID);
	bs.Write<UINT32>(skin_id);
	skinID = skin_id; // static skin autoupdate
	return SAMP::pSAMP->getRakNet()->EmulRPC(RPC_ScrSetPlayerSkin, &bs); // ret 0/1
}

void changeskin(int params) {
	// find lowest Z coordinate
	CVector vecPlayerPos = FindPlayerCoors(-1);
	bool someBool;
	CEntity* someCEntity;
	float lowestGroundZCoordinate = CWorld::FindGroundZFor3DCoord(vecPlayerPos.x, vecPlayerPos.y, vecPlayerPos.z, &someBool, &someCEntity) + 1.1; // + 1.1 just to keep it safe
	
	// get boolean "is player in air"
	bool xz = vecPlayerPos.z > lowestGroundZCoordinate;
	if (!xz) { // if ped ISN'T in air
		if (rpc_process(params)) { // changing skin
			CPlayerPed* pLocalPlayer = FindPlayerPed();
			pLocalPlayer->m_nAnimGroup = 118; // cjrun/skaterun/etc fix
		}
	}
	else { // if player IS in air
		SAMP::pSAMP->addMessageToChat(-1, "[skinchanger FIXED 1.3] èñïîëüçîâàíèå êîìàíäû â âîçäóõå çàïðåùåíî.");
	}
}

bool __stdcall RakClientRPCRecvHook(SAMP::CallBacks::HookedStructs::stRakClientRPCRecv* params) {
	CPlayerPed* pLocalPlayer = FindPlayerPed();

	if (params->rpc_id == 12 &&
		skinID &&
		!pLocalPlayer->m_pVehicle) { // RPC_ScrSetPlayerPos
		
			CVector vecPlayerCoors = FindPlayerCoors(-1);
		/*
		    struct stSetPlayerPosData {
			float x;
			float y;
			float z;
			}; 
		*/
			stSetPlayerPosData setPlayerPosData;
			params->bitStream->Read(setPlayerPosData.x);
			params->bitStream->Read(setPlayerPosData.y);
			params->bitStream->Read(setPlayerPosData.z);
			// .Magnitude() would be fine too
			CVector vecNewPlayerPos = CVector(setPlayerPosData.x, setPlayerPosData.y, setPlayerPosData.z) - vecPlayerCoors;

			if (vecNewPlayerPos.x > 20.0 && // some important teleports may be interrupted, so..
				vecNewPlayerPos.y > 20.0 &&
				vecNewPlayerPos.z > 20.0) {
					rpc_process(skinID);
			}
	}
	return true;
}

void _cdecl cmd(char* params) {
	changeskin(atoi(params));
}

void __stdcall GameLoop() {
	static bool initialized = false;
	if (!initialized) {
		if (SAMP::pSAMP->LoadAPI()) {
			initialized = true;
			isPluginInitialized = true;
			SAMP::pSAMP->addMessageToChat(-1, "[skinchanger FIXED 1.3] loaded. cmd:  /sskin [id]  | vk/@sinhxxx");
			SAMP::pSAMP->addClientCommand("sskin", cmd);
		}
	}
	if (initialized) {	//gameLoop

	}
}

int __stdcall DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH: {
		SAMP::Init();
		SAMP::CallBacks::pCallBackRegister->RegisterGameLoopCallback(GameLoop);//register gameloop hook
		SAMP::CallBacks::pCallBackRegister->RegisterRakClientCallback(RakClientRPCRecvHook);
		break;
	}
	case DLL_PROCESS_DETACH: {
		SAMP::ShutDown();
		SAMP::pSAMP->unregisterChatCommand(cmd);
		break;
	}
	}
	return true;
}
