AGAPE_DIR=$(abspath ../Agape)
ANSI_EDITOR_DIR=$(abspath ../ANSIEditor)
CARLO_DIR=$(abspath ../Carlo)
EDITOR_DIR=$(abspath ../Editor)
ESTELLE_DIR=$(abspath Estelle)
KIAMA_DIR=$(abspath ../kiamafs)
LINDA2_DIR=$(abspath ../Linda2)
VPATH=$(AGAPE_DIR) $(ANSI_EDITOR_DIR) $(CARLO_DIR) $(EDITOR_DIR) $(KIAMA_DIR) $(LINDA2_DIR)
XC32BIN=/opt/microchip/xc32/v4.45/bin
DEFINES=-DLOG_TUPLES_BRIEF
#DEFINES=-DKIAMA_DEBUG -DLOG_LOADERS

CXX=$(XC32BIN)/xc32-g++
CXXFLAGS=-mprocessor=32MZ2048EFG064 -mmalloc-variant=binned -g $(DEFINES) -I. -I$(AGAPE_DIR) -I$(ANSI_EDITOR_DIR) -I$(CARLO_DIR) -I$(EDITOR_DIR) -I$(ESTELLE_DIR) -I$(KIAMA_DIR) -I$(LINDA2_DIR) -Wl,--defsym=_min_heap_size=262144,--defsym=_DEBUGGER=1,--report-mem -ffunction-sections -fdata-sections -fno-common -Wl,--gc-sections,--print-gc-sections -save-temps -O0 -Wl,-Map,mapfile.map -fno-exceptions -fno-rtti
#CXXFLAGS=-mprocessor=32MZ2048EFG064 -mmalloc-variant=binned -g $(DEFINES) -I. -I$(AGAPE_DIR) -I$(ANSI_EDITOR_DIR) -I$(CARLO_DIR) -I$(EDITOR_DIR) -I$(ESTELLE_DIR) -I$(KIAMA_DIR) -I$(LINDA2_DIR) -Wl,--defsym=_min_heap_size=262144,--defsym=_DEBUGGER=1,--report-mem -ffunction-sections -fdata-sections -fno-common -flto -Wl,--gc-sections,--print-gc-sections -save-temps -Os -Wl,-Map,mapfile.map -fno-exceptions -fno-rtti
#CXXFLAGS=-mprocessor=32MX470F512H -mmalloc-variant=binned -mips16 -g $(DEFINES) -I. -I$(AGAPE_DIR) -I$(ANSI_EDITOR_DIR) -I$(CARLO_DIR) -I$(EDITOR_DIR) -I$(ESTELLE_DIR) -I$(KIAMA_DIR) -I$(LINDA2_DIR) -Wl,--defsym=_min_heap_size=110000,--defsym=_DEBUGGER=1,--report-mem -ffunction-sections -fdata-sections -fno-common -flto -Wl,--gc-sections,--print-gc-sections -save-temps -Os -Wl,-Map,mapfile.map -fno-exceptions -fno-rtti
#CXXFLAGS=-mprocessor=32MX470F512H -mips16 -g -I. -I$(AGAPE_DIR) -I$(CARLO_DIR) -I$(EDITOR_DIR) -I$(KIAMA_DIR) -I$(LINDA2_DIR) -Wl,--defsym=_min_heap_size=100000,--defsym=_DEBUGGER=1,--report-mem -fno-common -save-temps -Os -Wl,-Map,mapfile.map -fno-exceptions
#CXXFLAGS=-mprocessor=32MX170F256B -mips16 -I. -I$(AGAPE_DIR) -I$(KIAMA_DIR) -I$(LINDA2_DIR) -Wl,--defsym=_min_heap_size=32768,--report-mem -ffunction-sections -fdata-sections -Wl,--gc-sections,--print-gc-sections -save-temps -Os -Wl,-Map,mapfile.map -fno-exceptions

