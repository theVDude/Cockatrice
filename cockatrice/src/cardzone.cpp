#include <QMenu>
#include <QAction>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include "cardzone.h"
#include "carditem.h"
#include "player.h"
#include "zoneviewzone.h"
#include "protocol_items.h"

CardZone::CardZone(Player *_p, const QString &_name, bool _hasCardAttr, bool _isShufflable, bool _contentsKnown, QGraphicsItem *parent, bool isView)
	: AbstractGraphicsItem(parent), player(_p), name(_name), cards(_contentsKnown), view(NULL), menu(NULL), doubleClickAction(0), hasCardAttr(_hasCardAttr), isShufflable(_isShufflable)
{
	if (!isView)
		player->addZone(this);
}

CardZone::~CardZone()
{
	qDebug() << "CardZone destructor: " << name;
	delete view;
	clearContents();
}

void CardZone::retranslateUi()
{
	for (int i = 0; i < cards.size(); ++i)
		cards[i]->retranslateUi();
}

void CardZone::clearContents()
{
	for (int i = 0; i < cards.size(); i++) {
		// If an incorrectly implemented server doesn't return attached cards to whom they belong before dropping a player,
		// we have to return them to avoid a crash.
		const QList<CardItem *> &attachedCards = cards[i]->getAttachedCards();
		for (int j = 0; j < attachedCards.size(); ++j)
			attachedCards[j]->setParentItem(attachedCards[j]->getZone());
		
		player->deleteCard(cards.at(i));
	}
	cards.clear();
	emit cardCountChanged();
}

QString CardZone::getTranslatedName(bool hisOwn, GrammaticalCase gc) const
{
	QString ownerName = player->getName();
	bool female = player->getUserInfo()->getGender() == ServerInfo_User::Female;
	if (name == "hand")
		switch (gc) {
			case CaseNominative: return female ? (hisOwn ? tr("her hand", "nominative, female owner") : tr("%1's hand", "nominative, female owner").arg(ownerName)) : (hisOwn ? tr("his hand", "nominative, male owner") : tr("%1's hand", "nominative, male owner").arg(ownerName));
			case CaseGenitive: return female ? (hisOwn ? tr("of her hand", "genitive, female owner") : tr("of %1's hand", "genitive, female owner").arg(ownerName)) : (hisOwn ? tr("of his hand", "genitive, male owner") : tr("of %1's hand", "genitive, male owner").arg(ownerName));
			case CaseAccusative: return female ? (hisOwn ? tr("her hand", "accusative, female owner") : tr("%1's hand", "accusative, female owner").arg(ownerName)) : (hisOwn ? tr("his hand", "accusative, male owner") : tr("%1's hand", "accusative, male owner").arg(ownerName));
		}
	else if (name == "deck")
		switch (gc) {
			case CaseNominative: return female ? (hisOwn ? tr("her library", "nominative, female owner") : tr("%1's library", "nominative, female owner").arg(ownerName)) : (hisOwn ? tr("his library", "nominative, male owner") : tr("%1's library", "nominative, male owner").arg(ownerName));
			case CaseGenitive: return female ? (hisOwn ? tr("of her library", "genitive, female owner") : tr("of %1's library", "genitive, female owner").arg(ownerName)) : (hisOwn ? tr("of his library", "genitive, male owner") : tr("of %1's library", "genitive, male owner").arg(ownerName));
			case CaseAccusative: return female ? (hisOwn ? tr("her library", "accusative, female owner") : tr("%1's library", "accusative, female owner").arg(ownerName)) : (hisOwn ? tr("his library", "accusative, male owner") : tr("%1's library", "accusative, male owner").arg(ownerName));
		}
	else if (name == "grave")
		switch (gc) {
			case CaseNominative: return female ? (hisOwn ? tr("her graveyard", "nominative, female owner") : tr("%1's graveyard", "nominative, female owner").arg(ownerName)) : (hisOwn ? tr("his graveyard", "nominative, male owner") : tr("%1's graveyard", "nominative, male owner").arg(ownerName));
			case CaseGenitive: return female ? (hisOwn ? tr("of her graveyard", "genitive, female owner") : tr("of %1's graveyard", "genitive, female owner").arg(ownerName)) : (hisOwn ? tr("of his graveyard", "genitive, male owner") : tr("of %1's graveyard", "genitive, male owner").arg(ownerName));
			case CaseAccusative: return female ? (hisOwn ? tr("her graveyard", "accusative, female owner") : tr("%1's graveyard", "accusative, female owner").arg(ownerName)) : (hisOwn ? tr("his graveyard", "accusative, male owner") : tr("%1's graveyard", "accusative, male owner").arg(ownerName));
		}
	else if (name == "rfg")
		switch (gc) {
			case CaseNominative: return female ? (hisOwn ? tr("her exile", "nominative, female owner") : tr("%1's exile", "nominative, female owner").arg(ownerName)) : (hisOwn ? tr("his exile", "nominative, male owner") : tr("%1's exile", "nominative, male owner").arg(ownerName));
			case CaseGenitive: return female ? (hisOwn ? tr("of her exile", "genitive, female owner") : tr("of %1's exile", "genitive, female owner").arg(ownerName)) : (hisOwn ? tr("of his exile", "genitive, male owner") : tr("of %1's exile", "genitive, male owner").arg(ownerName));
			case CaseAccusative: return female ? (hisOwn ? tr("her exile", "accusative, female owner") : tr("%1's exile", "accusative, female owner").arg(ownerName)) : (hisOwn ? tr("his exile", "accusative, male owner") : tr("%1's exile", "accusative, male owner").arg(ownerName));
		}
	else if (name == "sb")
		switch (gc) {
			case CaseNominative: return female ? (hisOwn ? tr("her sideboard", "nominative, female owner") : tr("%1's sideboard", "nominative, female owner").arg(ownerName)) : (hisOwn ? tr("his sideboard", "nominative, male owner") : tr("%1's sideboard", "nominative, male owner").arg(ownerName));
			case CaseGenitive: return female ? (hisOwn ? tr("of her sideboard", "genitive, female owner") : tr("of %1's sideboard", "genitive, female owner").arg(ownerName)) : (hisOwn ? tr("of his sideboard", "genitive, male owner") : tr("of %1's sideboard", "genitive, male owner").arg(ownerName));
			case CaseAccusative: return female ? (hisOwn ? tr("her sideboard", "accusative, female owner") : tr("%1's sideboard", "accusative, female owner").arg(ownerName)) : (hisOwn ? tr("his sideboard", "accusative, male owner") : tr("%1's sideboard", "accusative, male owner").arg(ownerName));
		}
	return QString();
}

