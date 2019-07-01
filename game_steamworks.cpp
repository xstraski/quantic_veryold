#include "game_steamworks.h"

void
GainAccessToSteamworksInterfaces(steamworks_api *SteamworksAPI) {
	Assert(SteamworksAPI);
	Assert(SteamworksAPI->CreateInterface);
	
	SteamworksAPI->Client = (ISteamClient *)SteamworksAPI->CreateInterface(STEAMCLIENT_INTERFACE_VERSION);
	SteamworksAPI->User = (ISteamUser *)SteamworksAPI->CreateInterface(STEAMUSER_INTERFACE_VERSION);
	SteamworksAPI->UserStats = (ISteamUserStats *)SteamworksAPI->CreateInterface(STEAMUSERSTATS_INTERFACE_VERSION);
	SteamworksAPI->Friends = (ISteamFriends *)SteamworksAPI->CreateInterface(STEAMFRIENDS_INTERFACE_VERSION);
	SteamworksAPI->Utils = (ISteamUtils *)SteamworksAPI->CreateInterface(STEAMUTILS_INTERFACE_VERSION);
	SteamworksAPI->Matchmaking = (ISteamMatchmaking *)SteamworksAPI->CreateInterface(STEAMMATCHMAKING_INTERFACE_VERSION);
	SteamworksAPI->MatchmakingServers
		= (ISteamMatchmakingServers *)SteamworksAPI->CreateInterface(STEAMMATCHMAKINGNSERVERS_INTERFACE_VERSION);
	SteamworksAPI->GameSearch = (ISteamGameSearch *)SteamworksAPI->CreateInterface(STEAMGAMESEARCH_INTERFACE_VERSION);
	SteamworksAPI->GameServer = (ISteamGameServer *)SteamworksAPI->CreateInterface(STEAMGAMESERVER_INTERFACE_VERSION);
	SteamworksAPI->GameServerStats
		= (ISteamGameServerStats *)SteamworksAPI->CreateInterface(STEAMGAMESERVERSTATS_INTERFACE_VERSION);
	SteamworksAPI->Apps = (ISteamApps *)SteamworksAPI->CreateInterface(STEAMAPPS_INTERFACE_VERSION);
	SteamworksAPI->AppList = (ISteamAppList *)SteamworksAPI->CreateInterface(STEAMAPPLIST_INTERFACE_VERSION);
	SteamworksAPI->Networking = (ISteamNetworking *)SteamworksAPI->CreateInterface(STEAMNETWORKING_INTERFACE_VERSION);
	SteamworksAPI->RemoteStorage
		= (ISteamRemoteStorage *)SteamworksAPI->CreateInterface(STEAMREMOTESTORAGE_INTERFACE_VERSION);
	SteamworksAPI->Screenshots = (ISteamScreenshots *)SteamworksAPI->CreateInterface(STEAMSCREENSHOTS_INTERFACE_VERSION);
	SteamworksAPI->HTTP = (ISteamHTTP *)SteamworksAPI->CreateInterface(STEAMHTTP_INTERFACE_VERSION);
	SteamworksAPI->Controller = (ISteamController *)SteamworksAPI->CreateInterface(STEAMCONTROLLER_INTERFACE_VERSION);
	SteamworksAPI->UGC = (ISteamUGC *)SteamworksAPI->CreateInterface(STEAMUGC_INTERFACE_VERSION);
	SteamworksAPI->Music = (ISteamMusic *)SteamworksAPI->CreateInterface(STEAMMUSIC_INTERFACE_VERSION);
	SteamworksAPI->MusicRemote = (ISteamMusicRemote *)SteamworksAPI->CreateInterface(STEAMMUSICREMOTE_INTERFACE_VERSION);
	SteamworksAPI->HTMLSurface = (ISteamHTMLSurface *)SteamworksAPI->CreateInterface(STEAMHTMLSURFACE_INTERFACE_VERSION);
	SteamworksAPI->Inventory = (ISteamInventory *)SteamworksAPI->CreateInterface(STEAMINVENTORY_INTERFACE_VERSION);
	SteamworksAPI->Video = (ISteamVideo *)SteamworksAPI->CreateInterface(STEAMVIDEO_INTERFACE_VERSION);
	SteamworksAPI->ParentalSettings
		= (ISteamParentalSettings *)SteamworksAPI->CreateInterface(STEAMPARENTALSETTINGS_INTERFACE_VERSION);
	SteamworksAPI->Input = (ISteamInput *)SteamworksAPI->CreateInterface(STEAMINPUT_INTERFACE_VERSION);
	SteamworksAPI->ContentServer
		= (ISteamContentServer *)SteamworksAPI->CreateInterface(STEAMCONTENTSERVER_INTERFACE_VERSION);
	SteamworksAPI->PS3OverlayRender
		= (ISteamPS3OverlayRender *)SteamworksAPI->CreateInterface(STEAMPS3OVERLAYRENDER_INTERFACE_VERSION);
	SteamworksAPI->Parties = (ISteamParties *)SteamworksAPI->CreateInterface(STEAMPARTIES_INTERFACE_VERSION);
}
