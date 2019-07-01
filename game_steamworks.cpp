#include "game_steamworks.h"

void
GainAccessToSteamworksInterfaces(steamworks_api *SteamworksAPI) {
	Assert(SteamworksAPI);
	Assert(SteamworksAPI->_CreateInterface);
	
	SteamworksAPI->Client = (ISteamClient *)SteamworksAPI->_CreateInterface(STEAMCLIENT_INTERFACE_VERSION);
	SteamworksAPI->User = (ISteamUser *)SteamworksAPI->_CreateInterface(STEAMUSER_INTERFACE_VERSION);
	SteamworksAPI->UserStats = (ISteamUserStats *)SteamworksAPI->_CreateInterface(STEAMUSERSTATS_INTERFACE_VERSION);
	SteamworksAPI->Friends = (ISteamFriends *)SteamworksAPI->_CreateInterface(STEAMFRIENDS_INTERFACE_VERSION);
	SteamworksAPI->Utils = (ISteamUtils *)SteamworksAPI->_CreateInterface(STEAMUTILS_INTERFACE_VERSION);
	SteamworksAPI->Matchmaking = (ISteamMatchmaking *)SteamworksAPI->_CreateInterface(STEAMMATCHMAKING_INTERFACE_VERSION);
	SteamworksAPI->MatchmakingServers
		= (ISteamMatchmakingServers *)SteamworksAPI->_CreateInterface(STEAMMATCHMAKINGSERVERS_INTERFACE_VERSION);
	SteamworksAPI->GameSearch = (ISteamGameSearch *)SteamworksAPI->_CreateInterface(STEAMGAMESEARCH_INTERFACE_VERSION);
	SteamworksAPI->GameServer = (ISteamGameServer *)SteamworksAPI->_CreateInterface(STEAMGAMESERVER_INTERFACE_VERSION);
	SteamworksAPI->GameServerStats
		= (ISteamGameServerStats *)SteamworksAPI->_CreateInterface(STEAMGAMESERVERSTATS_INTERFACE_VERSION);
	SteamworksAPI->Apps = (ISteamApps *)SteamworksAPI->_CreateInterface(STEAMAPPS_INTERFACE_VERSION);
	SteamworksAPI->AppList = (ISteamAppList *)SteamworksAPI->_CreateInterface(STEAMAPPLIST_INTERFACE_VERSION);
	SteamworksAPI->Networking = (ISteamNetworking *)SteamworksAPI->_CreateInterface(STEAMNETWORKING_INTERFACE_VERSION);
	SteamworksAPI->RemoteStorage
		= (ISteamRemoteStorage *)SteamworksAPI->_CreateInterface(STEAMREMOTESTORAGE_INTERFACE_VERSION);
	SteamworksAPI->Screenshots = (ISteamScreenshots *)SteamworksAPI->_CreateInterface(STEAMSCREENSHOTS_INTERFACE_VERSION);
	SteamworksAPI->HTTP = (ISteamHTTP *)SteamworksAPI->_CreateInterface(STEAMHTTP_INTERFACE_VERSION);
	SteamworksAPI->Controller = (ISteamController *)SteamworksAPI->_CreateInterface(STEAMCONTROLLER_INTERFACE_VERSION);
	SteamworksAPI->UGC = (ISteamUGC *)SteamworksAPI->_CreateInterface(STEAMUGC_INTERFACE_VERSION);
	SteamworksAPI->Music = (ISteamMusic *)SteamworksAPI->_CreateInterface(STEAMMUSIC_INTERFACE_VERSION);
	SteamworksAPI->MusicRemote = (ISteamMusicRemote *)SteamworksAPI->_CreateInterface(STEAMMUSICREMOTE_INTERFACE_VERSION);
	SteamworksAPI->HTMLSurface = (ISteamHTMLSurface *)SteamworksAPI->_CreateInterface(STEAMHTMLSURFACE_INTERFACE_VERSION);
	SteamworksAPI->Inventory = (ISteamInventory *)SteamworksAPI->_CreateInterface(STEAMINVENTORY_INTERFACE_VERSION);
	SteamworksAPI->Video = (ISteamVideo *)SteamworksAPI->_CreateInterface(STEAMVIDEO_INTERFACE_VERSION);
	SteamworksAPI->ParentalSettings
		= (ISteamParentalSettings *)SteamworksAPI->_CreateInterface(STEAMPARENTALSETTINGS_INTERFACE_VERSION);
	SteamworksAPI->Input = (ISteamInput *)SteamworksAPI->_CreateInterface(STEAMINPUT_INTERFACE_VERSION);
	SteamworksAPI->ContentServer
		= (ISteamContentServer *)SteamworksAPI->_CreateInterface(STEAMCONTENTSERVER_INTERFACE_VERSION);
	SteamworksAPI->PS3OverlayRender
		= (ISteamPS3OverlayRender *)SteamworksAPI->_CreateInterface(STEAMPS3OVERLAYRENDER_INTERFACE_VERSION);
	SteamworksAPI->Parties = (ISteamParties *)SteamworksAPI->_CreateInterface(STEAMPARTIES_INTERFACE_VERSION);
}
