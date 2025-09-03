#ifndef AGAPE_STRING_CONSTANTS_H
#define AGAPE_STRING_CONSTANTS_H

#include "String.h"

namespace Agape
{
    // Builder
    extern const char* _Map;
    extern const char* _Navigation;
    extern const char* _Status;
    extern const char* _Hotkeys;
    extern const char* _Presence;
    extern const char* _LargeDialogue;
    extern const char* _SmallDialogue;
    extern const char* _Errors;
    extern const char* _small_dialogue;
    extern const char* _small_dialogue_g;
    extern const char* _error;
    extern const char* _splash;
    extern const char* _connectWiFi;
    extern const char* _connect;
    extern const char* _onboarding;
    extern const char* _menu;
    extern const char* _modemConfig;
    extern const char* _worldbook;
    extern const char* _chooseAvatar;
    extern const char* _createWorld;
    extern const char* _joinWorld;
    extern const char* _enterWorld;
    extern const char* _walk;
    extern const char* _edit;
    extern const char* _linda2;
    extern const char* _enterKey;
    extern const char* _tela;
    extern const char* _minimap;
    extern const char* _wood;
    extern const char* _credits;
    extern const char* _ansiEditor;
    extern const char* _memory;
    extern const char* _test;

    // Actor/collection names
    extern const char* _Accounts;
    extern const char* _AssetLoader;
    extern const char* _AssetLoaderResponder;
    extern const char* _Attributes;
    extern const char* _Chat;
    extern const char* _Clock;
    extern const char* _InviteFriendClient;
    extern const char* _InviteFriendServer;
    extern const char* _Linda2;
    extern const char* _PresenceLoader;
    extern const char* _PresenceLoaderResponder;
    extern const char* _PushNotifier;
    extern const char* _SceneLoader;
    extern const char* _SceneLoaderResponder;
    extern const char* _TelegramLoader;
    extern const char* _TelegramLoaderResponder;
    extern const char* _Timer;
    extern const char* _World;
    extern const char* _WorldLoader;
    extern const char* _WorldLoaderResponder;
    extern const char* _my;
    extern const char* _this;

    // Tuple types
    extern const char* _Action;
    extern const char* _Agape;
    extern const char* _AssetCloseRequest;
    extern const char* _AssetCloseResponse;
    extern const char* _AssetEraseRequest;
    extern const char* _AssetEraseResponse;
    extern const char* _AssetMoveRequest;
    extern const char* _AssetMoveResponse;
    extern const char* _AssetOpenRequest;
    extern const char* _AssetOpenResponse;
    extern const char* _AssetReadRequest;
    extern const char* _AssetReadResponse;
    extern const char* _AssetWriteRequest;
    extern const char* _AssetWriteResponse;
    extern const char* _Assets;
    extern const char* _Authenticate;
    extern const char* _Bump;
    extern const char* _ChangeThing;
    extern const char* _ChatMessage;
    extern const char* _Crash;
    extern const char* _CreatePerson;
    extern const char* _CreateThing;
    extern const char* _DeletePerson;
    extern const char* _DeleteThing;
    extern const char* _DrawText;
    extern const char* _InvalidateCachedAsset;
    extern const char* _InviteFriendRequest;
    extern const char* _InviteFriendResponse;
    extern const char* _Load;
    extern const char* _Moved;
    extern const char* _MovePerson;
    extern const char* _MoveThing;
    extern const char* _PersonMoved;
    extern const char* _PresenceLoadRequest;
    extern const char* _PresenceLoadResponse;
    extern const char* _PresenceLoadWorldRequest;
    extern const char* _PresenceLoadWorldResponse;
    extern const char* _PresenceLoadWorldSummary;
    extern const char* _PresenceRequest;
    extern const char* _PresenceResponse;
    extern const char* _PresenceSummary;
    extern const char* _RoutingCriteria;
    extern const char* _SceneItemCreateAttributeRequest;
    extern const char* _SceneItemCreateAttributeResponse;
    extern const char* _SceneItemDeleteAttributesRequest;
    extern const char* _SceneItemDeleteAttributesResponse;
    extern const char* _SceneItemLoadAttributeRequest;
    extern const char* _SceneItemLoadAttributeResponse;
    extern const char* _SceneItemSaveAttributeRequest;
    extern const char* _SceneItemSaveAttributeResponse;
    extern const char* _SceneLoadRequest;
    extern const char* _SceneLoadResponse;
    extern const char* _SceneRequest;
    extern const char* _SceneResponse;
    extern const char* _Scenes;
    extern const char* _SceneSummary;
    extern const char* _TelegramEraseRequest;
    extern const char* _TelegramEraseResponse;
    extern const char* _TelegramLoadRequest;
    extern const char* _TelegramLoadResponse;
    extern const char* _TelegramLoadSentRequest;
    extern const char* _TelegramLoadSentResponse;
    extern const char* _TelegramLoadSentSummary;
    extern const char* _TelegramLoadSummary;
    extern const char* _TelegramMarkReadRequest;
    extern const char* _TelegramMarkReadResponse;
    extern const char* _TelegramSendRequest;
    extern const char* _TelegramSendResponse;
    extern const char* _TelegramUnreadRequest;
    extern const char* _TelegramUnreadResponse;
    extern const char* _TeleportPerson;
    extern const char* _ThingChanged;
    extern const char* _Tick;
    extern const char* _Time;
    extern const char* _TransportThing;
    extern const char* _Unload;
    extern const char* _WorldCreateRequest;
    extern const char* _WorldCreateResponse;
    extern const char* _WorldCreateTeleportRequest;
    extern const char* _WorldCreateTeleportResponse;
    extern const char* _WorldDeleteTeleportRequest;
    extern const char* _WorldDeleteTeleportResponse;
    extern const char* _WorldJoinRequest;
    extern const char* _WorldJoinResponse;
    extern const char* _WorldLoadJoinedRequest;
    extern const char* _WorldLoadJoinedResponse;
    extern const char* _WorldLoadJoinedSummary;
    extern const char* _WorldLoadTeleportsRequest;
    extern const char* _WorldLoadTeleportsResponse;
    extern const char* _WorldLoadTeleportsSummary;
    extern const char* _WorldLoadUniverseStatsRequest;
    extern const char* _WorldLoadUniverseStatsResponse;
    extern const char* _WorldLoadRequest;
    extern const char* _WorldLoadResponse;
    extern const char* _WorldLoadWorldSummariesRequest;
    extern const char* _WorldLoadWorldSummariesResponse;
    extern const char* _Worlds;

