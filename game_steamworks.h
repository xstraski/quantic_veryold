#ifndef GAME_STEAMWORKS_H
#define GAME_STEAMWORKS_H

#include "game_platform.h"
#include "deps/steamworks/public/steam/steam_api.h"
#include "deps/steamworks/public/steam/isteamgameserver.h"
#include "deps/steamworks/public/steam/isteamgameserverstats.h"

// NOTE(ivan): Steamworks API prototypes.
#define STEAM_API_INIT(Name) bool S_CALLTYPE Name(void)
typedef STEAM_API_INIT(steam_api_init);

#define STEAM_API_SHUTDOWN(Name) void S_CALLTYPE Name(void)
typedef STEAM_API_SHUTDOWN(steam_api_shutdown);

#define STEAM_API_IS_STEAM_RUNNING(Name) bool S_CALLTYPE Name(void)
typedef STEAM_API_IS_STEAM_RUNNING(steam_api_is_steam_running);

#define STEAM_API_RESTART_APP_IF_NECESSARY(Name) bool S_CALLTYPE Name(uint32 OwnAppID)
typedef STEAM_API_RESTART_APP_IF_NECESSARY(steam_api_restart_app_if_necessary);

#define STEAM_API_GET_HSTEAM_USER(Name) HSteamUser S_CALLTYPE Name(void)
typedef STEAM_API_GET_HSTEAM_USER(steam_api_get_hsteam_user);

#define STEAM_GAME_SERVER_GET_HSTEAM_USER(Name) HSteamUser S_CALLTYPE Name(void)
typedef STEAM_GAME_SERVER_GET_HSTEAM_USER(steam_game_server_get_hsteam_user);

#define STEAM_INTERNAL_CONTEXT_INIT(Name) void * S_CALLTYPE Name(void *ContextInitData)
typedef STEAM_INTERNAL_CONTEXT_INIT(steam_internal_context_init);

#define STEAM_INTERNAL_CREATE_INTERFACE(Name) void * S_CALLTYPE Name(const char *Version)
typedef STEAM_INTERNAL_CREATE_INTERFACE(steam_internal_create_interface);

#define STEAM_INTERNAL_FIND_OR_CREATE_USER_INTERFACE(Name) void * S_CALLTYPE Name(HSteamUser User, const char *Version)
typedef STEAM_INTERNAL_FIND_OR_CREATE_USER_INTERFACE(steam_internal_find_or_create_user_interface);

#define STEAM_INTERNAL_FIND_OR_CREATE_GAME_SERVER_INTERFACE(Name)	\
	void * S_CALLTYPE Name(HSteamUser User, const char *Version)
typedef STEAM_INTERNAL_FIND_OR_CREATE_GAME_SERVER_INTERFACE(steam_internal_find_or_create_game_server_interface);

// NOTE(ivan): Steamworks module structure.
struct steamworks_api {
	// NOTE(ivan): Steam API.
	steam_api_init *Init;
	steam_api_shutdown *Shutdown;
	steam_api_is_steam_running *IsSteamRunning;
	steam_api_restart_app_if_necessary *RestartAppIfNecessary;
	steam_api_get_hsteam_user *GetHSteamUser;
	steam_game_server_get_hsteam_user *GameServerGetHSteamUser;

	// NOTE(ivan): Steam internal API.
	steam_internal_context_init *_ContextInit;
	steam_internal_create_interface *_CreateInterface;
	steam_internal_find_or_create_user_interface *_FindOrCreateUserInterface;
	steam_internal_find_or_create_game_server_interface *_FindOrCreateGameServerInterface;

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
	ISteamApps *GameServerApps;
	ISteamAppList *AppList;
	ISteamNetworking *Networking;
	ISteamRemoteStorage *RemoteStorage;
	ISteamScreenshots *Screenshots;
	ISteamHTTP *HTTP;
	ISteamHTTP *GameServerHTTP;
	ISteamController *Controller;
	ISteamUGC *UGC;
	ISteamUGC *GameServerUGC;
	ISteamMusic *Music;
	ISteamMusicRemote *MusicRemote;
	ISteamHTMLSurface *HTMLSurface;
	ISteamInventory *Inventory;
	ISteamVideo *Video;
	ISteamParentalSettings *ParentalSettings;
	ISteamInput *Input;
	ISteamParties *Parties;
};

// NOTE(ivan): Steamworks exported methods are loaded in platform layer.
// This function loads all Steamworks interfaces using GameState.SteamworksAPI->_FindOrCreate??? methods.
void GainAccessToSteamworksInterfaces(void);

#endif // #ifndef GAME_STEAMWORKS_H
