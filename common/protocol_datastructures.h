#ifndef PROTOCOL_DATASTRUCTURES_H
#define PROTOCOL_DATASTRUCTURES_H

#include <QString>
#include <QDateTime>
#include "serializable_item.h"
#include "color.h"

class DeckList;

enum ResponseCode { RespNothing, RespOk, RespNotInRoom, RespInternalError, RespInvalidCommand, RespInvalidData, RespNameNotFound, RespLoginNeeded, RespFunctionNotAllowed, RespGameNotStarted, RespGameFull, RespContextError, RespWrongPassword, RespSpectatorsNotAllowed, RespOnlyBuddies, RespUserLevelTooLow, RespInIgnoreList, RespWouldOverwriteOldSession, RespChatFlood };

// PrivateZone: Contents of the zone are always visible to the owner,
// but not to anyone else.
// PublicZone: Contents of the zone are always visible to anyone.
// HiddenZone: Contents of the zone are never visible to anyone.
// However, the owner of the zone can issue a dump_zone command,
// setting beingLookedAt to true.
// Cards in a zone with the type HiddenZone are referenced by their
// list index, whereas cards in any other zone are referenced by their ids.
enum ZoneType { PrivateZone, PublicZone, HiddenZone };

class CardToMove : public SerializableItem_Map {
public:
	CardToMove(int _cardId = -1, bool _faceDown = false, const QString &_pt = QString(), bool _tapped = false);
	static SerializableItem *newItem() { return new CardToMove; }
	int getCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_id"))->getData(); }
	bool getFaceDown() const { return static_cast<SerializableItem_Bool *>(itemMap.value("facedown"))->getData(); }
	QString getPT() const { return static_cast<SerializableItem_String *>(itemMap.value("pt"))->getData(); }
	bool getTapped() const { return static_cast<SerializableItem_Bool *>(itemMap.value("tapped"))->getData(); }
};

class GameTypeId : public SerializableItem_Int {
public:
	GameTypeId(int _gameTypeId = -1) : SerializableItem_Int("game_type_id", _gameTypeId) { }
	static SerializableItem *newItem() { return new GameTypeId; }
};

class ServerInfo_User : public SerializableItem_Map {
public:
	enum UserLevelFlags {
		IsNothing = 0x00,
		IsUser = 0x01,
		IsRegistered = 0x02,
		IsModerator = 0x04,
		IsAdmin = 0x08
	};
	enum Gender {
		GenderUnknown = -1,
		Male = 0,
		Female = 1
	};
	ServerInfo_User(const QString &_name = QString(), int _userLevel = IsNothing, const QString &_address = QString(), const QString &_realName = QString(), Gender _gender = GenderUnknown, const QString &_country = QString(), const QByteArray &_avatarBmp = QByteArray());
	ServerInfo_User(const ServerInfo_User *other, bool complete = true, bool moderatorInfo = false);
	static SerializableItem *newItem() { return new ServerInfo_User; }
	QString getName() const { return static_cast<SerializableItem_String *>(itemMap.value("name"))->getData(); }
	void setName(const QString &_name) { static_cast<SerializableItem_String *>(itemMap.value("name"))->setData(_name); }
	int getUserLevel() const { return static_cast<SerializableItem_Int *>(itemMap.value("userlevel"))->getData(); }
	void setUserLevel(int _userLevel) { static_cast<SerializableItem_Int *>(itemMap.value("userlevel"))->setData(_userLevel); }
	QString getAddress() const { return static_cast<SerializableItem_String *>(itemMap.value("address"))->getData(); }
	void setAddress(const QString &_address) { static_cast<SerializableItem_String *>(itemMap.value("address"))->setData(_address); }
	QString getRealName() const { return static_cast<SerializableItem_String *>(itemMap.value("real_name"))->getData(); }
	Gender getGender() const { return static_cast<Gender>(static_cast<SerializableItem_Int *>(itemMap.value("gender"))->getData()); }
	QString getCountry() const { return static_cast<SerializableItem_String *>(itemMap.value("country"))->getData(); }
	QByteArray getAvatarBmp() const { return static_cast<SerializableItem_ByteArray *>(itemMap.value("avatar_bmp"))->getData(); }
};

class ServerInfo_UserList : public SerializableItem_Map {
public:
	ServerInfo_UserList(const QString &_itemType, const QList<ServerInfo_User *> &_userList = QList<ServerInfo_User *>());
	QList<ServerInfo_User *> getUserList() const { return typecastItemList<ServerInfo_User *>(); }
};

