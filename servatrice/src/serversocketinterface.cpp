/***************************************************************************
 *   Copyright (C) 2008 by Max-Wilhelm Bruker   *
 *   brukie@laptop   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QtSql>
#include <QHostAddress>
#include <QDebug>
#include "serversocketinterface.h"
#include "servatrice.h"
#include "protocol.h"
#include "protocol_items.h"
#include "decklist.h"
#include "server_player.h"
#include "main.h"
#include "server_logger.h"

ServerSocketInterface::ServerSocketInterface(Servatrice *_server, QTcpSocket *_socket, QObject *parent)
	: Server_ProtocolHandler(_server, parent), servatrice(_server), socket(_socket), topLevelItem(0), compressionSupport(false)
{
	xmlWriter = new QXmlStreamWriter(&xmlBuffer);
	xmlReader = new QXmlStreamReader;
	
	connect(socket, SIGNAL(readyRead()), this, SLOT(readClient()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(deleteLater()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(catchSocketError(QAbstractSocket::SocketError)));
	connect(this, SIGNAL(xmlBufferChanged()), this, SLOT(flushXmlBuffer()), Qt::QueuedConnection);
	
	xmlWriter->writeStartDocument();
	xmlWriter->writeStartElement("cockatrice_server_stream");
	xmlWriter->writeAttribute("version", QString::number(ProtocolItem::protocolVersion));
	flushXmlBuffer();
	
	int maxUsers = _server->getMaxUsersPerAddress();
	if ((maxUsers > 0) && (_server->getUsersWithAddress(socket->peerAddress()) >= maxUsers)) {
		sendProtocolItem(new Event_ConnectionClosed("too_many_connections"));
		deleteLater();
	} else
		sendProtocolItem(new Event_ServerMessage(Servatrice::versionString));
	
	server->addClient(this);
}

ServerSocketInterface::~ServerSocketInterface()
{
	logger->logMessage("ServerSocketInterface destructor", this);
	
	prepareDestroy();
	
	flushXmlBuffer();
	delete xmlWriter;
	delete xmlReader;
	delete socket;
	socket = 0;
	delete topLevelItem;
}

void ServerSocketInterface::processProtocolItem(ProtocolItem *item)
{
	CommandContainer *cont = qobject_cast<CommandContainer *>(item);
	if (!cont)
		sendProtocolItem(new ProtocolResponse(cont->getCmdId(), RespInvalidCommand));
	else
		processCommandContainer(cont);
}

void ServerSocketInterface::flushXmlBuffer()
{
	QMutexLocker locker(&xmlBufferMutex);
	if (xmlBuffer.isEmpty())
		return;
	servatrice->incTxBytes(xmlBuffer.size());
	socket->write(xmlBuffer.toUtf8());
	socket->flush();
	xmlBuffer.clear();
}

void ServerSocketInterface::readClient()
{
	QByteArray data = socket->readAll();
	servatrice->incRxBytes(data.size());
	if (!data.contains("<cmd type=\"ping\""))
		logger->logMessage(QString(data), this);
	xmlReader->addData(data);
	
	while (!xmlReader->atEnd()) {
		xmlReader->readNext();
		if (topLevelItem)
			topLevelItem->readElement(xmlReader);
		else if (xmlReader->isStartElement() && (xmlReader->name().toString() == "cockatrice_client_stream")) {
			if (xmlReader->attributes().value("comp").toString().toInt() == 1)
				compressionSupport = true;
			topLevelItem = new TopLevelProtocolItem;
			connect(topLevelItem, SIGNAL(protocolItemReceived(ProtocolItem *)), this, SLOT(processProtocolItem(ProtocolItem *)));
		}
	}
}

void ServerSocketInterface::catchSocketError(QAbstractSocket::SocketError socketError)
{
	qDebug() << "Socket error:" << socketError;
	
	deleteLater();
}

void ServerSocketInterface::sendProtocolItem(ProtocolItem *item, bool deleteItem)
{
	QMutexLocker locker(&xmlBufferMutex);
	
	item->write(xmlWriter);
	if (deleteItem)
		delete item;
	
	emit xmlBufferChanged();
}

int ServerSocketInterface::getUserIdInDB(const QString &name) const
{
	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	query.prepare("select id from " + servatrice->getDbPrefix() + "_users where name = :name");
	query.bindValue(":name", name);
	if (!servatrice->execSqlQuery(query))
		return -1;
	if (!query.next())
		return -1;
	return query.value(0).toInt();
}

ResponseCode ServerSocketInterface::cmdAddToList(Command_AddToList *cmd, CommandContainer *cont)
{
	if (authState != PasswordRight)
		return RespFunctionNotAllowed;
	
	QString list = cmd->getList();
	QString user = cmd->getUserName();
	
	if ((list != "buddy") && (list != "ignore"))
		return RespContextError;
	
	if ((list == "buddy") && buddyList.contains(user))
		return RespContextError;
	if ((list == "ignore") && ignoreList.contains(user))
		return RespContextError;
	
	int id1 = getUserIdInDB(userInfo->getName());
	int id2 = getUserIdInDB(user);
	if (id2 < 0)
		return RespNameNotFound;
	if (id1 == id2)
		return RespContextError;
	
	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	query.prepare("insert into " + servatrice->getDbPrefix() + "_" + list + "list (id_user1, id_user2) values(:id1, :id2)");
	query.bindValue(":id1", id1);
	query.bindValue(":id2", id2);
	if (!servatrice->execSqlQuery(query))
		return RespInternalError;
	
	ServerInfo_User *info = servatrice->getUserData(user);
	if (list == "buddy")
		buddyList.insert(info->getName(), info);
	else if (list == "ignore")
		ignoreList.insert(info->getName(), info);
	
	cont->enqueueItem(new Event_AddToList(list, new ServerInfo_User(info)));
	return RespOk;
}

ResponseCode ServerSocketInterface::cmdRemoveFromList(Command_RemoveFromList *cmd, CommandContainer *cont)
{
	if (authState != PasswordRight)
		return RespFunctionNotAllowed;
	
	QString list = cmd->getList();
	QString user = cmd->getUserName();
	
	if ((list != "buddy") && (list != "ignore"))
		return RespContextError;
	
	if ((list == "buddy") && !buddyList.contains(user))
		return RespContextError;
	if ((list == "ignore") && !ignoreList.contains(user))
		return RespContextError;
	
	int id1 = getUserIdInDB(userInfo->getName());
	int id2 = getUserIdInDB(user);
	if (id2 < 0)
		return RespNameNotFound;
	
	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	query.prepare("delete from " + servatrice->getDbPrefix() + "_" + list + "list where id_user1 = :id1 and id_user2 = :id2");
	query.bindValue(":id1", id1);
	query.bindValue(":id2", id2);
	if (!servatrice->execSqlQuery(query))
		return RespInternalError;
	
	if (list == "buddy") {
		delete buddyList.value(user);
		buddyList.remove(user);
	} else if (list == "ignore") {
		delete ignoreList.value(user);
		ignoreList.remove(user);
	}
	
	cont->enqueueItem(new Event_RemoveFromList(list, user));
	return RespOk;
}

int ServerSocketInterface::getDeckPathId(int basePathId, QStringList path)
{
	if (path.isEmpty())
		return 0;
	if (path[0].isEmpty())
		return 0;
	
	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	query.prepare("select id from " + servatrice->getDbPrefix() + "_decklist_folders where id_parent = :id_parent and name = :name and user = :user");
	query.bindValue(":id_parent", basePathId);
	query.bindValue(":name", path.takeFirst());
	query.bindValue(":user", userInfo->getName());
	if (!servatrice->execSqlQuery(query))
		return -1;
	if (!query.next())
		return -1;
	int id = query.value(0).toInt();
	if (path.isEmpty())
		return id;
	else
		return getDeckPathId(id, path);
}

int ServerSocketInterface::getDeckPathId(const QString &path)
{
	return getDeckPathId(0, path.split("/"));
}

bool ServerSocketInterface::deckListHelper(DeckList_Directory *folder)
{
	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	query.prepare("select id, name from " + servatrice->getDbPrefix() + "_decklist_folders where id_parent = :id_parent and user = :user");
	query.bindValue(":id_parent", folder->getId());
	query.bindValue(":user", userInfo->getName());
	if (!servatrice->execSqlQuery(query))
		return false;
	
	while (query.next()) {
		DeckList_Directory *newFolder = new DeckList_Directory(query.value(1).toString(), query.value(0).toInt());
		folder->appendItem(newFolder);
		if (!deckListHelper(newFolder))
			return false;
	}
	
	query.prepare("select id, name, upload_time from " + servatrice->getDbPrefix() + "_decklist_files where id_folder = :id_folder and user = :user");
	query.bindValue(":id_folder", folder->getId());
	query.bindValue(":user", userInfo->getName());
	if (!servatrice->execSqlQuery(query))
		return false;
	
	while (query.next()) {
		DeckList_File *newFile = new DeckList_File(query.value(1).toString(), query.value(0).toInt(), query.value(2).toDateTime());
		folder->appendItem(newFile);
	}
	
	return true;
}

// CHECK AUTHENTICATION!
// Also check for every function that data belonging to other users cannot be accessed.

ResponseCode ServerSocketInterface::cmdDeckList(Command_DeckList * /*cmd*/, CommandContainer *cont)
{
	if (authState != PasswordRight)
		return RespFunctionNotAllowed;
	
	servatrice->checkSql();
	
	DeckList_Directory *root = new DeckList_Directory(QString());
	QSqlQuery query;
	if (!deckListHelper(root))
		return RespContextError;
	
	ProtocolResponse *resp = new Response_DeckList(cont->getCmdId(), RespOk, root);
	if (getCompressionSupport())
		resp->setCompressed(true);
	cont->setResponse(resp);
	
	return RespNothing;
}

