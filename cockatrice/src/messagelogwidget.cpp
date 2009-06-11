#include "messagelogwidget.h"
#include "game.h"
#include <QScrollBar>

void MessageLogWidget::logConnecting(QString hostname)
{
	append(tr("Connecting to %1...").arg(hostname));
}

void MessageLogWidget::logConnected(const QStringList WelcomeMsg)
{
	append(tr("Connected."));

	QStringListIterator i(WelcomeMsg);
	while (i.hasNext())
		append(i.next());
}

void MessageLogWidget::logDisconnected()
{
	append(tr("Disconnected from server."));
}

void MessageLogWidget::logSocketError(const QString &errorString)
{
	append(errorString);
}

void MessageLogWidget::logPlayerListReceived(QStringList players)
{
	append("---");
	append(tr("You have joined the game. Player list:"));
	for (int i = 0; i < players.size(); i++)
		append(players.at(i));
	append("---");
}

void MessageLogWidget::logJoin(QString playerName)
{
	append(tr("%1 has joined the game").arg(playerName));
}

void MessageLogWidget::logLeave(QString playerName)
{
	append(tr("%1 has left the game").arg(playerName));
}

void MessageLogWidget::logReadyStart(QString playerName)
{
	append(tr("%1 is ready to start a new game.").arg(playerName));
}

void MessageLogWidget::logGameStart()
{
	append(tr("Game has started."));
}

void MessageLogWidget::logSay(QString playerName, QString message)
{
	append(QString("<font color=\"red\">%1:</font> %2").arg(playerName).arg(message));
}

void MessageLogWidget::logShuffle(QString playerName)
{
	append(tr("%1 shuffles his/her library").arg(playerName));
}

void MessageLogWidget::logRollDice(QString playerName, int sides, int roll)
{
	append(tr("%1 rolled a %2 with a %3-sided dice").arg(playerName).arg(roll).arg(sides));
}

void MessageLogWidget::logDraw(QString playerName, int number)
{
	if (number == 1)
		append(tr("%1 draws a card").arg(playerName));
	else
		append(tr("%1 draws %2 cards").arg(playerName).arg(number));
}

void MessageLogWidget::logMoveCard(QString playerName, QString cardName, QString startZone, QString targetZone)
{
	append(tr("%1 moves %2 from %3 to %4").arg(playerName).arg(cardName).arg(startZone).arg(targetZone));
}

void MessageLogWidget::logCreateToken(QString playerName, QString cardName)
{
	append(tr("%1 creates token: %2").arg(playerName).arg(cardName));
}

void MessageLogWidget::logSetCardCounters(QString playerName, QString cardName, int value, int oldValue)
{
	if (value > oldValue)
		append(tr("%1 places %2 counters on %3 (now %4)").arg(playerName).arg(value - oldValue).arg(cardName).arg(value));
	else
		append(tr("%1 removes %2 counters from %3 (now %4)").arg(playerName).arg(oldValue - value).arg(cardName).arg(value));
}

void MessageLogWidget::logSetTapped(QString playerName, QString cardName, bool tapped)
{
	if (cardName == "-1")
		cardName = tr("his permanents");
	append(tr("%1 %2 %3").arg(playerName).arg(tapped ? "taps" : "untaps").arg(cardName));
}

void MessageLogWidget::logSetCounter(QString playerName, QString counterName, int value, int oldValue)
{
	append(tr("%1 sets counter \"%2\" to %3 (%4%5)").arg(playerName).arg(counterName).arg(value).arg(value > oldValue ? "+" : "").arg(value - oldValue));
}

void MessageLogWidget::logSetDoesntUntap(QString playerName, QString cardName, bool doesntUntap)
{
	if (doesntUntap)
		append(tr("%1 sets %2 to not untap normally.").arg(playerName).arg(cardName));
	else
		append(tr("%1 sets %2 to untap normally.").arg(playerName).arg(cardName));
}

void MessageLogWidget::logDumpZone(QString playerName, QString zoneName, QString zoneOwner, int numberCards)
{
	if (numberCards)
		append(tr("%1 is looking at the top %2 cards of %3's %4").arg(playerName).arg(numberCards).arg(zoneOwner).arg(zoneName));
	else
		append(tr("%1 is looking at %2's %3").arg(playerName).arg(zoneOwner).arg(zoneName));
}


void MessageLogWidget::connectToGame(Game *game)
{
	connect(game, SIGNAL(logPlayerListReceived(QStringList)), this, SLOT(logPlayerListReceived(QStringList)));
	connect(game, SIGNAL(logJoin(QString)), this, SLOT(logJoin(QString)));
	connect(game, SIGNAL(logLeave(QString)), this, SLOT(logLeave(QString)));
	connect(game, SIGNAL(logReadyStart(QString)), this, SLOT(logReadyStart(QString)));
	connect(game, SIGNAL(logGameStart()), this, SLOT(logGameStart()));
	connect(game, SIGNAL(logSay(QString, QString)), this, SLOT(logSay(QString, QString)));
	connect(game, SIGNAL(logShuffle(QString)), this, SLOT(logShuffle(QString)));
	connect(game, SIGNAL(logRollDice(QString, int, int)), this, SLOT(logRollDice(QString, int, int)));
	connect(game, SIGNAL(logDraw(QString, int)), this, SLOT(logDraw(QString, int)));
	connect(game, SIGNAL(logMoveCard(QString, QString, QString, QString)), this, SLOT(logMoveCard(QString, QString, QString, QString)));
	connect(game, SIGNAL(logCreateToken(QString, QString)), this, SLOT(logCreateToken(QString, QString)));
	connect(game, SIGNAL(logSetCardCounters(QString, QString, int, int)), this, SLOT(logSetCardCounters(QString, QString, int, int)));
	connect(game, SIGNAL(logSetTapped(QString, QString, bool)), this, SLOT(logSetTapped(QString, QString, bool)));
	connect(game, SIGNAL(logSetCounter(QString, QString, int, int)), this, SLOT(logSetCounter(QString, QString, int, int)));
	connect(game, SIGNAL(logSetDoesntUntap(QString, QString, bool)), this, SLOT(logSetDoesntUntap(QString, QString, bool)));
	connect(game, SIGNAL(logDumpZone(QString, QString, QString, int)), this, SLOT(logDumpZone(QString, QString, QString, int)));
}

MessageLogWidget::MessageLogWidget(QWidget *parent)
	: QTextEdit(parent)
{
	setReadOnly(true);
	QFont f;
	f.setPointSize(9);
	setFont(f);
}