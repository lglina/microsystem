#include "String.h"
#include "StringConstants.h"

namespace Agape
{
    // Builder
    const char* _Map( "Map" );
    const char* _Navigation( "Navigation" );
    const char* _Status( "Status" );
    const char* _Hotkeys( "Hotkeys" );
    const char* _Presence( "Presence" );
    const char* _LargeDialogue( "LargeDialogue" );
    const char* _SmallDialogue( "SmallDialogue" );
    const char* _Errors( "Errors" );
    const char* _small_dialogue( "small-dialogue" );
    const char* _small_dialogue_g( "small-dialogue-g" );
    const char* _error( "error" );
    const char* _splash( "splash" );
    const char* _connectWiFi( "connectWiFi" );
    const char* _connect( "connect" );
    const char* _onboarding( "onboarding" );
    const char* _menu( "menu" );
    const char* _modemConfig( "modemConfig" );
    const char* _worldbook( "worldbook" );
    const char* _chooseAvatar( "chooseAvatar" );
    const char* _createWorld( "createWorld" );
    const char* _joinWorld( "joinWorld" );
    const char* _enterWorld( "enterWorld" );
    const char* _walk( "walk" );
    const char* _edit( "edit" );
    const char* _linda2( "linda2" );
    const char* _enterKey( "enterKey" );
    const char* _tela( "tela" );
    const char* _minimap( "minimap" );
    const char* _wood( "wood" );
    const char* _credits( "credits" );
    const char* _ansiEditor( "ansiEditor" );
    const char* _memory( "memory" );
    const char* _test( "test" );

    // Actor/collection names
    const char* _Accounts( "Accounts" );
    const char* _AssetLoaderResponder( "AssetLoaderResponder" );
    const char* _AssetLoader( "AssetLoader" );
    const char* _Attributes( "Attributes" );
    const char* _Chat( "Chat" );
    const char* _Clock( "Clock" );
    const char* _InviteFriendClient( "InviteFriendClient" );
    const char* _InviteFriendServer( "InviteFriendServer" );
    const char* _Linda2( "Linda2" );
    const char* _PresenceLoader( "PresenceLoader" );
    const char* _PresenceLoaderResponder( "PresenceLoaderResponder" );
    const char* _PushNotifier( "PushNotifier" );
    const char* _SceneLoader( "SceneLoader" );
    const char* _SceneLoaderResponder( "SceneLoaderResponder" );
    const char* _TelegramLoader( "TelegramLoader" );
    const char* _TelegramLoaderResponder( "TelegramLoaderResponder" );
    const char* _Timer( "Timer" );
    const char* _World( "World" );
    const char* _WorldLoader( "WorldLoader" );
    const char* _WorldLoaderResponder( "WorldLoaderResponder" );
    const char* _my( "my" );
    const char* _this( "this" );