ResponseCode ServerSocketInterface::cmdDeckNewDir(Command_DeckNewDir *cmd, CommandContainer * /*cont*/)
{
	if (authState != PasswordRight)
		return RespFunctionNotAllowed;
	
	servatrice->checkSql();
	
	int folderId = getDeckPathId(cmd->getPath());
	if (folderId == -1)
		return RespNameNotFound;
	
	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	query.prepare("insert into " + servatrice->getDbPrefix() + "_decklist_folders (id_parent, user, name) values(:id_parent, :user, :name)");
	query.bindValue(":id_parent", folderId);
	query.bindValue(":user", userInfo->getName());
	query.bindValue(":name", cmd->getDirName());
	if (!servatrice->execSqlQuery(query))
		return RespContextError;
	return RespOk;
}

void ServerSocketInterface::deckDelDirHelper(int basePathId)
{
	servatrice->checkSql();
	
	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	
	query.prepare("select id from " + servatrice->getDbPrefix() + "_decklist_folders where id_parent = :id_parent");
	query.bindValue(":id_parent", basePathId);
	servatrice->execSqlQuery(query);
	while (query.next())
		deckDelDirHelper(query.value(0).toInt());
	
	query.prepare("delete from " + servatrice->getDbPrefix() + "_decklist_files where id_folder = :id_folder");
	query.bindValue(":id_folder", basePathId);
	servatrice->execSqlQuery(query);
	
	query.prepare("delete from " + servatrice->getDbPrefix() + "_decklist_folders where id = :id");
	query.bindValue(":id", basePathId);
	servatrice->execSqlQuery(query);
}