class ServerInfo_Game : public SerializableItem_Map {
public:
	ServerInfo_Game(int _roomId = -1, int _gameId = -1, const QString &_description = QString(), bool _hasPassword = false, int _playerCount = -1, int _maxPlayers = -1, bool _started = false, const QList<GameTypeId *> &_gameTypes = QList<GameTypeId *>(), ServerInfo_User *creatorInfo = 0, bool _onlyBuddies = false, bool _onlyRegistered = false, bool _spectatorsAllowed = false, bool _spectatorsNeedPassword = false, int _spectatorCount = -1);
	static SerializableItem *newItem() { return new ServerInfo_Game; }
	int getRoomId() const { return static_cast<SerializableItem_Int *>(itemMap.value("room_id"))->getData(); }
	int getGameId() const { return static_cast<SerializableItem_Int *>(itemMap.value("game_id"))->getData(); }
	QString getDescription() const { return static_cast<SerializableItem_String *>(itemMap.value("description"))->getData(); }
	bool getHasPassword() const { return static_cast<SerializableItem_Bool *>(itemMap.value("has_password"))->getData(); }
	int getPlayerCount() const { return static_cast<SerializableItem_Int *>(itemMap.value("player_count"))->getData(); }
	int getMaxPlayers() const { return static_cast<SerializableItem_Int *>(itemMap.value("max_players"))->getData(); }
	bool getStarted() const { return static_cast<SerializableItem_Bool *>(itemMap.value("started"))->getData(); }
	QList<GameTypeId *> getGameTypes() const { return typecastItemList<GameTypeId *>(); }
	ServerInfo_User *getCreatorInfo() const { return static_cast<ServerInfo_User *>(itemMap.value("user")); }
	bool getOnlyBuddies() const { return static_cast<SerializableItem_Bool *>(itemMap.value("only_buddies"))->getData(); }
	bool getOnlyRegistered() const { return static_cast<SerializableItem_Bool *>(itemMap.value("only_registered"))->getData(); }
	bool getSpectatorsAllowed() const { return static_cast<SerializableItem_Bool *>(itemMap.value("spectators_allowed"))->getData(); }
	bool getSpectatorsNeedPassword() const { return static_cast<SerializableItem_Bool *>(itemMap.value("spectators_need_password"))->getData(); }
	int getSpectatorCount() const { return static_cast<SerializableItem_Int *>(itemMap.value("spectator_count"))->getData(); }
};

class ServerInfo_GameType : public SerializableItem_Map {
public:
	ServerInfo_GameType(int _gameTypeId = -1, const QString &_description = QString());
	static SerializableItem *newItem() { return new ServerInfo_GameType; }
	int getGameTypeId() const { return static_cast<SerializableItem_Int *>(itemMap.value("game_type_id"))->getData(); }
	QString getDescription() const { return static_cast<SerializableItem_String *>(itemMap.value("description"))->getData(); }
};

class ServerInfo_Room : public SerializableItem_Map {
private:
	QList<ServerInfo_Game *> gameList;
	QList<ServerInfo_User *> userList;
	QList<ServerInfo_GameType *> gameTypeList;
protected:
	void extractData();
public:
	ServerInfo_Room(int _id = -1, const QString &_name = QString(), const QString &_description = QString(), int _gameCount = -1, int _playerCount = -1, bool _autoJoin = false, const QList<ServerInfo_Game *> &_gameList = QList<ServerInfo_Game *>(), const QList<ServerInfo_User *> &_userList = QList<ServerInfo_User *>(), const QList<ServerInfo_GameType *> &_gameTypeList = QList<ServerInfo_GameType *>());
	static SerializableItem *newItem() { return new ServerInfo_Room; }
	int getRoomId() const { return static_cast<SerializableItem_Int *>(itemMap.value("room_id"))->getData(); }
	QString getName() const { return static_cast<SerializableItem_String *>(itemMap.value("name"))->getData(); }
	QString getDescription() const { return static_cast<SerializableItem_String *>(itemMap.value("description"))->getData(); }
	int getGameCount() const { return static_cast<SerializableItem_Int *>(itemMap.value("game_count"))->getData(); }
	int getPlayerCount() const { return static_cast<SerializableItem_Int *>(itemMap.value("player_count"))->getData(); }
	bool getAutoJoin() const { return static_cast<SerializableItem_Bool *>(itemMap.value("auto_join"))->getData(); }
	const QList<ServerInfo_Game *> &getGameList() const { return gameList; }
	const QList<ServerInfo_User *> &getUserList() const { return userList; }
	const QList<ServerInfo_GameType *> &getGameTypeList() const { return gameTypeList; }
};

class ServerInfo_CardCounter : public SerializableItem_Map {
public:
	ServerInfo_CardCounter(int _id = -1, int _value = 0);
	static SerializableItem *newItem() { return new ServerInfo_CardCounter; }
	int getId() const { return static_cast<SerializableItem_Int *>(itemMap.value("id"))->getData(); }
	int getValue() const { return static_cast<SerializableItem_Int *>(itemMap.value("value"))->getData(); }
};

