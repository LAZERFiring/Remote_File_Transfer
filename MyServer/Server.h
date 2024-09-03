#ifndef SERVER_H_
#define SERVER_H_
#include <QWebSocketServer>
#include <QWebSocket>
#include <QObject>
class Serve : public QObject
{
public:
	Serve(QObject* parent=nullptr);
	void init();
public:
	QWebSocketServer* server{};
	QList<QWebSocket*> clients;
};

#endif //!SERVER_H_