    // Tuple value keys
    extern const char* _action;
    extern const char* _add;
    extern const char* _allDevices;
    extern const char* _assetName;
    extern const char* _assetType;
    extern const char* _attribute;
    extern const char* _attributes;
    extern const char* _authKeyHash;
    extern const char* _author;
    extern const char* _bit;
    extern const char* _bottom;
    extern const char* _collectionName;
    extern const char* _colour;
    extern const char* _column;
    extern const char* _completed;
    extern const char* _computerid;
    extern const char* _coordinates;
    extern const char* _data;
    extern const char* _dateTime;
    extern const char* _default;
    extern const char* _destinationActor;
    extern const char* _destinationActors;
    extern const char* _destinationID;
    extern const char* _destinationIDs;
    extern const char* _devices;
    extern const char* _direction;
    extern const char* _doInvite;
    extern const char* _doTeleport;
    extern const char* _edge;
    extern const char* _findThing;
    extern const char* _findThings;
    extern const char* _flags;
    extern const char* _friendsEmail;
    extern const char* _friendsName;
    extern const char* _from;
    extern const char* _getHeight;
    extern const char* _glyph;
    extern const char* _hash;
    extern const char* _height;
    extern const char* _id;
    extern const char* _insert;
    extern const char* _inviteFriend;
    extern const char* _itemAt;
    extern const char* _itemKey;
    extern const char* _items;
    extern const char* _joinedWorlds;
    extern const char* _keyboard;
    extern const char* _keyType;
    extern const char* _lastSeen;
    extern const char* _length;
    extern const char* _linkedItem;
    extern const char* _message;
    extern const char* _metadata;
    extern const char* _mode;
    extern const char* _modificationSnowflake;
    extern const char* _name;
    extern const char* _nearbyPeople;
    extern const char* _nearbyThings;
    extern const char* _newCoordinates;
    extern const char* _newName;
    extern const char* _next;
    extern const char* _nonInteractive;
    extern const char* _now;
    extern const char* _number;
    extern const char* _numUnread;
    extern const char* _offset;
    extern const char* _once;
    extern const char* _oobe;
    extern const char* _openMode;
    extern const char* _originatorID;
    extern const char* _overlap;
    extern const char* _owner;
    extern const char* _presenceOperation;
    extern const char* _present;
    extern const char* _privateKey;
    extern const char* _push;
    extern const char* _read;
    extern const char* _recipientSnowflake;
    extern const char* _reenter;
    extern const char* _row;
    extern const char* _sceneItem;
    extern const char* _sceneItems;
    extern const char* _sceneOperation;
    extern const char* _scenePresence;
    extern const char* _sealedWorldKey;
    extern const char* _sealingKey;
    extern const char* _senderSnowflake;
    extern const char* _setHeight;
    extern const char* _size;
    extern const char* _snowflake;
    extern const char* _sourceActor;
    extern const char* _sourceID;
    extern const char* _state;
    extern const char* _subject;
    extern const char* _success;
    extern const char* _telegram;
    extern const char* _telegramSnowflake;
    extern const char* _teleport;
    extern const char* _teleportDemo;
    extern const char* _teleports;
    extern const char* _text;
    extern const char* _top;
    extern const char* _totalItems;
    extern const char* _type;
    extern const char* _types;
    extern const char* _universeStats;
    extern const char* _unread;
    extern const char* _update;
    extern const char* _user;
    extern const char* _userid;
    extern const char* _userRead;
    extern const char* _users;
    extern const char* _userWrite;
    extern const char* _value;
    extern const char* _values;
    extern const char* _width;
    extern const char* _world;
    extern const char* _worldAuthKey;
    extern const char* _worldID;
    extern const char* _worldKey;
    extern const char* _worldSummaries;
    extern const char* _writable;
    extern const char* _write;
    extern const char* _x;
    extern const char* _y;