    // Tuple types
    const char* _Action( "Action" );
    const char* _Agape( "Agape" );
    const char* _AssetCloseRequest( "AssetCloseRequest" );
    const char* _AssetCloseResponse( "AssetCloseResponse" );
    const char* _AssetEraseRequest( "AssetEraseRequest" );
    const char* _AssetEraseResponse( "AssetEraseResponse" );
    const char* _AssetMoveRequest( "AssetMoveRequest" );
    const char* _AssetMoveResponse( "AssetMoveResponse" );
    const char* _AssetOpenRequest( "AssetOpenRequest" );
    const char* _AssetOpenResponse( "AssetOpenResponse" );
    const char* _AssetReadRequest( "AssetReadRequest" );
    const char* _AssetReadResponse( "AssetReadResponse" );
    const char* _AssetWriteRequest( "AssetWriteRequest" );
    const char* _AssetWriteResponse( "AssetWriteResponse" );
    const char* _Assets( "Assets" );
    const char* _Authenticate( "Authenticate" );
    const char* _Bump( "Bump" );
    const char* _ChangeThing( "ChangeThing" );
    const char* _ChatMessage( "ChatMessage" );
    const char* _Crash( "Crash" );
    const char* _CreatePerson( "CreatePerson" );
    const char* _CreateThing( "CreateThing" );
    const char* _DeletePerson( "DeletePerson" );
    const char* _DeleteThing( "DeleteThing" );
    const char* _DrawText( "DrawText" );
    const char* _InvalidateCachedAsset( "InvalidateCachedAsset" );
    const char* _InviteFriendRequest( "InviteFriendRequest" );
    const char* _InviteFriendResponse( "InviteFriendResponse" );
    const char* _Load( "Load" );
    const char* _Moved( "Moved" );
    const char* _MovePerson( "MovePerson" );
    const char* _MoveThing( "MoveThing" );
    const char* _PersonMoved( "PersonMoved" );
    const char* _PresenceLoadRequest( "PresenceLoadRequest" );
    const char* _PresenceLoadResponse( "PresenceLoadResponse" );
    const char* _PresenceLoadWorldRequest( "PresenceLoadWorldRequest" );
    const char* _PresenceLoadWorldResponse( "PresenceLoadWorldResponse" );
    const char* _PresenceLoadWorldSummary( "PresenceLoadWorldSummary" );
    const char* _PresenceRequest( "PresenceRequest" );
    const char* _PresenceResponse( "PresenceResponse" );
    const char* _PresenceSummary( "PresenceSummary" );
    const char* _RoutingCriteria( "RoutingCriteria" );
    const char* _SceneItemCreateAttributeRequest( "SceneItemCreateAttributeRequest" );
    const char* _SceneItemCreateAttributeResponse( "SceneItemCreateAttributeResponse" );
    const char* _SceneItemDeleteAttributesRequest( "SceneItemDeleteAttributesRequest" );
    const char* _SceneItemDeleteAttributesResponse( "SceneItemDeleteAttributesResponse" );
    const char* _SceneItemLoadAttributeRequest( "SceneItemLoadAttributeRequest" );
    const char* _SceneItemLoadAttributeResponse( "SceneItemLoadAttributeResponse" );
    const char* _SceneItemSaveAttributeRequest( "SceneItemSaveAttributeRequest" );
    const char* _SceneItemSaveAttributeResponse( "SceneItemSaveAttributeResponse" );
    const char* _SceneLoadRequest( "SceneLoadRequest" );
    const char* _SceneLoadResponse( "SceneLoadResponse" );
    const char* _SceneRequest( "SceneRequest" );
    const char* _SceneResponse( "SceneResponse" );
    const char* _Scenes( "Scenes" );
    const char* _SceneSummary( "SceneSummary" );
    const char* _TelegramEraseRequest( "TelegramEraseRequest" );
    const char* _TelegramEraseResponse( "TelegramEraseResponse" );
    const char* _TelegramLoadRequest( "TelegramLoadRequest" );
    const char* _TelegramLoadResponse( "TelegramLoadResponse" );
    const char* _TelegramLoadSentRequest( "TelegramLoadSentRequest" );
    const char* _TelegramLoadSentResponse( "TelegramLoadSentResponse" );
    const char* _TelegramLoadSentSummary( "TelegramLoadSentSummary" );
    const char* _TelegramLoadSummary( "TelegramLoadSummary" );
    const char* _TelegramMarkReadRequest( "TelegramMarkReadRequest" );
    const char* _TelegramMarkReadResponse( "TelegramMarkReadResponse" );
    const char* _TelegramSendRequest( "TelegramSendRequest" );
    const char* _TelegramSendResponse( "TelegramSendResponse" );
    const char* _TelegramUnreadRequest( "TelegramUnreadRequest" );
    const char* _TelegramUnreadResponse( "TelegramUnreadResponse" );
    const char* _TeleportPerson( "TeleportPerson" );
    const char* _ThingChanged( "ThingChanged" );
    const char* _Tick( "Tick" );
    const char* _Time( "Time" );
    const char* _TransportThing( "TransportThing" );
    const char* _Unload( "Unload" );
    const char* _WorldCreateRequest( "WorldCreateRequest" );
    const char* _WorldCreateResponse( "WorldCreateResponse" );
    const char* _WorldCreateTeleportRequest( "WorldCreateTeleportRequest" );
    const char* _WorldCreateTeleportResponse( "WorldCreateTeleportResponse" );
    const char* _WorldDeleteTeleportRequest( "WorldDeleteTeleportRequest" );
    const char* _WorldDeleteTeleportResponse( "WorldDeleteTeleportResponse" );
    const char* _WorldJoinRequest( "WorldJoinRequest" );
    const char* _WorldJoinResponse( "WorldJoinResponse" );
    const char* _WorldLoadJoinedRequest( "WorldLoadJoinedRequest" );
    const char* _WorldLoadJoinedResponse( "WorldLoadJoinedResponse" );
    const char* _WorldLoadJoinedSummary( "WorldLoadJoinedSummary" );
    const char* _WorldLoadTeleportsRequest( "WorldLoadTeleportsRequest" );
    const char* _WorldLoadTeleportsResponse( "WorldLoadTeleportsResponse" );
    const char* _WorldLoadTeleportsSummary( "WorldLoadTeleportsSummary" );
    const char* _WorldLoadUniverseStatsRequest( "WorldLoadUniverseStatsRequest" );
    const char* _WorldLoadUniverseStatsResponse( "WorldLoadUniverseStatsResponse" );
    const char* _WorldLoadRequest( "WorldLoadRequest" );
    const char* _WorldLoadResponse( "WorldLoadResponse" );
    const char* _WorldLoadWorldSummariesRequest( "WorldLoadWorldSummariesRequest" );
    const char* _WorldLoadWorldSummariesResponse( "WorldLoadWorldSummariesResponse" );
    const char* _Worlds( "Worlds" );

