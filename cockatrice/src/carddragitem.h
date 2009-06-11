#ifndef CARDDRAGITEM_H
#define CARDDRAGITEM_H

#include "carditem.h"

class QGraphicsScene;
class CardZone;
class CardInfo;

class CardDragItem : public QGraphicsItem {
private:
	int id;
	CardInfo *info;
	QPointF hotSpot;
	CardZone *startZone;
	bool faceDown;
public:
	enum { Type = typeCardDrag };
	int type() const { return Type; }
	CardDragItem(QGraphicsScene *scene, CardZone *_startZone, CardInfo *_info, int _id, const QPointF &_hotSpot, bool _faceDown, QGraphicsItem *parent = 0);
	~CardDragItem();
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	QPointF getHotSpot() const { return hotSpot; }
protected:
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

#endif