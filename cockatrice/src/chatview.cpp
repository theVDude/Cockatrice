#include <QTextEdit>
#include <QDateTime>
#include <QScrollBar>
#include <QMouseEvent>
#include <QDesktopServices>
#include "chatview.h"

ChatView::ChatView(const QString &_ownName, bool _showTimestamps, QWidget *parent)
	: QTextBrowser(parent), evenNumber(true), ownName(_ownName), showTimestamps(_showTimestamps)
{
	setReadOnly(true);
	setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
	setOpenLinks(false);
	connect(this, SIGNAL(anchorClicked(const QUrl &)), this, SLOT(openLink(const QUrl &)));
}

QTextCursor ChatView::prepareBlock(bool same)
{
	lastSender.clear();
	
	QTextCursor cursor(document()->lastBlock());
	cursor.movePosition(QTextCursor::End);
	if (!same) {
		QTextBlockFormat blockFormat;
		if ((evenNumber = !evenNumber))
			blockFormat.setBackground(palette().alternateBase());
		blockFormat.setBottomMargin(2);
		cursor.insertBlock(blockFormat);
	} else
		cursor.insertHtml("<br>");
	
	return cursor;
}

void ChatView::appendHtml(const QString &html)
{
	bool atBottom = verticalScrollBar()->value() >= verticalScrollBar()->maximum();
	prepareBlock().insertHtml(html);
	if (atBottom)
		verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void ChatView::appendMessage(QString sender, QString message, QColor playerColor, bool playerBold)
{
	bool atBottom = verticalScrollBar()->value() >= verticalScrollBar()->maximum();
	bool sameSender = (sender == lastSender) && !lastSender.isEmpty();
	QTextCursor cursor = prepareBlock(sameSender);
	lastSender = sender;
	
	if (showTimestamps && !sameSender) {
		QTextCharFormat timeFormat;
		timeFormat.setForeground(Qt::black);
		cursor.setCharFormat(timeFormat);
		cursor.insertText(QDateTime::currentDateTime().toString("[hh:mm] "));
	}
	
	QTextCharFormat senderFormat;
	if (sender == ownName) {
		senderFormat.setFontWeight(QFont::Bold);
		senderFormat.setForeground(Qt::red);
	} else {
		if (playerColor == QColor())
			senderFormat.setForeground(QColor(0, 0, 254));
		else
			senderFormat.setForeground(playerColor);
		if (playerBold)
			senderFormat.setFontWeight(QFont::Bold);
	}
	if (!sameSender) {
		cursor.setCharFormat(senderFormat);
		if (!sender.isEmpty())
			sender.append(": ");
		cursor.insertText(sender);
	} else
		cursor.insertText("    ");
	
	QTextCharFormat messageFormat;
	if (sender.isEmpty())
		messageFormat.setForeground(Qt::darkGreen);
	cursor.setCharFormat(messageFormat);
	
	int from = 0, index = 0;
	while ((index = message.indexOf('[', from)) != -1) {
		cursor.insertText(message.left(index));
		message = message.mid(index);
		if (message.isEmpty())
			break;
		
		if (message.startsWith("[card]")) {
			message = message.mid(6);
			QTextCharFormat tempFormat = messageFormat;
			tempFormat.setForeground(Qt::blue);
			cursor.setCharFormat(tempFormat);
			int closeTagIndex = message.indexOf("[/card]");
			cursor.insertText(message.left(closeTagIndex));
			cursor.setCharFormat(messageFormat);
			if (closeTagIndex == -1)
				message.clear();
			else
				message = message.mid(closeTagIndex + 7);
		} else if (message.startsWith("[url]")) {
			message = message.mid(5);
			int closeTagIndex = message.indexOf("[/url]");
			QString url = message.left(closeTagIndex);
			if (!url.startsWith("http://"))
				url.prepend("http://");
			QTextCharFormat tempFormat = messageFormat;
			tempFormat.setForeground(QColor(0, 0, 254));
			tempFormat.setAnchor(true);
			tempFormat.setAnchorHref(url);
			cursor.setCharFormat(tempFormat);
			cursor.insertText(url);
			cursor.setCharFormat(messageFormat);
			if (closeTagIndex == -1)
				message.clear();
			else
				message = message.mid(closeTagIndex + 6);
		} else
			from = 1;
	}
	if (!message.isEmpty())
		cursor.insertText(message);
	
	if (atBottom)
		verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void ChatView::enterEvent(QEvent * /*event*/)
{
	setMouseTracking(true);
}

void ChatView::leaveEvent(QEvent * /*event*/)
{
	setMouseTracking(false);
}

QTextFragment ChatView::getFragmentUnderMouse(const QPoint &pos) const
{
	QTextCursor cursor(cursorForPosition(pos));
	QTextBlock block(cursor.block());
	QTextBlock::iterator it;
	for (it = block.begin(); !(it.atEnd()); ++it) {
		QTextFragment frag = it.fragment();
		if (frag.contains(cursor.position()))
			return frag;
	}
	return QTextFragment();
}

QString ChatView::getCardNameUnderMouse(QTextFragment frag) const
{
	if (frag.charFormat().foreground().color() == Qt::blue)
		return frag.text();
	return QString();
}

QString ChatView::getCardNameUnderMouse(const QPoint &pos) const
{
	return getCardNameUnderMouse(getFragmentUnderMouse(pos));
}

void ChatView::mouseMoveEvent(QMouseEvent *event)
{
	QTextFragment frag = getFragmentUnderMouse(event->pos());
	QString cardName = getCardNameUnderMouse(frag);
	if (!cardName.isEmpty()) {
		viewport()->setCursor(Qt::PointingHandCursor);
		emit cardNameHovered(cardName);
	} else if (frag.charFormat().isAnchor())
		viewport()->setCursor(Qt::PointingHandCursor);
	else
		viewport()->setCursor(Qt::IBeamCursor);

	QTextBrowser::mouseMoveEvent(event);
}

void ChatView::mousePressEvent(QMouseEvent *event)
{
	if ((event->button() == Qt::MidButton) || (event->button() == Qt::LeftButton)) {
		QString cardName = getCardNameUnderMouse(event->pos());
		if (!cardName.isEmpty())
			emit showCardInfoPopup(event->globalPos(), cardName);
	}
	
	QTextBrowser::mousePressEvent(event);
}

void ChatView::mouseReleaseEvent(QMouseEvent *event)
{
	if ((event->button() == Qt::MidButton) || (event->button() == Qt::LeftButton))
		emit deleteCardInfoPopup(QString("_"));
	
	QTextBrowser::mouseReleaseEvent(event);
}

void ChatView::openLink(const QUrl &link)
{
	QDesktopServices::openUrl(link);
}
