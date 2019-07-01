#include "game_steamworks.h"

// NOTE(ivan): Internal macros for accessing steam interfaces without static linking.
// These were stolen from steam_api_internal.h header file and then a bit modified.
#define DEFINE_STEAM_INTERFACE_ACCESSOR(Type, Name, Expression)			\
	inline void S_CALLTYPE Get##Name##InitRoutine(Type *Ptr) {*Ptr = (Type)(Expression);} \
	inline Type Get##Name(void) {										\
		static void *CallbackCounterAndContext[3] = {(void *)&Get##Name##InitRoutine, 0, 0}; \
		return *(Type *)GameState.SteamworksAPI->_ContextInit(CallbackCounterAndContext); \
	}
#define DEFINE_STEAM_USER_INTERFACE_ACCESSOR(Type, Name, Version)	\
	DEFINE_STEAM_INTERFACE_ACCESSOR(Type, Name, GameState.SteamworksAPI->_FindOrCreateUserInterface(GameState.SteamworksAPI->GetHSteamUser(), Version))
#define DEFINE_STEAM_GAME_SERVER_INTERFACE_ACCESSOR(Type, Name, Version) \
	DEFINE_STEAM_INTERFACE_ACCESSOR(Type, Name, GameState.SteamworksAPI->_FindOrCreateGameServerInterface(GameState.SteamworksAPI->GameServerGetHSteamUser(), Version))

// NOTE(ivan): Internal Steam interfaces accessors for GainAccessToSteamworksInterfaces() function.
DEFINE_STEAM_INTERFACE_ACCESSOR(ISteamClient *, ISteamClient, GameState.SteamworksAPI->_CreateInterface(STEAMCLIENT_INTERFACE_VERSION));
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamUser *, ISteamUser, STEAMUSER_INTERFACE_VERSION);
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamUserStats *, ISteamUserStats, STEAMUSERSTATS_INTERFACE_VERSION);
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamFriends *, ISteamFriends, STEAMFRIENDS_INTERFACE_VERSION);
DEFINE_STEAM_INTERFACE_ACCESSOR(ISteamUtils *, ISteamUtils, GameState.SteamworksAPI->_CreateInterface(STEAMUTILS_INTERFACE_VERSION));
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamMatchmaking *, ISteamMatchmaking, STEAMMATCHMAKING_INTERFACE_VERSION);
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamMatchmakingServers *, ISteamMatchmakingServers, \
									 STEAMMATCHMAKINGSERVERS_INTERFACE_VERSION);
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamGameSearch *, ISteamGameSearch, STEAMGAMESEARCH_INTERFACE_VERSION);
DEFINE_STEAM_GAME_SERVER_INTERFACE_ACCESSOR(ISteamGameServer *, ISteamGameServer, STEAMGAMESERVER_INTERFACE_VERSION);
DEFINE_STEAM_GAME_SERVER_INTERFACE_ACCESSOR(ISteamGameServerStats *, ISteamGameServerStats, \
											STEAMGAMESERVERSTATS_INTERFACE_VERSION);
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamApps *, ISteamApps, STEAMAPPS_INTERFACE_VERSION);
DEFINE_STEAM_GAME_SERVER_INTERFACE_ACCESSOR(ISteamApps *, GameServerISteamApps, STEAMAPPS_INTERFACE_VERSION);
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamAppList *, ISteamAppList, STEAMAPPLIST_INTERFACE_VERSION);
DEFINE_STEAM_GAME_SERVER_INTERFACE_ACCESSOR(ISteamNetworking *, ISteamNetworking, STEAMNETWORKING_INTERFACE_VERSION);
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamRemoteStorage *, ISteamRemoteStorage, STEAMREMOTESTORAGE_INTERFACE_VERSION);
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamScreenshots *, ISteamScreenshots, STEAMSCREENSHOTS_INTERFACE_VERSION);
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamHTTP *, ISteamHTTP, STEAMHTTP_INTERFACE_VERSION);
DEFINE_STEAM_GAME_SERVER_INTERFACE_ACCESSOR(ISteamHTTP *, GameServerISteamHTTP, STEAMHTTP_INTERFACE_VERSION);
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamController *, ISteamController, STEAMCONTROLLER_INTERFACE_VERSION);
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamUGC *, ISteamUGC, STEAMUGC_INTERFACE_VERSION);
DEFINE_STEAM_GAME_SERVER_INTERFACE_ACCESSOR(ISteamUGC *, GameServerISteamUGC, STEAMUGC_INTERFACE_VERSION);
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamMusic *, ISteamMusic, STEAMMUSIC_INTERFACE_VERSION);
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamMusicRemote *, ISteamMusicRemote, STEAMMUSICREMOTE_INTERFACE_VERSION);
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamHTMLSurface *, ISteamHTMLSurface, STEAMHTMLSURFACE_INTERFACE_VERSION);
DEFINE_STEAM_GAME_SERVER_INTERFACE_ACCESSOR(ISteamInventory *, ISteamInventory, STEAMINVENTORY_INTERFACE_VERSION);
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamVideo *, ISteamVideo, STEAMVIDEO_INTERFACE_VERSION);
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamParentalSettings *, ISteamParentalSettings,
									 STEAMPARENTALSETTINGS_INTERFACE_VERSION);
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamInput *, ISteamInput, STEAMINPUT_INTERFACE_VERSION);
DEFINE_STEAM_USER_INTERFACE_ACCESSOR(ISteamParties *, ISteamParties, STEAMPARTIES_INTERFACE_VERSION);

