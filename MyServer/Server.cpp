#include "Server.h"
#include <QHostAddress>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
static int connectionSum = 0;
Serve::Serve(QObject* parent)
	:QObject(parent)
{
	init();
}

void Serve::init()
{
	this->server = new QWebSocketServer("Server", QWebSocketServer::NonSecureMode, this);
	server->listen(QHostAddress::LocalHost, 8888);
	connect(server, &QWebSocketServer::newConnection, [=]()
		{
			qDebug() << "Has new connection!Current Connects sum:"<<++connectionSum;
			while (server->hasPendingConnections())
			{
				auto client = server->nextPendingConnection();
				clients.append(client);
				QJsonDocument jdom;
				QJsonArray jarray;
				QJsonObject jboj;
				jboj.insert("type", "currentConClients");
				for (auto client : clients)
				{
					jarray.append(client->peerAddress().toString());
				}
				jboj.insert("ClientsList", jarray);
				jdom.setObject(jboj);
				for (auto client : clients)
				{
					qInfo() << jdom;
					client->sendTextMessage(jdom.toJson(QJsonDocument::Compact));
				}
				connect(client, &QWebSocket::binaryMessageReceived, this, [=](const QByteArray& message)
					{
						qDebug() << "binary message received! message:" << message;
						auto c = dynamic_cast<QWebSocket*>(sender());
						for (auto client : clients)
						{
							if (c != client)
								client->sendBinaryMessage(message);
						}
					});
				connect(client, &QWebSocket::textMessageReceived, this, [=](const QString& message)
					{
						qDebug() << "text Message received! message:" << message;
						auto c = dynamic_cast<QWebSocket*>(sender());
						for (auto client : clients)
						{
							if(c != client)
								client->sendTextMessage(message);
						}
					});
			}
		});
	connect(server, &QWebSocketServer::acceptError, [=](QAbstractSocket::SocketError socketError)
		{
			qDebug() << "Has error when new conection in:" << socketError;
		});
	
}