#ifndef CLIENT_H
#define CLIENT_H

#include "servereventdata.h"
#include "serverresponse.h"
#include "servergame.h"
#include "serverplayer.h"
#include "serverzone.h"
#include "serverzonecard.h"
#include "pendingcommand.h"
#include <QTcpSocket>
#include <QColor>

class QTimer;

// Connection state.
// The protocol handler itself is stateless once the connection
// has been established.
enum ProtocolStatus { StatusDisconnected,
		      StatusConnecting,
		      StatusAwaitingWelcome,
		      StatusConnected };

class Client : public QObject {
	Q_OBJECT
signals:
	void statusChanged(ProtocolStatus _status);
	void welcomeMsgReceived(QStringList welcomeMsg);
	void gameListEvent(ServerGame *game);
	void playerListReceived(QList<ServerPlayer *> players);
	void zoneListReceived(int commandId, QList<ServerZone *> zones);
	void zoneDumpReceived(int commandId, QList<ServerZoneCard *> cards);
	void responseReceived(ServerResponse *response);
	void playerIdReceived(int id, QString name);
	void gameEvent(const ServerEventData &msg);
	void serverTimeout();
	void logSocketError(const QString &errorString);

private slots:
	void slotConnected();
	void readLine();
	void checkTimeout();
	void slotSocketError(QAbstractSocket::SocketError error);

private:
	QTimer *timer;
	QList<PendingCommand *> PendingCommands;
	QTcpSocket *socket;
	ProtocolStatus status;
	QList<QStringList> msgbuf;
	QString PlayerName;
	QString password;
	unsigned int MsgId;
	void msg(const QString &s);
	int cmd(const QString &s);
	void setStatus(const ProtocolStatus _status);
public:
	Client(QObject *parent = 0);
	~Client();
	ProtocolStatus getStatus() const { return status; }
	QString peerName() const { return socket->peerName(); }
	
	void connectToServer(const QString &hostname, unsigned int port, const QString &playername, const QString &password);
	void disconnectFromServer();
	int ping();
	int listGames();
	int listPlayers();
	int createGame(const QString &description, const QString &password, unsigned int maxPlayers);
	int joinGame(int gameId, const QString &password);
	int leaveGame();
	int login(const QString &name, const QString &pass);
	int say(const QString &s);
	int shuffle();
	int rollDice(unsigned int sides);
	int drawCards(unsigned int number);
	int moveCard(int cardid, const QString &startzone, const QString &targetzone, int x, int y = 0, bool faceDown = false);
	int createToken(const QString &zone, const QString &name, const QString &powtough, int x, int y);
	int setCardAttr(const QString &zone, int cardid, const QString &aname, const QString &avalue);
	int readyStart();
	int incCounter(const QString &counter, int delta);
	int addCounter(const QString &counter, QColor color, int value);
	int setCounter(const QString &counter, int value);
	int delCounter(const QString &counter);
	int setActivePlayer(int player);
	int setActivePhase(int phase);
	int dumpZone(int player, const QString &zone, int numberCards);
public slots:
	void submitDeck(const QStringList &deck);
};

#endif