    // Tuple value keys
    const char* _action( "action" );
    const char* _add( "add" );
    const char* _allDevices( "allDevices" );
    const char* _assetName( "assetName" );
    const char* _assetType( "assetType" );
    const char* _attribute( "attrib" );
    const char* _attributes( "attribs" ); // Mongoid barfs on "attributes", so best to avoid it everywhere!
    const char* _authKeyHash( "authKeyHash" );
    const char* _author( "author" );
    const char* _bit( "bit" );
    const char* _bottom( "bottom" );
    const char* _collectionName( "collectionName" );
    const char* _colour( "colour" );
    const char* _column( "column" );
    const char* _completed( "completed" );
    const char* _computerid( "computerid" );
    const char* _coordinates( "coordinates" );
    const char* _data( "data" );
    const char* _dateTime( "dateTime" );
    const char* _default( "default" );
    const char* _destinationActor( "destinationActor" );
    const char* _destinationActors( "destinationActors" );
    const char* _destinationID( "destinationID" );
    const char* _destinationIDs( "destinationIDs" );
    const char* _devices( "devices" );
    const char* _direction( "direction" );
    const char* _doInvite( "doInvite" );
    const char* _doTeleport( "doTeleport" );
    const char* _edge( "edge" );
    const char* _findThing( "findThing" );
    const char* _findThings( "findThings" );
    const char* _flags( "flags" );
    const char* _friendsEmail( "friendsEmail" );
    const char* _friendsName( "friendsName" );
    const char* _from( "from" );
    const char* _getHeight( "getHeight" );
    const char* _glyph( "glyph" );
    const char* _hash( "hash" );
    const char* _height( "height" );
    const char* _id( "id" );
    const char* _insert( "insert" );
    const char* _inviteFriend( "inviteFriend" );
    const char* _itemAt( "itemAt" );        
    const char* _itemKey( "itemKey" );
    const char* _items( "items" );
    const char* _joinedWorlds( "joinedWorlds" );
    const char* _keyboard( "keyboard" );
    const char* _keyType( "keyType" );
    const char* _lastSeen( "lastSeen" );
    const char* _length( "length" );
    const char* _linkedItem( "linkedItem" );
    const char* _message( "message" );
    const char* _metadata( "metadata" );
    const char* _mode( "mode" );
    const char* _modificationSnowflake( "modificationSnowflake" );
    const char* _name( "name" );
    const char* _nearbyPeople( "nearbyPeople" );
    const char* _nearbyThings( "nearbyThings" );
    const char* _newCoordinates( "newCoordinates" );
    const char* _newName( "newName" );
    const char* _next( "next" );
    const char* _nonInteractive( "nonInteractive" );
    const char* _now( "now" );
    const char* _number( "number" );
    const char* _numUnread( "numUnread" );
    const char* _offset( "offset" );
    const char* _once( "once" );
    const char* _oobe( "oobe" );
    const char* _openMode( "openMode" );
    const char* _originatorID( "originatorID" );
    const char* _overlap( "overlap" );
    const char* _owner( "owner" );
    const char* _presenceOperation( "presenceOperation" );
    const char* _present( "present" );
    const char* _privateKey( "privateKey" );
    const char* _push( "push" );
    const char* _read( "read" );
    const char* _recipientSnowflake( "recipientSnowflake" );
    const char* _reenter( "reenter" );
    const char* _row( "row" );
    const char* _sceneItem( "sceneItem" );
    const char* _sceneItems( "sceneItems" );
    const char* _sceneOperation( "sceneOperation" );
    const char* _scenePresence( "scenePresence" );
    const char* _sealedWorldKey( "sealedWorldKey" );
    const char* _sealingKey( "sealingKey" );
    const char* _senderSnowflake( "senderSnowflake" );
    const char* _setHeight( "setHeight" );
    const char* _size( "size" );
    const char* _snowflake( "snowflake" );
    const char* _sourceActor( "sourceActor" );
    const char* _sourceID( "sourceID" );
    const char* _state( "state" );
    const char* _subject( "subject" );
    const char* _success( "success" );
    const char* _telegram( "telegram" );
    const char* _telegramSnowflake( "telegramSnowflake" );
    const char* _teleport( "teleport" );
    const char* _teleportDemo( "teleportDemo" );
    const char* _teleports( "teleports" );
    const char* _text( "text" );
    const char* _top( "top" );
    const char* _totalItems( "totalItems" );
    const char* _type( "type" );
    const char* _types( "types" );
    const char* _universeStats( "universeStats" );
    const char* _unread( "unread" );
    const char* _update( "update" );
    const char* _user( "user" );
    const char* _userid( "userid" );
    const char* _userRead( "userRead" );
    const char* _users( "users" );
    const char* _userWrite( "userWrite" );
    const char* _value( "value" );
    const char* _values( "values" );
    const char* _width( "width" );
    const char* _world( "world" );
    const char* _worldAuthKey( "worldAuthKey" );
    const char* _worldID( "worldID" );
    const char* _worldKey( "worldKey" );
    const char* _worldSummaries( "worldSummaries" );
    const char* _writable( "writable" );
    const char* _write( "write" );
    const char* _x( "x" );
    const char* _y( "y" );

