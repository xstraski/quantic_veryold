#ifndef GAME_STEAMWORKS_H
#define GAME_STEAMWORKS_H

#include "game_platform.h"
#include "deps/steamworks/public/steam/steam_api.h"

// NOTE(ivan): Steamworks API prototypes.
#define STEAM_API_INIT(Name) bool S_CALLTYPE Name(void)
typedef STEAM_API_INIT(steam_api_init);

#define STEAM_API_SHUTDOWN(Name) void S_CALLTYPE Name(void)
typedef STEAM_API_SHUTDOWN(steam_api_shutdown);

#define STEAM_API_IS_STEAM_RUNNING(Name) bool S_CALLTYPE Name(void)
typedef STEAM_API_IS_STEAM_RUNNING(steam_api_is_steam_running);

#define STEAM_API_RESTART_APP_IF_NECESSARY(Name) bool S_CALLTYPE Name(uint32 OwnAppID)
typedef STEAM_API_RESTART_APP_IF_NECESSARY(steam_api_restart_app_if_necessary);

#define STEAM_INTERNAL_CONTEXT_INIT(Name) void * S_CALLTYPE Name(void *ContextInitData)
typedef STEAM_INTERNAL_CONTEXT_INIT(steam_internal_context_init);

#define STEAM_INTERNAL_CREATE_INTERFACE(Name) void * S_CALLTYPE Name(const char *Version)
typedef STEAM_INTERNAL_CREATE_INTERFACE(steam_internal_create_interface);

// NOTE(ivan): Steamworks module structure.
struct steamworks_api {
	// NOTE(ivan): Steam API.
	steam_api_init *Init;
	steam_api_shutdown *Shutdown;
	steam_api_is_steam_running *IsSteamRunning;
	steam_api_restart_app_if_necessary *RestartAppIfNecessary;

	// NOTE(ivan): Steam internal API.
	steam_internal_context_init *ContextInit;
	steam_internal_create_interface *CreateInterface;

	// NOTE(ivan): Steam API interfaces.
	ISteamClient *Client;
	ISteamUser *User;
	ISteamUserStats *UserStats;
	ISteamFriends *Friends;
	ISteamUtils *Utils;
	ISteamMatchmaking *Matchmaking;
	ISteamMatchmakingServers *MatchmakingServers;
	ISteamGameSearch *GameSearch;
	ISteamGameServer *GameServer;
	ISteamGameServerStats *GameServerStats;
	ISteamApps *Apps;
	ISteamAppList *AppList;
	ISteamNetworking *Networking;
	ISteamRemoteStorage *RemoteStorage;
	ISteamScreenshots *Screenshots;
	ISteamHTTP *HTTP;
	ISteamController *Controller;
	ISteamUGC *UGC;
	ISteamMusic *Music;
	ISteamMusicRemote *MusicRemote;
	ISteamHTMLSurface *HTMLSurface;
	ISteamInventory *Inventory;
	ISteamVideo *Video;
	ISteamParentalSettings *ParentalSettings;
	ISteamInput *Input;
	ISteamContentServer *ContentServer;
	ISteamPS3OverlayRender *PS3OverlayRender;
	ISteamParties *Parties;
};

// NOTE(ivan): Steamworks exported methods are loaded in platform layer.
// This function loads all Steamworks interfaces using SteamworksAPI->CreateInterface() method.
void GainAccessToSteamworksInterfaces(steamworks_api *SteamworksAPI);

#endif // #ifndef GAME_STEAMWORKS_H
