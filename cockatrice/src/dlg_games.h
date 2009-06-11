#ifndef DLG_GAMES_H
#define DLG_GAMES_H

#include <QDialog>
#include "gamesmodel.h"
#include "servergame.h"
#include "client.h"

class QPushButton;
class QTreeView;

class DlgGames : public QDialog {
	Q_OBJECT
public:
	DlgGames(Client *_client, QWidget *parent = 0);
private slots:
	void actCreate();
	void actRefresh();
	void actJoin();
	void checkResponse(ServerResponse *response);
private:
	Client *client;
	int msgid;
	
	QTreeView *gameListView;
	GamesModel *gameListModel;
	QPushButton *refreshButton, *createButton, *joinButton;
};

#endif
 