    // UI elements & internal strings
    extern const char* _defaultPhonebookEntry;
    extern const char* _phonebook;

    extern const char* _worlds;
    extern const char* _defaultWorldID;

    extern const char* _Avatar1;
    extern const char* _Avatar2;
    extern const char* _Avatar3;
    extern const char* _Avatar4;

    extern const char* _checker;

    extern const char* _My_WiFi_Networks;
    extern const char* _Network;
    extern const char* _Password;

    extern const char* _Access_Point_Names;
    extern const char* _Access_Point_Scan;
    extern const char* _Add_Access_Point_Name;
    extern const char* _Add_Access_Point_Password;
    extern const char* _Delete_Access_Point_Name;

    extern const char* _Enter_World;
    extern const char* _Select_World;
    extern const char* _Select_Server;
    extern const char* _Settings;
    extern const char* _Credits;

    extern const char* _Name;
    extern const char* _Number;
    extern const char* _Add_Entry;

    extern const char* _WiFi_Networks;
    extern const char* _Account_Key;
    extern const char* _Device_Key;

    extern const char* _Add_World;

    extern const char* _title;
    extern const char* _welcome;
    extern const char* _settings;

    extern const char* _up;
    extern const char* _down;
    extern const char* _left;
    extern const char* _right;
    extern const char* _none;
    
    extern const char* _remove;

    extern const char* _carlo;

    extern const char* _nothing;

    extern const char* _accountSubKey;
    extern const char* _accountAuthKey;
    extern const char* _deviceAuthKey;
    extern const char* _telaDeviceAuthKeyHash;

    extern const char* _authenticate;

    extern const char* _blit;
    extern const char* _sprite;
    extern const char* _animate;

    extern const char* _Telegrams;
    extern const char* _Telegrams_Inbox;
    extern const char* _Telegrams_Sent;
    extern const char* _Recipient;
    extern const char* _Subject;

    extern const char* _Teleports;
    extern const char* _People;
    extern const char* _X;
    extern const char* _Y;
    extern const char* _Row;
    extern const char* _Col;
    extern const char* _Add_Teleport;
    extern const char* _Description;
    extern const char* _home;
    extern const char* _Go_To;

    extern const char* _Welcome;
    extern const char* _Invite_A_Friend;

    extern const char* _Owner;
    extern const char* _Insert_Item;
    extern const char* _Update_Item;

    extern const char* _Open_Template;

    extern const char* _Ready;
    extern const char* _Carrier;
    extern const char* _Secure;

    extern const char* _Power;
    extern const char* _Memory;

    extern const char* _command;

    extern const char* _Set_Template;
    extern const char* _Save_As;

    extern const char* _Open_Or_Create;

    extern const char* _asset;
    extern const char* _program;

    extern const char* _servermessage;

    extern const char* _boss;

    extern const char* _ground;
    extern const char* _unknown;

    extern const char* _scene_;

    extern const char* _loop;

    // Baked-in world IDs and keys
    extern const char* _sharedAssetsWorldID;
    extern const unsigned char _sharedAssetsItemKey[16];

    extern const char* _defaultPhoneName;
    extern const char* _defaultPhoneNumber;
    
    extern const char* _learningWorldID;
    extern const unsigned char _learningWorldKey[16];
} // namespace Agape

#endif // AGAPE_STRING_CONSTANTS_H
