#include <QTimer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QCryptographicHash>
#include "remoteclient.h"
#include "protocol.h"
#include "protocol_items.h"

RemoteClient::RemoteClient(QObject *parent)
	: AbstractClient(parent), timeRunning(0), lastDataReceived(0), topLevelItem(0)
{
	ProtocolItem::initializeHash();
	
	timer = new QTimer(this);
	timer->setInterval(1000);
	connect(timer, SIGNAL(timeout()), this, SLOT(ping()));

	socket = new QTcpSocket(this);
	connect(socket, SIGNAL(connected()), this, SLOT(slotConnected()));
	connect(socket, SIGNAL(readyRead()), this, SLOT(readData()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotSocketError(QAbstractSocket::SocketError)));
	
	xmlReader = new QXmlStreamReader;
	xmlWriter = new QXmlStreamWriter;
	xmlWriter->setDevice(socket);
}

RemoteClient::~RemoteClient()
{
	disconnectFromServer();
}

void RemoteClient::slotSocketError(QAbstractSocket::SocketError /*error*/)
{
	QString errorString = socket->errorString();
	disconnectFromServer();
	emit socketError(errorString);
}

void RemoteClient::slotConnected()
{
	timeRunning = lastDataReceived = 0;
	timer->start();
	setStatus(StatusAwaitingWelcome);
}

void RemoteClient::loginResponse(ProtocolResponse *response)
{
	if (response->getResponseCode() == RespOk) {
		Response_Login *resp = qobject_cast<Response_Login *>(response);
		if (!resp) {
			disconnectFromServer();
			return;
		}
		setStatus(StatusLoggedIn);
		emit userInfoChanged(resp->getUserInfo());
		emit buddyListReceived(resp->getBuddyList());
		emit ignoreListReceived(resp->getIgnoreList());
	} else {
		emit serverError(response->getResponseCode());
		setStatus(StatusDisconnecting);
	}
}

void RemoteClient::readData()
{
	QByteArray data = socket->readAll();
	qDebug() << data;
	xmlReader->addData(data);
	lastDataReceived = timeRunning;

	while (!xmlReader->atEnd()) {
		xmlReader->readNext();
		if (topLevelItem)
			topLevelItem->readElement(xmlReader);
		else if (xmlReader->isStartElement() && (xmlReader->name().toString() == "cockatrice_server_stream")) {
			int serverVersion = xmlReader->attributes().value("version").toString().toInt();
			if (serverVersion != ProtocolItem::protocolVersion) {
				emit protocolVersionMismatch(ProtocolItem::protocolVersion, serverVersion);
				disconnectFromServer();
				return;
			}
			xmlWriter->writeStartDocument();
			xmlWriter->writeStartElement("cockatrice_client_stream");
			xmlWriter->writeAttribute("version", QString::number(ProtocolItem::protocolVersion));
			xmlWriter->writeAttribute("comp", "1");
			
			topLevelItem = new TopLevelProtocolItem;
			connect(topLevelItem, SIGNAL(protocolItemReceived(ProtocolItem *)), this, SLOT(processProtocolItem(ProtocolItem *)));
			
			setStatus(StatusLoggingIn);
			Command_Login *cmdLogin = new Command_Login(userName, password);
			connect(cmdLogin, SIGNAL(finished(ProtocolResponse *)), this, SLOT(loginResponse(ProtocolResponse *)));
			sendCommand(cmdLogin);
		}
	}
	if (status == StatusDisconnecting)
		disconnectFromServer();
}

void RemoteClient::sendCommandContainer(CommandContainer *cont)
{
	cont->write(xmlWriter);
	pendingCommands.insert(cont->getCmdId(), cont);
}

void RemoteClient::connectToServer(const QString &hostname, unsigned int port, const QString &_userName, const QString &_password)
{
	disconnectFromServer();
	
	userName = _userName;
	password = _password;
	socket->connectToHost(hostname, port);
	setStatus(StatusConnecting);
}

void RemoteClient::disconnectFromServer()
{
	delete topLevelItem;
	topLevelItem = 0;
	
	xmlReader->clear();
	
	timer->stop();

	QList<CommandContainer *> pc = pendingCommands.values();
	for (int i = 0; i < pc.size(); i++)
		delete pc[i];
	pendingCommands.clear();

	setStatus(StatusDisconnected);
	socket->close();
}

void RemoteClient::ping()
{
	QMutableMapIterator<int, CommandContainer *> i(pendingCommands);
	while (i.hasNext())
		if (i.next().value()->tick() > maxTimeout) {
			CommandContainer *cont = i.value();
			i.remove();
			cont->deleteLater();
		}
	
	int maxTime = timeRunning - lastDataReceived;
	emit maxPingTime(maxTime, maxTimeout);
	if (maxTime >= maxTimeout) {
		disconnectFromServer();
		emit serverTimeout();
	} else {
		sendCommand(new Command_Ping);
		++timeRunning;
	}
}