void CardZone::mouseDoubleClickEvent(QGraphicsSceneMouseEvent */*event*/)
{
	if (doubleClickAction)
		doubleClickAction->trigger();
}

bool CardZone::showContextMenu(const QPoint &screenPos)
{
	if (menu) {
		menu->exec(screenPos);
		return true;
	}
	return false;
}

void CardZone::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->button() == Qt::RightButton) {
		if (showContextMenu(event->screenPos()))
			event->accept();
		else
			event->ignore();
	} else
		event->ignore();
}

void CardZone::addCard(CardItem *card, bool reorganize, int x, int y)
{
	if (view)
		if ((x <= view->getCards().size()) || (view->getNumberCards() == -1))
			view->addCard(new CardItem(player, card->getName(), card->getId()), reorganize, x, y);

	card->setZone(this);
	addCardImpl(card, x, y);

	if (reorganize)
		reorganizeCards();
	
	emit cardCountChanged();
}

CardItem *CardZone::getCard(int cardId, const QString &cardName)
{
	CardItem *c = cards.findCard(cardId, false);
	if (!c) {
		qDebug() << "CardZone::getCard: card id=" << cardId << "not found";
		return 0;
	}
	// If the card's id is -1, this zone is invisible,
	// so we need to give the card an id and a name as it comes out.
	// It can be assumed that in an invisible zone, all cards are equal.
	if ((c->getId() == -1) || (c->getName().isEmpty())) {
		c->setId(cardId);
		c->setName(cardName);
	}
	return c;
}

CardItem *CardZone::takeCard(int position, int cardId, bool /*canResize*/)
{
	if (position == -1) {
		// position == -1 means either that the zone is indexed by card id
		// or that it doesn't matter which card you take.
		for (int i = 0; i < cards.size(); ++i)
			if (cards[i]->getId() == cardId) {
				position = i;
				break;
			}
		if (position == -1)
			position = 0;
	}
	if (position >= cards.size())
		return 0;

	CardItem *c = cards.takeAt(position);

	if (view)
		view->removeCard(position);

	c->setId(cardId);

	reorganizeCards();
	emit cardCountChanged();
	return c;
}

void CardZone::removeCard(CardItem *card)
{
	cards.removeAt(cards.indexOf(card));
	reorganizeCards();
	emit cardCountChanged();
	player->deleteCard(card);
}

void CardZone::moveAllToZone()
{
	QList<QVariant> data = static_cast<QAction *>(sender())->data().toList();
	QString targetZone = data[0].toString();
	int targetX = data[1].toInt();

	QList<CardToMove *> idList;
	for (int i = 0; i < cards.size(); ++i)
		idList.append(new CardToMove(cards[i]->getId()));
	
	player->sendGameCommand(new Command_MoveCard(-1, getName(), idList, player->getId(), targetZone, targetX));
}

QPointF CardZone::closestGridPoint(const QPointF &point)
{
	return point;
}