class ServerInfo_Card : public SerializableItem_Map {
public:
	ServerInfo_Card(int _id = -1, const QString &_name = QString(), int _x = -1, int _y = -1, bool _faceDown = false, bool _tapped = false, bool _attacking = false, const QString &_color = QString(), const QString &_pt = QString(), const QString &_annotation = QString(), bool _destroyOnZoneChange = false, bool _doesntUntap = false, const QList<ServerInfo_CardCounter *> &_counterList = QList<ServerInfo_CardCounter *>(), int attachPlayerId = -1, const QString &_attachZone = QString(), int attachCardId = -1);
	static SerializableItem *newItem() { return new ServerInfo_Card; }
	int getId() const { return static_cast<SerializableItem_Int *>(itemMap.value("id"))->getData(); }
	QString getName() const { return static_cast<SerializableItem_String *>(itemMap.value("name"))->getData(); }
	int getX() const { return static_cast<SerializableItem_Int *>(itemMap.value("x"))->getData(); }
	int getY() const { return static_cast<SerializableItem_Int *>(itemMap.value("y"))->getData(); }
	bool getFaceDown() const { return static_cast<SerializableItem_Bool *>(itemMap.value("facedown"))->getData(); }
	bool getTapped() const { return static_cast<SerializableItem_Bool *>(itemMap.value("tapped"))->getData(); }
	bool getAttacking() const { return static_cast<SerializableItem_Bool *>(itemMap.value("attacking"))->getData(); }
	QString getColor() const { return static_cast<SerializableItem_String *>(itemMap.value("color"))->getData(); }
	QString getPT() const { return static_cast<SerializableItem_String *>(itemMap.value("pt"))->getData(); }
	QString getAnnotation() const { return static_cast<SerializableItem_String *>(itemMap.value("annotation"))->getData(); }
	bool getDestroyOnZoneChange() const { return static_cast<SerializableItem_Bool *>(itemMap.value("destroy_on_zone_change"))->getData(); }
	bool getDoesntUntap() const { return static_cast<SerializableItem_Bool *>(itemMap.value("doesnt_untap"))->getData(); }
	QList<ServerInfo_CardCounter *> getCounters() const { return typecastItemList<ServerInfo_CardCounter *>(); }
	int getAttachPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("attach_player_id"))->getData(); }
	QString getAttachZone() const { return static_cast<SerializableItem_String *>(itemMap.value("attach_zone"))->getData(); }
	int getAttachCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("attach_card_id"))->getData(); }
};

class ServerInfo_Zone : public SerializableItem_Map {
private:
	ZoneType typeFromString(const QString &type) const;
	QString typeToString(ZoneType type) const;
public:
	ServerInfo_Zone(const QString &_name = QString(), ZoneType _type = PrivateZone, bool _hasCoords = false, int _cardCount = -1, const QList<ServerInfo_Card *> &_cardList = QList<ServerInfo_Card *>());
	static SerializableItem *newItem() { return new ServerInfo_Zone; }
	QString getName() const { return static_cast<SerializableItem_String *>(itemMap.value("name"))->getData(); }
	ZoneType getType() const { return typeFromString(static_cast<SerializableItem_String *>(itemMap.value("type"))->getData()); }
	bool getHasCoords() const { return static_cast<SerializableItem_Bool *>(itemMap.value("has_coords"))->getData(); }
	int getCardCount() const { return static_cast<SerializableItem_Int *>(itemMap.value("card_count"))->getData(); }
	QList<ServerInfo_Card *> getCardList() const;
};

class ServerInfo_Counter : public SerializableItem_Map {
public:
	ServerInfo_Counter(int _id = -1, const QString &_name = QString(), const Color &_color = Color(), int _radius = -1, int _count = -1);
	static SerializableItem *newItem() { return new ServerInfo_Counter; }
	int getId() const { return static_cast<SerializableItem_Int *>(itemMap.value("id"))->getData(); }
	QString getName() const { return static_cast<SerializableItem_String *>(itemMap.value("name"))->getData(); }
	Color getColor() const { return static_cast<SerializableItem_Color *>(itemMap.value("color"))->getData(); }
	int getRadius() const { return static_cast<SerializableItem_Int *>(itemMap.value("radius"))->getData(); }
	int getCount() const { return static_cast<SerializableItem_Int *>(itemMap.value("count"))->getData(); }
};