ResponseCode ServerSocketInterface::cmdDeckDelDir(Command_DeckDelDir *cmd, CommandContainer * /*cont*/)
{
	if (authState != PasswordRight)
		return RespFunctionNotAllowed;
	
	servatrice->checkSql();
	
	int basePathId = getDeckPathId(cmd->getPath());
	if (basePathId == -1)
		return RespNameNotFound;
	deckDelDirHelper(basePathId);
	return RespOk;
}

ResponseCode ServerSocketInterface::cmdDeckDel(Command_DeckDel *cmd, CommandContainer * /*cont*/)
{
	if (authState != PasswordRight)
		return RespFunctionNotAllowed;
	
	servatrice->checkSql();
	
	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	
	query.prepare("select id from " + servatrice->getDbPrefix() + "_decklist_files where id = :id and user = :user");
	query.bindValue(":id", cmd->getDeckId());
	query.bindValue(":user", userInfo->getName());
	servatrice->execSqlQuery(query);
	if (!query.next())
		return RespNameNotFound;
	
	query.prepare("delete from " + servatrice->getDbPrefix() + "_decklist_files where id = :id");
	query.bindValue(":id", cmd->getDeckId());
	servatrice->execSqlQuery(query);
	
	return RespOk;
}

ResponseCode ServerSocketInterface::cmdDeckUpload(Command_DeckUpload *cmd, CommandContainer *cont)
{
	if (authState != PasswordRight)
		return RespFunctionNotAllowed;
	
	servatrice->checkSql();
	
	if (!cmd->getDeck())
		return RespInvalidData;
	int folderId = getDeckPathId(cmd->getPath());
	if (folderId == -1)
		return RespNameNotFound;
	
	QString deckContents;
	QXmlStreamWriter deckWriter(&deckContents);
	deckWriter.writeStartDocument();
	cmd->getDeck()->write(&deckWriter);
	deckWriter.writeEndDocument();
	
	QString deckName = cmd->getDeck()->getName();
	if (deckName.isEmpty())
		deckName = "Unnamed deck";

	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	query.prepare("insert into " + servatrice->getDbPrefix() + "_decklist_files (id_folder, user, name, upload_time, content) values(:id_folder, :user, :name, NOW(), :content)");
	query.bindValue(":id_folder", folderId);
	query.bindValue(":user", userInfo->getName());
	query.bindValue(":name", deckName);
	query.bindValue(":content", deckContents);
	servatrice->execSqlQuery(query);
	
	cont->setResponse(new Response_DeckUpload(cont->getCmdId(), RespOk, new DeckList_File(deckName, query.lastInsertId().toInt(), QDateTime::currentDateTime())));
	return RespNothing;
}