    // UI and internal strings
    const char* _defaultPhonebookEntry( "defaultPhonebookEntry" );
    const char* _phonebook( "phonebook" );

    const char* _worlds( "worlds" );
    const char* _defaultWorldID( "defaultWorldID" );

    const char* _Avatar1( "Avatar1" );
    const char* _Avatar2( "Avatar2" );
    const char* _Avatar3( "Avatar3" );
    const char* _Avatar4( "Avatar4" );

    const char* _checker( "checker" );

    const char* _My_WiFi_Networks( "My WiFi Networks" );
    const char* _Network( "Network" );
    const char* _Password( "Password" );

    const char* _Access_Point_Names( "Access Point Names" );
    const char* _Access_Point_Scan( "Access Point Scan" );
    const char* _Add_Access_Point_Name( "Add Access Point Name" );
    const char* _Add_Access_Point_Password( "Add Access Point Password" );
    const char* _Delete_Access_Point_Name( "Delete Access Point Name" );

    const char* _Enter_World( "Enter World" );
    const char* _Select_World( "Select World" );
    const char* _Select_Server( "Select Server" );
    const char* _Settings( "Settings" );
    const char* _Credits( "Credits" );

    const char* _Name( "Name" );
    const char* _Number( "Number" );
    const char* _Add_Entry( "Add Entry" );

