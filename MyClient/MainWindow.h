#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_
#include <QWidget>
#include <QDir>
#include <QList>
#include <qstandarditemmodel.h>
#include <QWebSocket>
#include <QModelIndexList>
#include <QItemSelectionModel>
namespace Ui
{
	class MainWindow;
};
class mainWindow :public QWidget
{
	Q_OBJECT
public:
	enum ProgressBar
	{
		SendState,
		ReceiveState
	};
	mainWindow(QWidget* parent = nullptr);
	~mainWindow();
	void init();
	void updateLocalDir(const QString& Path);
	void updateRemoteDir(const QString& Path, const QJsonArray& array);
	qint64 SendBinaryMessage(const QByteArray& data);
	qint64 SendTextMessage(const QString& message);
	QString fileType(QFileInfo* info);
	void SendJson(const QString& jsonData);
	QByteArray getUpFiles(const QString& path);
	QByteArray getFiles(const QString& path);
private:
	void sendFile(const QString& path,const QString& fileName);
	void sendFolder(const QString& path);
	void updateProgressBar(const QString& filePath,const qint64 currentSendSize,const qint64 totalSendSize,const mainWindow::ProgressBar State);
signals:
	void s1();
public slots:
	void onbtn_Backclicked();
	void onTbw_LocalFiledoubleClicked(const QModelIndex& index);
	void onbtn_Freshclicked();
	void onbtn_NewFileclicked();
	void onCox_LocalFileCoxcurrentIndexChanged(int index);
	void onCox_LocalFilecurrentTextChanged(const QString& text);
	void ons1();

	void onbtn_RemoteFileBackclicked();
	void onTbw_Tbw_RemoteFiledoubleClicked(const QModelIndex& index);
	void onbtn_RemoteFIleFreshclicked();
	void onbtn_RemoteNewFileclicked();
	void onCox_RemoteFileCoxcurrentIndexChanged(int index);
	void onCox_RemoteFilecurrentTextChanged(const QString& text);
	void onEditFinish(QWidget* editer);

	void onBinaryMessageReceived(const QByteArray& message);
	void onTextMessageReceived(const QString& message);
	
	void onbtn_Transferclicked();
	void onbtn_StartAll();
	void onbtn_PauseAll();
	void onbtn_CanselAll();

public:
	Ui::MainWindow* ui;

	QStandardItemModel LocalModel;
	QDir LocalFileDir;

	QStandardItemModel RemoteModel;
	QDir RemoteFileDir;

	QStandardItemModel TransferModel;
	QDir TransferFileDir;
	int transferState;

	QWebSocket* m_socket;

	int ChangeIndex{};

	quint64 m_totalReceiveSize{};
	qint64 m_CurrentReceiveSize{};
	QString m_fileName;
	QString m_fileType;
	QString m_transferPath;
	QString m_transfertoPath;
	QFile* m_Receivefile{};

	qint64 m_totalSendSize{};
	qint64 m_CurrentSendSize{};
	QString m_SendfileName;
	QString m_SendfileType;
	QString m_SendtransferPath;
	QString m_SendtransfertoPath;
	QFile* m_Sendfile{};
};

#endif //!MAINWINDOW_H_