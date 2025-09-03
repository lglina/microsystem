AGAPE_DIR=$(abspath ../Agape)
CARLO_DIR=$(abspath ../Carlo)
LINDA2_DIR=$(abspath ../Linda2)
VPATH=$(AGAPE_DIR) $(CARLO_DIR) $(LINDA2_DIR)

BSONCXX_DIR=/usr/local/include/bsoncxx/v_noabi
MONGOCXX_DIR=/usr/local/include/mongocxx/v_noabi
ASIO_DIR=$(HOME)/asio-1.30.2/include
WEBSOCKETPP_DIR=$(HOME)/websocketpp-0.8.2
CPR_DIR=/usr/local/include/cpr

CXXFLAGS=--std=c++17 -DLOG_TUPLES_BRIEF -DASIO_STANDALONE -I. -I$(AGAPE_DIR) -I$(AGAPE_DIR)/AssetLoaders -I$(AGAPE_DIR)/AssetLoaders/Factories -I$(AGAPE_DIR)/Clocks -I$(AGAPE_DIR)/PresenceLoaders -I$(AGAPE_DIR)/PresenceLoaders/Factories -I$(AGAPE_DIR)/SceneLoaders -I$(AGAPE_DIR)/SceneLoaders/Factories -I$(AGAPE_DIR)/TelegramLoaders -I$(AGAPE_DIR)/TelegramLoaders/Factories -I$(AGAPE_DIR)/WorldLoaders -I$(AGAPE_DIR)/WorldLoaders/Factories -I$(CARLO_DIR) -I$(LINDA2_DIR) -I$(BSONCXX_DIR) -I$(MONGOCXX_DIR) -I$(ASIO_DIR) -I$(WEBSOCKETPP_DIR) -I$(CPR_DIR) -g

CFLAGS=-I$(AGAPE_DIR) -g

LIBS=-L/usr/local/lib64 -lbsoncxx -lmongocxx -lpthread -lssl -lcrypto -lcpr

SOURCES=Actors/NativeActors/NativeActor.cpp \
		AssetLoaders/Factories/MongoAssetLoaderFactory.cpp \
		AssetLoaders/AssetLoader.cpp \
		AssetLoaders/MongoAssetLoader.cpp \
		AssetLoaders/Linda2AssetLoaderResponder.cpp \
		Clocks/CClock.cpp \
		Databases/MongoDB/MongoDB.cpp \
		Databases/MongoDB/MongoDocumentBuilder.cpp \
		Encryptors/AES/AESEncryptor.cpp \
		Encryptors/Factories/AESEncryptorFactory.cpp \
		Encryptors/SHA256/SHA256Hash.cpp \
		Encryptors/Utils/SecureIdentifier.cpp \
		Encryptors/Encryptor.cpp \
		EntropySources/DevRandom.cpp \
		Loggers/Logger.cpp \
		Loggers/StreamLogger.cpp \
		Network/WebSocketsConnection.cpp \
		PresenceLoaders/Factories/SharedPresenceLoaderFactory.cpp \
		PresenceLoaders/Linda2PresenceLoaderResponder.cpp \
		PresenceLoaders/PresenceLoader.cpp \
		PresenceLoaders/PresenceRequest.cpp \
		PresenceLoaders/SharedPresenceLoader.cpp \
		SceneLoaders/Factories/MongoSceneLoaderFactory.cpp \
		SceneLoaders/MongoSceneLoader.cpp \
		SceneLoaders/Linda2SceneLoaderResponder.cpp \
		SceneLoaders/SceneLoader.cpp \
		SceneLoaders/SceneRequest.cpp \
		TelegramLoaders/Factories/MongoTelegramLoaderFactory.cpp \
		TelegramLoaders/Linda2TelegramLoaderResponder.cpp \
		TelegramLoaders/MongoTelegramLoader.cpp \
		TelegramLoaders/TelegramLoader.cpp \
		Timers/Factories/CTimerFactory.cpp \
		Timers/Factories/HighResTimerFactory.cpp \
		Timers/CTimer.cpp \
		Timers/HighResTimer.cpp \
		TupleFilters/StratusTupleFilter.cpp \
		TupleRoutes/QueueingTupleRoute.cpp \
		TupleRoutes/ReadableWritableTupleRoute.cpp \
		TupleRoutes/TupleRoute.cpp \
		Utils/LiteStream.cpp \
		Utils/printf.cpp \
		Utils/RingBuffer.cpp \
		Utils/Snowflake.cpp \
		Utils/StrToHex.cpp \
		World/Direction.cpp \
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
		WorldLoaders/Factories/MongoWorldLoaderFactory.cpp \
		WorldLoaders/Linda2WorldLoaderResponder.cpp \
		WorldLoaders/MongoWorldLoader.cpp \
		Authenticator.cpp \
		Block.cpp \
		ExecutionContext.cpp \
		FunctionDispatcher.cpp \
		Handler.cpp \
		HandlerFactory.cpp \
		Hydra.cpp \
		InbuiltFunctions.cpp \
		Inviter.cpp \
		KeyUtilities.cpp \
		MasterClock.cpp \
		Promise.cpp \
		PushNotifier.cpp \
		ReadableWritable.cpp \
		RWBuffer.cpp \
		Stratus.cpp \
		String.cpp \
		StringConstants.cpp \
		SyntaxTreeNode.cpp \
		Tuple.cpp \
		TupleDispatcher.cpp \
		TupleHandler.cpp \
		TupleRouter.cpp \
		TupleRoutingCriteria.cpp \
		Value.cpp \
		Warp.cpp \
		main.cpp

CSOURCES=Encryptors/SHA256/sha256.c \
         Encryptors/AES/tiny-AES-c/aes.c \
         Utils/base64/base64.c

OBJECTS:=$(patsubst %,build/%,$(patsubst %.cpp,%.o,$(SOURCES))) \
         $(patsubst %,build/%,$(patsubst %.c,%.o,$(CSOURCES)))
DEPS:=$(patsubst %,build/%,$(patsubst %.cpp,%.d,$(SOURCES))) \
	  $(patsubst %,build/%,$(patsubst %.c,%.d,$(CSOURCES)))

EXECUTABLE=Stratus

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

build/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

build/%.d: %.cpp
	@echo [DEP] $*
	@mkdir -p $(@D)
	@set -e; rm -f $@; \
	 $(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	 sed 's,$(notdir $*)\.o[ :]*,$*.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$

build/%.d: %.c
	@echo [DEP] $*
	@mkdir -p $(@D)
	@set -e; rm -f $@; \
	 $(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	 sed 's,$(notdir $*)\.o[ :]*,$*.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$

.PHONY: clean
clean:
	rm -rf $(EXECUTABLE) $(OBJECTS) $(DEPS)

-include $(DEPS)