DeckList *ServerSocketInterface::getDeckFromDatabase(int deckId)
{
	servatrice->checkSql();
	
	QMutexLocker locker(&servatrice->dbMutex);
	QSqlQuery query;
	
	query.prepare("select content from " + servatrice->getDbPrefix() + "_decklist_files where id = :id and user = :user");
	query.bindValue(":id", deckId);
	query.bindValue(":user", userInfo->getName());
	servatrice->execSqlQuery(query);
	if (!query.next())
		throw RespNameNotFound;
	
	QXmlStreamReader deckReader(query.value(0).toString());
	DeckList *deck = new DeckList;
	deck->loadFromXml(&deckReader);
	
	return deck;
}

ResponseCode ServerSocketInterface::cmdDeckDownload(Command_DeckDownload *cmd, CommandContainer *cont)
{
	if (authState != PasswordRight)
		return RespFunctionNotAllowed;
	
	DeckList *deck;
	try {
		deck = getDeckFromDatabase(cmd->getDeckId());
	} catch(ResponseCode r) {
		return r;
	}
	cont->setResponse(new Response_DeckDownload(cont->getCmdId(), RespOk, deck));
	return RespNothing;
}

// MODERATOR FUNCTIONS.
// May be called by admins and moderators. Permission is checked by the calling function.

ResponseCode ServerSocketInterface::cmdBanFromServer(Command_BanFromServer *cmd, CommandContainer * /*cont*/)
{
	QString userName = cmd->getUserName();
	QString address = cmd->getAddress();
	int minutes = cmd->getMinutes();
	
	servatrice->dbMutex.lock();
	QSqlQuery query;
	query.prepare("insert into " + servatrice->getDbPrefix() + "_bans (user_name, ip_address, id_admin, time_from, minutes, reason) values(:user_name, :ip_address, :id_admin, NOW(), :minutes, :reason)");
	query.bindValue(":user_name", userName);
	query.bindValue(":ip_address", address);
	query.bindValue(":id_admin", getUserIdInDB(userInfo->getName()));
	query.bindValue(":minutes", minutes);
	query.bindValue(":reason", cmd->getReason() + "\n");
	servatrice->execSqlQuery(query);
	servatrice->dbMutex.unlock();
	
	ServerSocketInterface *user = static_cast<ServerSocketInterface *>(server->getUsers().value(userName));
	if (user) {
		user->sendProtocolItem(new Event_ConnectionClosed("banned"));
		user->deleteLater();
	}
	
	return RespOk;
}

// ADMIN FUNCTIONS.
// Permission is checked by the calling function.

ResponseCode ServerSocketInterface::cmdUpdateServerMessage(Command_UpdateServerMessage * /*cmd*/, CommandContainer * /*cont*/)
{
	servatrice->updateLoginMessage();
	return RespOk;
}

ResponseCode ServerSocketInterface::cmdShutdownServer(Command_ShutdownServer *cmd, CommandContainer * /*cont*/)
{
	servatrice->scheduleShutdown(cmd->getReason(), cmd->getMinutes());
	return RespOk;
}