class ServerInfo_Arrow : public SerializableItem_Map {
public:
	ServerInfo_Arrow(int _id = -1, int _startPlayerId = -1, const QString &_startZone = QString(), int _startCardId = -1, int _targetPlayerId = -1, const QString &_targetZone = QString(), int _targetCardId = -1, const Color &_color = Color());
	static SerializableItem *newItem() { return new ServerInfo_Arrow; }
	int getId() const { return static_cast<SerializableItem_Int *>(itemMap.value("id"))->getData(); }
	int getStartPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("start_player_id"))->getData(); }
	QString getStartZone() const { return static_cast<SerializableItem_String *>(itemMap.value("start_zone"))->getData(); }
	int getStartCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("start_card_id"))->getData(); }
	int getTargetPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("target_player_id"))->getData(); }
	QString getTargetZone() const { return static_cast<SerializableItem_String *>(itemMap.value("target_zone"))->getData(); }
	int getTargetCardId() const { return static_cast<SerializableItem_Int *>(itemMap.value("target_card_id"))->getData(); }
	Color getColor() const { return static_cast<SerializableItem_Color *>(itemMap.value("color"))->getData(); }
};

class ServerInfo_PlayerProperties : public SerializableItem_Map {
public:
	ServerInfo_PlayerProperties(int _playerId = -1, ServerInfo_User *_userInfo = 0, bool _spectator = false, bool _conceded = false, bool _readyStart = false, const QString &_deckHash = QString());
	static SerializableItem *newItem() { return new ServerInfo_PlayerProperties; }
	int getPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("player_id"))->getData(); }
	ServerInfo_User *getUserInfo() const { return static_cast<ServerInfo_User *>(itemMap.value("user")); }
	bool getSpectator() const { return static_cast<SerializableItem_Bool *>(itemMap.value("spectator"))->getData(); }
	bool getConceded() const { return static_cast<SerializableItem_Bool *>(itemMap.value("conceded"))->getData(); }
	bool getReadyStart() const { return static_cast<SerializableItem_Bool *>(itemMap.value("ready_start"))->getData(); }
	QString getDeckHash() const { return static_cast<SerializableItem_String *>(itemMap.value("deck_hash"))->getData(); }
};

class ServerInfo_Player : public SerializableItem_Map {
private:
	QList<ServerInfo_Zone *> zoneList;
	QList<ServerInfo_Counter *> counterList;
	QList<ServerInfo_Arrow *> arrowList;
protected:
	void extractData();
public:
	ServerInfo_Player(ServerInfo_PlayerProperties *_properties = 0, DeckList *_deck = 0, const QList<ServerInfo_Zone *> &_zoneList = QList<ServerInfo_Zone *>(), const QList<ServerInfo_Counter *> &_counterList = QList<ServerInfo_Counter *>(), const QList<ServerInfo_Arrow *> &_arrowList = QList<ServerInfo_Arrow *>());
	static SerializableItem *newItem() { return new ServerInfo_Player; }
	ServerInfo_PlayerProperties *getProperties() const { return static_cast<ServerInfo_PlayerProperties *>(itemMap.value("player_properties")); }
	DeckList *getDeck() const;
	const QList<ServerInfo_Zone *> &getZoneList() const { return zoneList; }
	const QList<ServerInfo_Counter *> &getCounterList() const { return counterList; }
	const QList<ServerInfo_Arrow *> &getArrowList() const { return arrowList; }
};

class ServerInfo_PlayerPing : public SerializableItem_Map {
public:
	ServerInfo_PlayerPing(int _playerId = -1, int _pingTime = -1);
	static SerializableItem *newItem() { return new ServerInfo_PlayerPing; }
	int getPlayerId() const { return static_cast<SerializableItem_Int *>(itemMap.value("player_id"))->getData(); }
	int getPingTime() const { return static_cast<SerializableItem_Int *>(itemMap.value("ping_time"))->getData(); }
};

class DeckList_TreeItem : public SerializableItem_Map {
public:
	DeckList_TreeItem(const QString &_itemType, const QString &_name, int _id);
	QString getName() const { return static_cast<SerializableItem_String *>(itemMap.value("name"))->getData(); }
	int getId() const { return static_cast<SerializableItem_Int *>(itemMap.value("id"))->getData(); }
};
class DeckList_File : public DeckList_TreeItem {
public:
	DeckList_File(const QString &_name = QString(), int _id = -1, QDateTime _uploadTime = QDateTime());
	static SerializableItem *newItem() { return new DeckList_File; }
	QDateTime getUploadTime() const { return static_cast<SerializableItem_DateTime *>(itemMap.value("upload_time"))->getData(); }
};
class DeckList_Directory : public DeckList_TreeItem {
public:
	DeckList_Directory(const QString &_name = QString(), int _id = 0);
	static SerializableItem *newItem() { return new DeckList_Directory; }
	QList<DeckList_TreeItem *> getTreeItems() const { return typecastItemList<DeckList_TreeItem *>(); }
};

#endif
