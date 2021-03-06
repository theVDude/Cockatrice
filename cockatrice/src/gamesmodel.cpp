#include "gamesmodel.h"
#include "protocol_datastructures.h"

GamesModel::GamesModel(const QMap<int, QString> &_rooms, const QMap<int, GameTypeMap> &_gameTypes, QObject *parent)
	: QAbstractTableModel(parent), rooms(_rooms), gameTypes(_gameTypes)
{
}

GamesModel::~GamesModel()
{
	if (!gameList.isEmpty()) {
		beginRemoveRows(QModelIndex(), 0, gameList.size() - 1);
		for (int i = 0; i < gameList.size(); ++i)
			delete gameList[i];
		gameList.clear();
		endRemoveRows();
	}
}

QVariant GamesModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if (role == Qt::UserRole)
		return index.row();
	if (role != Qt::DisplayRole)
		return QVariant();
	if ((index.row() >= gameList.size()) || (index.column() >= columnCount()))
		return QVariant();
	
	ServerInfo_Game *g = gameList[index.row()];
	switch (index.column()) {
		case 0: return g->getDescription();
		case 1: return rooms.value(g->getRoomId());
		case 2: return g->getCreatorInfo()->getName();
		case 3: {
			QStringList result;
			QList<GameTypeId *> gameTypeList = g->getGameTypes();
			GameTypeMap gameTypeMap = gameTypes.value(g->getRoomId());
			for (int i = 0; i < gameTypeList.size(); ++i)
				result.append(gameTypeMap.value(gameTypeList[i]->getData()));
			return result.join(", ");
		}
		case 4: return g->getHasPassword() ? (g->getSpectatorsNeedPassword() ? tr("yes") : tr("yes, free for spectators")) : tr("no");
		case 5: {
			QStringList result;
			if (g->getOnlyBuddies())
				result.append(tr("buddies only"));
			if (g->getOnlyRegistered())
				result.append(tr("reg. users only"));
			return result.join(", ");
		}
		case 6: return QString("%1/%2").arg(g->getPlayerCount()).arg(g->getMaxPlayers());
		case 7: return g->getSpectatorsAllowed() ? QVariant(g->getSpectatorCount()) : QVariant(tr("not allowed"));
		default: return QVariant();
	}
}

QVariant GamesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal))
		return QVariant();
	switch (section) {
		case 0: return tr("Description");
		case 1: return tr("Room");
		case 2: return tr("Creator");
		case 3: return tr("Game type");
		case 4: return tr("Password");
		case 5: return tr("Restrictions");
		case 6: return tr("Players");
		case 7: return tr("Spectators");
		default: return QVariant();
	}
}

ServerInfo_Game *GamesModel::getGame(int row)
{
	Q_ASSERT(row < gameList.size());
	return gameList[row];
}

void GamesModel::updateGameList(ServerInfo_Game *_game)
{
	QList<GameTypeId *> gameTypeList, oldGameTypeList = _game->getGameTypes();
	for (int i = 0; i < oldGameTypeList.size(); ++i)
		gameTypeList.append(new GameTypeId(oldGameTypeList[i]->getData()));
	
	ServerInfo_Game *game = new ServerInfo_Game(_game->getRoomId(), _game->getGameId(), _game->getDescription(), _game->getHasPassword(), _game->getPlayerCount(), _game->getMaxPlayers(), _game->getStarted(), gameTypeList, new ServerInfo_User(_game->getCreatorInfo()), _game->getOnlyBuddies(), _game->getOnlyRegistered(), _game->getSpectatorsAllowed(), _game->getSpectatorsNeedPassword(), _game->getSpectatorCount());
	for (int i = 0; i < gameList.size(); i++)
		if (gameList[i]->getGameId() == game->getGameId()) {
			if (game->getPlayerCount() == 0) {
				beginRemoveRows(QModelIndex(), i, i);
				delete gameList.takeAt(i);
				endRemoveRows();
			} else {
				delete gameList[i];
				gameList[i] = game;
				emit dataChanged(index(i, 0), index(i, 7));
			}
			return;
		}
	if (game->getPlayerCount() == 0)
		return;
	beginInsertRows(QModelIndex(), gameList.size(), gameList.size());
	gameList.append(game);
	endInsertRows();
}

GamesProxyModel::GamesProxyModel(QObject *parent, ServerInfo_User *_ownUser)
	: QSortFilterProxyModel(parent), ownUser(_ownUser), unavailableGamesVisible(false)
{
	setDynamicSortFilter(true);
}

void GamesProxyModel::setUnavailableGamesVisible(bool _unavailableGamesVisible)
{
	unavailableGamesVisible = _unavailableGamesVisible;
	invalidateFilter();
}

bool GamesProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
	GamesModel *model = qobject_cast<GamesModel *>(sourceModel());
	if (!model)
		return false;
	
	ServerInfo_Game *game = model->getGame(sourceRow);
	if (!unavailableGamesVisible) {
		if (game->getPlayerCount() == game->getMaxPlayers())
			return false;
		if (game->getStarted())
			return false;
		if (!(ownUser->getUserLevel() & ServerInfo_User::IsRegistered))
			if (game->getOnlyRegistered())
				return false;
	}
	
	return true;
}