CC=$(XC32BIN)/xc32-gcc
CFLAGS=-mprocessor=32MZ2048EFG064 -mmalloc-variant=binned -g -std=c99 $(DEFINES) -I. -I$(AGAPE_DIR) -I$(CARLO_DIR) -I$(EDITOR_DIR) -I$(KIAMA_DIR) -I$(LINDA2_DIR) -Wl,--defsym=_min_heap_size=262144,--defsym=_DEBUGGER=1,--report-mem -ffunction-sections -fdata-sections -fno-common -Wl,--gc-sections,--print-gc-sections -save-temps -O0 -Wl,-Map,mapfile.map -fno-exceptions
#CFLAGS=-mprocessor=32MZ2048EFG064 -mmalloc-variant=binned -g -std=c99 $(DEFINES) -I. -I$(AGAPE_DIR) -I$(CARLO_DIR) -I$(EDITOR_DIR) -I$(KIAMA_DIR) -I$(LINDA2_DIR) -Wl,--defsym=_min_heap_size=262144,--defsym=_DEBUGGER=1,--report-mem -ffunction-sections -fdata-sections -fno-common -flto -Wl,--gc-sections,--print-gc-sections -save-temps -Os -Wl,-Map,mapfile.map -fno-exceptions
#CFLAGS=-mprocessor=32MX470F512H -mmalloc-variant=binned -mips16 -g -std=c99 $(DEFINES) -I. -I$(AGAPE_DIR) -I$(CARLO_DIR) -I$(EDITOR_DIR) -I$(KIAMA_DIR) -I$(LINDA2_DIR) -Wl,--defsym=_min_heap_size=110000,--defsym=_DEBUGGER=1,--report-mem -ffunction-sections -fdata-sections -fno-common -flto -Wl,--gc-sections,--print-gc-sections -save-temps -Os -Wl,-Map,mapfile.map -fno-exceptions
#CFLAGS=-mprocessor=32MX470F512H -mips16 -g -std=c99 -I. -I$(AGAPE_DIR) -I$(CARLO_DIR) -I$(EDITOR_DIR) -I$(KIAMA_DIR) -I$(LINDA2_DIR) -Wl,--defsym=_min_heap_size=100000,--defsym=_DEBUGGER=1,--report-mem -fno-common -save-temps -Os -Wl,-Map,mapfile.map -fno-exceptions
#CFLAGS=-mprocessor=32MX170F256B -mips16 -std=c99 -I. -I$(AGAPE_DIR) -I$(KIAMA_DIR) -I$(LINDA2_DIR) -Wl,--defsym=_min_heap_size=32768,--report-mem -ffunction-sections -fdata-sections -Wl,--gc-sections,--print-gc-sections -save-temps -Os -Wl,-Map,mapfile.map -fno-exceptions

BIN2HEX=$(XC32BIN)/xc32-bin2hex

#AssetLoaders/Factories/KiamaFSAssetLoaderFactory.cpp \
#AssetLoaders/KiamaFSAssetLoader.cpp \
#InputDevices/NullInputDevice.cpp \

#PresenceLoaders/Factories/DemoPresenceLoaderFactory.cpp \
#PresenceLoaders/DemoPresenceLoader.cpp \

#ProgramLoaders/Factories/NullProgramLoaderFactory.cpp \
#ProgramLoaders/NullProgramLoader.cpp \
#SceneLoaders/Factories/KiamaFSSceneLoaderFactory.cpp \
#SceneLoaders/KiamaFSSceneLoader.cpp \


#WorldLoaders/Factories/KiamaFSWorldLoaderFactory.cpp \
#WorldLoaders/KiamaFSWorldLoader.cpp \