    const char* _WiFi_Networks( "WiFi Networks" );
    const char* _Account_Key( "Account Key" );
    const char* _Device_Key( "Device Key" );

    const char* _Add_World( "Add World" );

    const char* _title( "title" );
    const char* _welcome( "welcome" );
    const char* _settings( "settings" );

    const char* _up( "up" );
    const char* _down( "down" );
    const char* _left( "left" );
    const char* _right( "right" );
    const char* _none( "none" );

    const char* _remove( "remove" );

    const char* _carlo( "carlo" );

    const char* _nothing( "nothing" );

    const char* _accountSubKey( "accountSubKey" );
    const char* _accountAuthKey( "accountAuthKey" );
    const char* _deviceAuthKey( "deviceAuthKey" );
    const char* _telaDeviceAuthKeyHash( "telaDeviceAuthKeyHash" );

    const char* _authenticate( "authenticate" );

    const char* _blit( "blit" );
    const char* _sprite( "sprite" );
    const char* _animate( "animate" );

    const char* _Telegrams( "Telegrams" );
    const char* _Telegrams_Inbox( "Telegrams - Inbox" );
    const char* _Telegrams_Sent( "Telegrams - Sent" );
    const char* _Recipient( "Recipient" );
    const char* _Subject( "Subject" );

    const char* _Teleports( "Teleports" );
    const char* _People( "People" );
    const char* _X( "X (enter E/W)" );
    const char* _Y( "Y (enter N/S)" );
    const char* _Row( "Row (optional)" );
    const char* _Col( "Col (optional)" );
    const char* _Add_Teleport( "Add Teleport" );
    const char* _Description( "Description" );
    const char* _home( "home" );
    const char* _Go_To( "Go To" );

    const char* _Welcome( "Welcome!" );
    const char* _Invite_A_Friend( "Invite a Friend?" );

    const char* _Owner( "Owner" );
    const char* _Insert_Item( "Insert Item" );
    const char* _Update_Item( "Update Item" );

    const char* _Open_Template( "Open Template" );

    const char* _Ready( "Ready" );
    const char* _Carrier( "Carrier" );
    const char* _Secure( "Secure" );

    const char* _Power( "Power" );
    const char* _Memory( "Memory" );

    const char* _command( "command" );

    const char* _Set_Template( "Set Template" );
    const char* _Save_As( "Save As" );

    const char* _Open_Or_Create( "Open Or Create" );

    const char* _asset( "asset" );
    const char* _program( "program" );

    const char* _servermessage( "servermessage" );

    const char* _boss( "boss" );

    const char* _ground( "ground" );
    const char* _unknown( "unknown" );

    const char* _scene_( "scene_" );

    const char* _loop( "loop" );

    // Baked-in world IDs and keys
    const char* _sharedAssetsWorldID( "N0cI//dxndWXnsh11WzSKG9tPPfsMXo7JWMqqyjsN7s=" );
    const unsigned char _sharedAssetsItemKey[16] = { 0x5C, 0xDC, 0x9C, 0x7E, 0x16, 0x91, 0xE5, 0x65,
                                           0x70, 0x85, 0x55, 0x15, 0xDF, 0x9B, 0xA4, 0x1C };
    
    const char* _defaultPhoneName( "Stratus" );
    const char* _defaultPhoneNumber( "wss://glina.com.au:8443/" );
    //const char* _defaultPhoneNumber( "wss://127.0.0.1:8443/" );

    const char* _learningWorldID( _sharedAssetsWorldID );
    const unsigned char _learningWorldKey[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
} // namespace Agape
