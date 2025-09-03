TEMPLATE = app
TARGET = TerminalSimulator
INCLUDEPATH += ../Agape
INCLUDEPATH += ../ANSIEditor
INCLUDEPATH += ../Carlo
INCLUDEPATH += ../Editor
INCLUDEPATH += ../KiamaFS
INCLUDEPATH += ../Linda2
QMAKE_CXXFLAGS += -DLOG_TUPLES_BRIEF -DNO_PIXEL_ART -Wno-unused-parameter
CONFIG += debug

## When cross-compiling for Windows using MXE, to enable debug output
## on console.
#mxe
#{
#    CONFIG += console
#}

SOURCES += ../Agape/AssetLoaders/Caches/KiamaFSAssetCache.cpp \
           ../Agape/AssetLoaders/Factories/BakedAssetLoaderFactory.cpp \
           ../Agape/AssetLoaders/Factories/CacheAssetLoaderFactory.cpp \
           ../Agape/AssetLoaders/Factories/EncryptedAssetLoaderFactory.cpp \
           ../Agape/AssetLoaders/Factories/FileAssetLoaderFactory.cpp \
           ../Agape/AssetLoaders/Factories/Linda2AssetLoaderFactory.cpp \
           ../Agape/AssetLoaders/Factories/MiniMapAssetLoaderFactory.cpp \
           ../Agape/AssetLoaders/Factories/RAMAssetLoaderFactory.cpp \
           ../Agape/AssetLoaders/AssetLoader.cpp \
           ../Agape/AssetLoaders/CacheAssetLoader.cpp \
           ../Agape/AssetLoaders/BakedAssetLoader.cpp \
           ../Agape/AssetLoaders/EncryptedAssetLoader.cpp \
           ../Agape/AssetLoaders/FileAssetLoader.cpp \
           ../Agape/AssetLoaders/Linda2AssetLoader.cpp \
           ../Agape/AssetLoaders/MiniMapAssetLoader.cpp \
           ../Agape/AssetLoaders/RAMAssetLoader.cpp \
           ../Agape/Assets/*.cpp \
           ../Agape/Audio/MIDIPlayers/NullMIDIPlayer.cpp \
           ../Agape/Audio/MIDIPlayer.cpp \
           ../Agape/Clocks/Clock.cpp \
           ../Agape/Clocks/VRTimeClock.cpp \
           ../Agape/Encryptors/AES/*.cpp \
           ../Agape/Encryptors/AES/tiny-AES-c/aes.c \
           ../Agape/Encryptors/Factories/*.cpp \
           ../Agape/Encryptors/SHA256/*.c* \
           ../Agape/Encryptors/Utils/BIP39/*.c* \
           ../Agape/Encryptors/Utils/SecureIdentifier.cpp \
           ../Agape/Encryptors/*.cpp \
           ../Agape/EntropySources/CRand.cpp \
           ../Agape/EventClocks/*.cpp \
           ../Agape/GraphicsDrivers/GraphicsDriver.cpp \
           ../Agape/GraphicsDrivers/Headless.cpp \
           ../Agape/GraphicsDrivers/QtWindGraphicsDriver.cpp \
           ../Agape/InputDevices/QtWindInputDevice.cpp \
           ../Agape/LineDrivers/NullLineDriver.cpp \
           ../Agape/LineDrivers/QtWebSocketsLineDriver.cpp \
           ../Agape/Lines/DummyModemLine.cpp \
           ../Agape/Lines/Line.cpp \
           ../Agape/Lines/ModemLine.cpp \
           ../Agape/Loggers/Logger.cpp \
           ../Agape/Loggers/StreamLogger.cpp \
           ../Agape/Memories/FileMemory.cpp \
           ../Agape/Memories/KiamaFSMemory.cpp \
           ../Agape/Memories/Memory.cpp \
           ../Agape/NativeActors/*.cpp \
           ../Agape/Network/QtWebSocketsConnection.cpp \
           ../Agape/Platforms/Platform.cpp \
           ../Agape/Platforms/SimulatedPlatform.cpp \
           ../Agape/PresenceLoaders/Factories/EncryptedPresenceLoaderFactory.cpp \
           ../Agape/PresenceLoaders/Factories/Linda2PresenceLoaderFactory.cpp \
           ../Agape/PresenceLoaders/Factories/OfflinePresenceLoaderFactory.cpp \
           ../Agape/PresenceLoaders/EncryptedPresenceLoader.cpp \
           ../Agape/PresenceLoaders/Linda2PresenceLoader.cpp \
           ../Agape/PresenceLoaders/Linda2PresenceLoaderResponder.cpp \
           ../Agape/PresenceLoaders/OfflinePresenceLoader.cpp \
           ../Agape/PresenceLoaders/PresenceLoader.cpp \
           ../Agape/PresenceLoaders/PresenceRequest.cpp \
           ../Agape/SceneLoaders/Factories/EncryptedSceneLoaderFactory.cpp \
           ../Agape/SceneLoaders/Factories/FileSceneLoaderFactory.cpp \
           ../Agape/SceneLoaders/Factories/Linda2SceneLoaderFactory.cpp \
           ../Agape/SceneLoaders/EncryptedSceneLoader.cpp \
           ../Agape/SceneLoaders/FileSceneLoader.cpp \
           ../Agape/SceneLoaders/Linda2SceneLoader.cpp \
           ../Agape/SceneLoaders/Linda2SceneLoaderResponder.cpp \
           ../Agape/SceneLoaders/SceneLoader.cpp \
           ../Agape/SceneLoaders/SceneRequest.cpp \
           ../Agape/TelegramLoaders/Factories/FileTelegramLoaderFactory.cpp \
           ../Agape/TelegramLoaders/Factories/Linda2TelegramLoaderFactory.cpp \
           ../Agape/TelegramLoaders/FileTelegramLoader.cpp \
           ../Agape/TelegramLoaders/Linda2TelegramLoader.cpp \
           ../Agape/TelegramLoaders/TelegramLoader.cpp \
           ../Agape/Timers/Factories/*.cpp \
           ../Agape/Timers/*.cpp \
           ../Agape/UI/*.cpp \
           ../Agape/UI/Forms/*.cpp \
           ../Agape/UI/Forms/Utils/*.cpp \
           ../Agape/UI/Strategies/*.cpp \
           ../Agape/Utils/*.cpp \
           ../Agape/Utils/base64/*.cpp \
           ../Agape/ValueLoaders/*.cpp \
           ../Agape/World/*.cpp \
           ../Agape/WorldLoaders/Factories/FileWorldLoaderFactory.cpp \
           ../Agape/WorldLoaders/Factories/Linda2WorldLoaderFactory.cpp \
           ../Agape/WorldLoaders/FileWorldLoader.cpp \
           ../Agape/WorldLoaders/Linda2WorldLoader.cpp \
           ../Agape/Allocator.cpp \
           ../Agape/ANSITerminal.cpp \
           ../Agape/ANSIWriter.cpp \
           ../Agape/Chat.cpp \
           ../Agape/Chooser.cpp \
           ../Agape/Client.cpp \
           ../Agape/ClientBuilder.cpp \
           ../Agape/ConfigurationStore.cpp \
           ../Agape/ConnectionMonitor.cpp \
           ../Agape/FileWriter.cpp \
           ../Agape/KeyUtilities.cpp \
           ../Agape/Phonebook.cpp \
           ../Agape/RandomReadableWritable.cpp \
           ../Agape/ReadableWritable.cpp \
           ../Agape/RWBuffer.cpp \
           ../Agape/Session.cpp \
           ../Agape/String.cpp \
           ../Agape/StringConstants.cpp \
           ../Agape/Terminal.cpp \
           ../Agape/Value.cpp \
           ../Agape/Warp.cpp \
           ../Agape/WindowManager.cpp \
           ../Agape/Worldbook.cpp \
           ../ANSIEditor/ANSIEditor.cpp \
           ../ANSIEditor/ANSIEditorFactory.cpp \
           ../Carlo/Expressions/*.cpp \
           ../Carlo/Statements/*.cpp \
           ../Carlo/Block.cpp \
           ../Carlo/ExecutionContext.cpp \
           ../Carlo/FunctionDispatcher.cpp \
           ../Carlo/InbuiltFunctions.cpp \
           ../Carlo/Lexer.cpp \
           ../Carlo/Linda2.cpp \
           ../Carlo/Parser.cpp \
           ../Carlo/ProgramManager.cpp \
           ../Carlo/SyntaxTreeNode.cpp \
           ../Editor/Highlighters/Factories/*.cpp \
           ../Editor/Highlighters/*.cpp \
           ../Editor/*.cpp \
           ../KiamaFS/KiamaFS.cpp \
           ../Linda2/Actors/NativeActors/*.cpp \
           ../Linda2/Actors/*.cpp \
           ../Linda2/TupleRoutes/NullTupleRoute.cpp \
           ../Linda2/TupleRoutes/ReadableWritableTupleRoute.cpp \
           ../Linda2/TupleRoutes/TupleRoute.cpp \
           ../Linda2/Promise.cpp \
           ../Linda2/Tuple.cpp \
           ../Linda2/TupleDispatcher.cpp \
           ../Linda2/TupleHandler.cpp \
           ../Linda2/TupleRouter.cpp \
           ../Linda2/TupleRoutingCriteria.cpp \
           SimulatedOfflineClientBuilder.cpp \
           SimulatedOnlineClientBuilder.cpp \
           TerminalSimulator.cpp

HEADERS += ../Agape/AssetLoaders/*.h \
           ../Agape/AssetLoaders/Caches/*.h \
           ../Agape/AssetLoaders/Factories/*.h \
           ../Agape/Assets/*.h \
           ../Agape/Audio/*.h \
           ../Agape/Audio/MIDIPlayers/*.h \
           ../Agape/Clocks/*.h \
           ../Agape/Encryptors/*.h \
           ../Agape/Encryptors/AES/*.h \
           ../Agape/Encryptors/AES/tiny-AES-c/aes.h* \
           ../Agape/Encryptors/Factories/*.h \
           ../Agape/Encryptors/SHA256/sha256.hpp \
           ../Agape/Encryptors/SHA256/SHA256Hash.h \
           ../Agape/Encryptors/Utils/BIP39/*.h \
           ../Agape/Encryptors/Utils/*.h \
           ../Agape/EntropySources/*.h \
           ../Agape/EventClocks/*.h \
           ../Agape/GraphicsDrivers/*.h \
           ../Agape/InputDevices/*.h \
           ../Agape/LineDrivers/*.h \
           ../Agape/Lines/*.h \
           ../Agape/Loggers/*.h \
           ../Agape/Memories/*.h \
           ../Agape/NativeActors/*.h \
           ../Agape/Network/*.h \
           ../Agape/Platforms/*.h \
           ../Agape/PresenceLoaders/*.h \
           ../Agape/PresenceLoaders/Factories/*.h \
           ../Agape/SceneLoaders/*.h \
           ../Agape/SceneLoaders/Factories/*.h \
           ../Agape/TelegramLoaders/*.h \
           ../Agape/TelegramLoaders/Factories/*.h \
           ../Agape/Timers/Factories/*.h \
           ../Agape/Timers/*.h \
           ../Agape/UI/*.h \
           ../Agape/UI/Forms/*.h \
           ../Agape/UI/Forms/Utils/*.h \
           ../Agape/UI/Strategies/*.h \
           ../Agape/Utils/*.h \
           ../Agape/Utils/base64/*.h* \
           ../Agape/ValueLoaders/*.h \
           ../Agape/World/*.h \
           ../Agape/WorldLoaders/Factories/*.h \
           ../Agape/WorldLoaders/*.h \
           ../Agape/*.h \
           ../ANSIEditor/*.h \
           ../Carlo/Expressions/*.h \
           ../Carlo/Statements/*.h \
           ../Carlo/*.h \
           ../Editor/Highlighters/Factories/*.h \
           ../Editor/Highlighters/*.h \
           ../Editor/*.h \
           ../KiamaFS/*.h \
           ../Linda2/Actors/NativeActors/*.h \
           ../Linda2/Actors/*.h \
           ../Linda2/TupleRoutes/*.h \
           ../Linda2/*.h \
           SimulatedOfflineClientBuilder.h \
           SimulatedOnlineClientBuilder.h \
           TerminalSimulator.h

OBJECTS_DIR = build
MOC_DIR = build

QT += widgets websockets

## Only build ALSA MIDI player when not cross-compiling for Windows.
!mxe {
    SOURCES += ../Agape/Audio/MIDIPlayers/ALSAMIDIPlayer.cpp \
               ../Agape/Audio/FrobMIDIPlayer.cpp
    LIBS += -lasound
}