void
GainAccessToSteamworksInterfaces(void) {
	steamworks_api *SteamworksAPI = GameState.SteamworksAPI;
	
	Assert(SteamworksAPI);
	Assert(SteamworksAPI->GetHSteamUser);
	Assert(SteamworksAPI->GameServerGetHSteamUser);
	Assert(SteamworksAPI->_ContextInit);
	Assert(SteamworksAPI->_FindOrCreateUserInterface);
	Assert(SteamworksAPI->_FindOrCreateGameServerInterface);
	
	SteamworksAPI->Client = GetISteamClient();
	SteamworksAPI->User = GetISteamUser();
	SteamworksAPI->UserStats = GetISteamUserStats();
	SteamworksAPI->Friends = GetISteamFriends();
	SteamworksAPI->Utils = GetISteamUtils();
	SteamworksAPI->Matchmaking = GetISteamMatchmaking();
	SteamworksAPI->MatchmakingServers = GetISteamMatchmakingServers();
	SteamworksAPI->GameSearch = GetISteamGameSearch();
	SteamworksAPI->GameServer = GetISteamGameServer();
	SteamworksAPI->GameServerStats = GetISteamGameServerStats();
	SteamworksAPI->Apps = GetISteamApps();
	SteamworksAPI->GameServerApps = GetGameServerISteamApps();
	SteamworksAPI->AppList = GetISteamAppList();
	SteamworksAPI->Networking = GetISteamNetworking();
	SteamworksAPI->RemoteStorage = GetISteamRemoteStorage();
	SteamworksAPI->Screenshots = GetISteamScreenshots();
	SteamworksAPI->HTTP = GetISteamHTTP();
	SteamworksAPI->GameServerHTTP = GetGameServerISteamHTTP();
	SteamworksAPI->Controller = GetISteamController();
	SteamworksAPI->UGC = GetISteamUGC();
	SteamworksAPI->GameServerUGC = GetGameServerISteamUGC();
	SteamworksAPI->Music = GetISteamMusic();
	SteamworksAPI->MusicRemote = GetISteamMusicRemote();
	SteamworksAPI->HTMLSurface = GetISteamHTMLSurface();
	SteamworksAPI->Inventory = GetISteamInventory();
	SteamworksAPI->Video = GetISteamVideo();
	SteamworksAPI->ParentalSettings = GetISteamParentalSettings();
	SteamworksAPI->Input = GetISteamInput();
	SteamworksAPI->Parties = GetISteamParties();
}
