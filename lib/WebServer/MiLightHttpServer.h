#include <RichHttpServer.h>
#include <MiLightClient.h>
#include <Settings.h>
#include <WebSocketsServer.h>
#include <GroupStateStore.h>
#include <RadioSwitchboard.h>
#include <PacketSender.h>
#include <TransitionController.h>

#ifndef _MILIGHT_HTTP_SERVER
#define _MILIGHT_HTTP_SERVER

#define MAX_DOWNLOAD_ATTEMPTS 3

typedef std::function<void(void)> SettingsSavedHandler;
typedef std::function<void(const BulbId& id)> GroupDeletedHandler;
typedef std::function<void(void)> THandlerFunction;
typedef std::function<void(JsonDocument& response)> AboutHandler;

using RichHttpConfig = RichHttp::Generics::Configs::EspressifBuiltin;
using RequestContext = RichHttpConfig::RequestContextType;

const char APPLICATION_OCTET_STREAM[] PROGMEM = "application/octet-stream";
const char TEXT_PLAIN[] PROGMEM = "text/plain";
const char APPLICATION_JSON[] = "application/json";
const std::vector<GroupStateField> NORMALIZED_GROUP_STATE_FIELDS = {
    GroupStateField::STATE,
    GroupStateField::COLOR_MODE,
    GroupStateField::LEVEL,
    GroupStateField::COLOR,
    GroupStateField::KELVIN,
};

static const uint8_t DEFAULT_PAGE_SIZE = 10;

class MiLightHttpServer {
public:
  MiLightHttpServer(
    Settings& settings,
    MiLightClient*& milightClient,
    GroupStateStore*& stateStore,
    PacketSender*& packetSender,
    RadioSwitchboard*& radios,
    TransitionController& transitions
  )
    : authProvider(settings)
    , server(80, authProvider)
    , wsServer(WebSocketsServer(81))
    , numWsClients(0)
    , milightClient(milightClient)
    , settings(settings)
    , stateStore(stateStore)
    , packetSender(packetSender)
    , radios(radios)
    , transitions(transitions)
  { }

  void begin();
  void handleClient();
  void onSettingsSaved(SettingsSavedHandler handler);
  void onGroupDeleted(GroupDeletedHandler handler);
  void onAbout(AboutHandler handler);
  void on(const char* path, HTTPMethod method, THandlerFunction handler);
  void handlePacketSent(uint8_t* packet, const MiLightRemoteConfig& config, const BulbId& bulbId, const JsonObject& result);
  WiFiClient client();

protected:

  bool serveFile(const char* file, const char* contentType = "text/html");
  void handleServe_P(const char* data, size_t length, const char* contentType);
  void sendGroupState(bool allowAsync, BulbId& bulbId, RichHttp::Response& response);

  void serveSettings();
  void handleUpdateSettings(RequestContext& request);
  void handleUpdateSettingsPost(RequestContext& request);
  void handleUpdateFile(const char* filename);

  void handleGetRadioConfigs(RequestContext& request);

  void handleAbout(RequestContext& request);
  void handleSystemPost(RequestContext& request);
  void handleFirmwareUpload();
  void handleFirmwarePost();
  void handleListenGateway(RequestContext& request);
  void handleSendRaw(RequestContext& request);

  void handleUpdateGroup(RequestContext& request);
  void handleUpdateGroupAlias(RequestContext& request);

  void handleListGroups();
  void handleGetGroup(RequestContext& request);
  void handleGetGroupAlias(RequestContext& request);
  void _handleGetGroup(bool allowAsync, BulbId bulbId, RequestContext& request);
  void handleBatchUpdateGroups(RequestContext& request);

  void handleDeleteGroup(RequestContext& request);
  void handleDeleteGroupAlias(RequestContext& request);
  void _handleDeleteGroup(BulbId bulbId, RequestContext& request);

  void handleGetTransition(RequestContext& request);
  void handleDeleteTransition(RequestContext& request);
  void handleCreateTransition(RequestContext& request);
  void handleListTransitions(RequestContext& request);

  // CRUD methods for /aliases
  void handleListAliases(RequestContext& request);
  void handleCreateAlias(RequestContext& request);
  void handleDeleteAlias(RequestContext& request);
  void handleUpdateAlias(RequestContext& request);
  void handleDeleteAliases(RequestContext& request);
  void handleUpdateAliases(RequestContext& request);

  void handleCreateBackup(RequestContext& request);
  void handleRestoreBackup(RequestContext& request);

  void handleRequest(const JsonObject& request);
  void handleWsEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

  void saveSettings();

  File updateFile;

  PassthroughAuthProvider<Settings> authProvider;
  RichHttpServer<RichHttp::Generics::Configs::EspressifBuiltin> server;
  WebSocketsServer wsServer;
  size_t numWsClients;
  MiLightClient*& milightClient;
  Settings& settings;
  GroupStateStore*& stateStore;
  SettingsSavedHandler settingsSavedHandler;
  GroupDeletedHandler groupDeletedHandler;
  THandlerFunction _handleRootPage;
  PacketSender*& packetSender;
  RadioSwitchboard*& radios;
  TransitionController& transitions;
  AboutHandler aboutHandler;


};

#endif