SOURCES=Actors/Linda2Actor.cpp \
        Actors/NativeActors/NativeActor.cpp \
		AssetLoaders/Caches/KiamaFSAssetCache.cpp \
		AssetLoaders/Caches/RAMAssetCache.cpp \
        AssetLoaders/Factories/BakedAssetLoaderFactory.cpp \
		AssetLoaders/Factories/CacheAssetLoaderFactory.cpp \
		AssetLoaders/Factories/DemoAssetLoaderFactory.cpp \
		AssetLoaders/Factories/EncryptedAssetLoaderFactory.cpp \
		AssetLoaders/Factories/Linda2AssetLoaderFactory.cpp \
		AssetLoaders/Factories/MiniMapAssetLoaderFactory.cpp \
		AssetLoaders/Factories/RAMAssetLoaderFactory.cpp \
		AssetLoaders/AssetLoader.cpp \
		AssetLoaders/BakedAssetLoader.cpp \
		AssetLoaders/CacheAssetLoader.cpp \
		AssetLoaders/DemoAssetLoader.cpp \
		AssetLoaders/EncryptedAssetLoader.cpp \
		AssetLoaders/Linda2AssetLoader.cpp \
		AssetLoaders/MiniMapAssetLoader.cpp \
		AssetLoaders/RAMAssetLoader.cpp \
		Assets/ANSIFile.cpp \
		Assets/Asset.cpp \
		Assets/SAUCE.cpp \
		Audio/MIDIPlayers/NullMIDIPlayer.cpp \
		Audio/MIDIPlayers/SAM2695MIDIPlayer.cpp \
		Audio/FrobMIDIPlayer.cpp \
		Audio/MIDIPlayer.cpp \
		Clocks/Clock.cpp \
		Clocks/DummyClock.cpp \
		Clocks/VRTimeClock.cpp \
		Encryptors/Factories/AESBlockEncryptorFactory.cpp \
		Encryptors/Factories/AESEncryptorFactory.cpp \
		Encryptors/AES/AESBlockEncryptor.cpp \
		Encryptors/AES/AESEncryptor.cpp \
		Encryptors/BlockEncryptor.cpp \
		Encryptors/Encryptor.cpp \
		Encryptors/SHA256/SHA256Hash.cpp \
		Encryptors/Utils/BIP39/Mnemonic.cpp \
		Encryptors/Utils/BIP39/TextSquasher.cpp \
		Encryptors/Utils/SecureIdentifier.cpp \
		EntropySources/DummyEntropySource.cpp \
		EntropySources/SPIEntropySource.cpp \
		Expressions/ArithmeticExpression.cpp \
		Expressions/ComparisonExpression.cpp \
		Expressions/FunctionExpression.cpp \
		Expressions/IdentifierExpression.cpp \
		Expressions/LogicalExpression.cpp \
		EventClocks/DummyEventClock.cpp \
		GraphicsDrivers/GraphicsDriver.cpp \
		GraphicsDrivers/Headless.cpp \
		GraphicsDrivers/RA8873.cpp \
		GraphicsDrivers/RA8875.cpp \
		Highlighters/Factories/CarloHighlighterFactory.cpp \
		Highlighters/CarloHighlighter.cpp \
		InputDevices/NullInputDevice.cpp \
		InputDevices/SPIKeyboard.cpp \
		LineDrivers/NullLineDriver.cpp \
		LineDrivers/PICSerialLineDriver.cpp \
		Lines/DummyModemLine.cpp \
		Lines/Line.cpp \
		Lines/ModemLine.cpp \
		Loggers/Logger.cpp \
		Loggers/SerialLogger.cpp \
		Memories/KiamaFSMemory.cpp \
		Memories/Memory.cpp \
		Memories/RAMMemory.cpp \
		Memories/SPIFlash.cpp \
		NativeActors/EntropyActor.cpp \
		NativeActors/EventTimerActor.cpp \
		NativeActors/Musician.cpp \
		NativeActors/PlatformActor.cpp \
		NativeActors/SnowflakeActor.cpp \
		NativeActors/ThingActor.cpp \
		NativeActors/UserActor.cpp \
		NativeActors/WorldActor.cpp \
		Platforms/BoopiePlatform.cpp \
		Platforms/Platform.cpp \
		PresenceLoaders/Factories/DummyPresenceLoaderFactory.cpp \
		PresenceLoaders/Factories/EncryptedPresenceLoaderFactory.cpp \
		PresenceLoaders/Factories/Linda2PresenceLoaderFactory.cpp \
		PresenceLoaders/DummyPresenceLoader.cpp \
		PresenceLoaders/EncryptedPresenceLoader.cpp \
		PresenceLoaders/Linda2PresenceLoader.cpp \
		PresenceLoaders/PresenceLoader.cpp \
		PresenceLoaders/PresenceRequest.cpp \
		SceneLoaders/Factories/DemoSceneLoaderFactory.cpp \
		SceneLoaders/Factories/EncryptedSceneLoaderFactory.cpp \
		SceneLoaders/Factories/Linda2SceneLoaderFactory.cpp \
		SceneLoaders/DemoSceneLoader.cpp \
		SceneLoaders/EncryptedSceneLoader.cpp \
		SceneLoaders/Linda2SceneLoader.cpp \
		SceneLoaders/Linda2SceneLoaderResponder.cpp \
		SceneLoaders/SceneLoader.cpp \
		SceneLoaders/SceneRequest.cpp \
		Statements/CreatesStatement.cpp \
		Statements/EachStatement.cpp \
		Statements/IfStatement.cpp \
		Statements/MakesStatement.cpp \
		Statements/SendsStatement.cpp \
		Statements/StopStatement.cpp \
		Statements/WhileStatement.cpp \
		TelegramLoaders/Factories/DummyTelegramLoaderFactory.cpp \
		TelegramLoaders/Factories/Linda2TelegramLoaderFactory.cpp \
		TelegramLoaders/DummyTelegramLoader.cpp \
		TelegramLoaders/Linda2TelegramLoader.cpp \
		TelegramLoaders/TelegramLoader.cpp \
		Timers/Factories/PIC32PrecisionTimerFactory.cpp \
		Timers/Factories/PIC32TimerFactory.cpp \
		Timers/NullTimer.cpp \
		Timers/PIC32AbsoluteTimer.cpp \
		Timers/PIC32PrecisionTimer.cpp \
		Timers/PIC32Timer.cpp \
		TupleRoutes/NullTupleRoute.cpp \
		TupleRoutes/ReadableWritableTupleRoute.cpp \
		TupleRoutes/TupleRoute.cpp \
		UI/Forms/Utils/BIP39FormUtil.cpp \
		UI/Forms/Form.cpp \
		UI/Forms/FormTypes.cpp \
		UI/Strategies/ANSIEditorStrategy.cpp \
		UI/Strategies/CarloStrategy.cpp \
		UI/Strategies/ChooseAvatar.cpp \
		UI/Strategies/ConnectStrategy.cpp \
		UI/Strategies/ConnectWiFi.cpp \
		UI/Strategies/CreateWorld.cpp \
		UI/Strategies/Credits.cpp \
		UI/Strategies/EditWorldStrategy.cpp \
		UI/Strategies/EnterKeyStrategy.cpp \
		UI/Strategies/EnterWorldStrategy.cpp \
		UI/Strategies/InviteFriendStrategy.cpp \
		UI/Strategies/JoinWorld.cpp \
		UI/Strategies/Linda2Strategy.cpp \
		UI/Strategies/MemoryStrategy.cpp \
		UI/Strategies/Menu.cpp \
		UI/Strategies/MessageStrategy.cpp \
		UI/Strategies/MiniMapStrategy.cpp \
		UI/Strategies/ModemConfigStrategy.cpp \
		UI/Strategies/OnboardingStrategy.cpp \
		UI/Strategies/PhonebookStrategy.cpp \
		UI/Strategies/SettingsStrategy.cpp \
		UI/Strategies/Splash.cpp \
		UI/Strategies/StrategyHelper.cpp \
		UI/Strategies/TelaStrategy.cpp \
		UI/Strategies/TelegramStrategy.cpp \
		UI/Strategies/TeleportStrategy.cpp \
		UI/Strategies/TestStrategy.cpp \
		UI/Strategies/WalkStrategy.cpp \
		UI/Strategies/WoodStrategy.cpp \
		UI/Strategies/WorldbookStrategy.cpp \
		UI/Dialogue.cpp \
		UI/Hotkeys.cpp \
		UI/Navigation.cpp \
		UI/NotificationsUI.cpp \
		UI/PlatformUI.cpp \
		UI/PresenceUI.cpp \
		UI/TabBar.cpp \
		UI/VRTime.cpp \
		Utils/Cartesian.cpp \
		Utils/EscapeBase64.cpp \
		Utils/LiteStream.cpp \
		Utils/printf.cpp \
		Utils/RingBuffer.cpp \
		Utils/Snowflake.cpp \
		Utils/StrToHex.cpp \
		Utils/Tokeniser.cpp \
		ValueLoaders/SceneItemValueLoader.cpp \
		World/Compositor.cpp \
		World/Direction.cpp \
		World/MiniMap.cpp \
		World/Scene.cpp \
		World/SceneItem.cpp \
		World/ScenePresence.cpp \
		World/Telegram.cpp \
		World/Teleport.cpp \
		World/UniverseStats.cpp \
		World/User.cpp \
		World/WorldCoordinates.cpp \
		World/WorldMetadata.cpp \
		World/WorldSummary.cpp \
		World/WorldUtilities.cpp \
		WorldLoaders/Factories/DummyWorldLoaderFactory.cpp \
		WorldLoaders/Factories/Linda2WorldLoaderFactory.cpp \
		WorldLoaders/DummyWorldLoader.cpp \
		WorldLoaders/Linda2WorldLoader.cpp \
		Allocator.cpp \
		ANSIEditor.cpp \
		ANSIEditorFactory.cpp \
		ANSITerminal.cpp \
		ANSIWriter.cpp \
		Block.cpp \
		BoopieOfflineClientBuilder.cpp \
		BoopieOnlineClientBuilder.cpp \
		BusController.cpp \
		Chat.cpp \
		Chooser.cpp \
		Client.cpp \
		ConfigurationStore.cpp \
		ConnectionMonitor.cpp \
		DummyData.cpp \
		Editor.cpp \
		EditorFactory.cpp \
		ExecutionContext.cpp \
		FunctionDispatcher.cpp \
		InbuiltFunctions.cpp \
		KeyUtilities.cpp \
		KiamaFS.cpp \
		Lexer.cpp \
		Linda2.cpp \
		Multiplexer.cpp \
		Parser.cpp \
		Phonebook.cpp \
		PICSerial.cpp \
		ProgramManager.cpp \
		Promise.cpp \
		RandomReadableWritable.cpp \
		ReadableWritable.cpp \
		RWBuffer.cpp \
		SPIController.cpp \
		SPIRequester.cpp \
		String.cpp \
		StringConstants.cpp \
		SyntaxTreeNode.cpp \
		Session.cpp \
		Terminal.cpp \
		Tuple.cpp \
		TupleDispatcher.cpp \
		TupleHandler.cpp \
		TupleRouter.cpp \
		TupleRoutingCriteria.cpp \
		Value.cpp \
		Warp.cpp \
		WindowManager.cpp \
		Worldbook.cpp \
		ClientBuilder.cpp \
		main.cpp \
		InterruptHandler.cpp

CSOURCES=Encryptors/AES/tiny-AES-c/aes.c \
         Encryptors/SHA256/sha256.c \
		 Utils/base64/base64.c

OBJECTS:=${SOURCES:.cpp=.o} ${CSOURCES:.c=.o}
DEPS:=$(SOURCES:.cpp=.d) $(CSOURCES:.c=.d)

EXECUTABLE=agape
HEXFILE=$(EXECUTABLE).hex

all: $(EXECUTABLE) $(HEXFILE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(HEXFILE): $(EXECUTABLE)
	$(BIN2HEX) $(EXECUTABLE)

%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

%.d: %.cpp
	@echo [DEP] $*
	@mkdir -p $(@D)
	@set -e; rm -f $@; \
	 $(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	 sed 's,$(notdir $*)\.o[ :]*,$*.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$

%.d: %.c
	@echo [DEP] $*
	@mkdir -p $(@D)
	@set -e; rm -f $@; \
	 $(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	 sed 's,$(notdir $*)\.o[ :]*,$*.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$

.PHONY: clean
clean:
	rm -rf $(OBJECTS) $(DEPS) agape *.i *.ii *.s *.d* *.map

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif
