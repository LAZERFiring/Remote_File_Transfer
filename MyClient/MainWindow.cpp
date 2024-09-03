#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QStandardPaths>
#include <QDirIterator>
#include <qfileinfo.h>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QAbstractItemDelegate>
#include <QMessageBox>
#include <QTimer>
#include <QHeaderView>
#include <QCoreApplication>
#define FILE_BACK "FileBack"
#define FILE_INITIAL_PATH "FileInitialPath"
#define FILE_DOUBLE_CLICKED "FiledoubleClicked"
#define FILE_FREASH "FileFresh"
#define FILE_NEW_FILE "NewFile"
#define COX_INDEX_CHANGE "CoxIndexChange"
#define RESPONSE_FILE_LIST "ResponseFileList"
#define EDIT_FINISH "EditFinish"
#define PUT "Put"
#define REMOTE_PUT "RemotePut"
#define CLIENTS "currentConClients"
inline quint64 operator""B(quint64 bytes)
{
	return bytes;
}
inline quint64 operator""KB(quint64 bytes)
{
	return bytes * 1024B;
}
inline quint64 operator""MB(quint64 bytes)
{
	return bytes * 1024KB;
}
inline quint64 operator""GB(quint64 bytes)
{
	return bytes * 1024MB;
}
mainWindow::mainWindow(QWidget* parent)
	:QWidget(parent)
	,ui(new Ui::MainWindow)
	,m_socket(new QWebSocket("LAZER"))
{
	ui->setupUi(this);
	init();
}

mainWindow::~mainWindow()
{
	delete m_socket;
}

void mainWindow::init()
{
	setWindowTitle("FileTransfer");
	//setWindowFlags(Qt::FramelessWindowHint);
	//与服务器建立连接
	m_socket->open(QUrl("ws://127.0.0.1:8888"));
	connect(m_socket, &QWebSocket::connected, [=]()
		{
			qInfo() << "Connect to Server!";
			connect(m_socket, &QWebSocket::textMessageReceived, this, &mainWindow::onTextMessageReceived);
			connect(m_socket, &QWebSocket::binaryMessageReceived, this, &mainWindow::onBinaryMessageReceived);
		});
	connect(m_socket, &QWebSocket::errorOccurred, [](QAbstractSocket::SocketError error)
		{
			qInfo() << "error occured! :" << error;
		});
	connect(m_socket, &QWebSocket::disconnected, []()
		{
			qInfo() << "dis connected!";
		});
	//进度条初始化
	ui->Pb_Progressbar->setRange(0, 100);
	ui->Pb_Progressbar->setValue(0);
	ui->Pb_SendFileName->setText("Stand by...");
	//本地设备
	ui->Cox_LocalFileCox->setEditable(true);
	ui->Tbw_LocalFile->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->Tbw_LocalFile->setSelectionBehavior(QAbstractItemView::SelectRows);
	QDir dirLocal(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
	ui->Cox_LocalFileCox->setCurrentText(dirLocal.path());
	updateLocalDir(dirLocal.absolutePath());
	LocalModel.setHeaderData(0, Qt::Horizontal, QString::fromUtf8("Name"));
	LocalModel.setHeaderData(1, Qt::Horizontal, "Size");
	LocalModel.setHeaderData(2, Qt::Horizontal, "Time");
	LocalModel.setHeaderData(3, Qt::Horizontal, "Type");
	//LocalModel.setHeaderData(1, Qt::Horizontal,"")
	connect(ui->btn_Back, &QPushButton::clicked, this, &mainWindow::onbtn_Backclicked);
	connect(ui->Tbw_LocalFile, &QTableView::doubleClicked,this, &mainWindow::onTbw_LocalFiledoubleClicked);
	connect(ui->btn_Fresh, &QPushButton::clicked, this, &mainWindow::onbtn_Freshclicked);
	connect(ui->btn_NewFile, &QPushButton::clicked, this, &mainWindow::onbtn_NewFileclicked);
	connect(ui->Cox_LocalFileCox, &QComboBox::currentIndexChanged, this, &mainWindow::onCox_LocalFileCoxcurrentIndexChanged);
	connect(ui->Cox_LocalFileCox, &QComboBox::currentTextChanged, this, &mainWindow::onCox_RemoteFilecurrentTextChanged);
	//远程设备
	ui->Cox_RemoteFileCox->setEditable(true);
	ui->Tbw_RemoteFile->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->Tbw_RemoteFile->setSelectionBehavior(QAbstractItemView::SelectRows);
	RemoteModel.setHeaderData(0, Qt::Horizontal, QString::fromUtf8("Name"));
	RemoteModel.setHeaderData(1, Qt::Horizontal, "Size");
	RemoteModel.setHeaderData(2, Qt::Horizontal, "Time");
	RemoteModel.setHeaderData(3, Qt::Horizontal, "Type");
	//QDir dirRemote(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
	//ui->Cox_RemoteFileCox->setCurrentText(dirRemote.path());
	//updateRemoteDir(dirRemote.absolutePath());
	//QJsonDocument jdom;
	//QJsonObject jobj;
	//jobj.insert("type", FILE_INITIAL_PATH);
	//jdom.setObject(jobj);
	//SendJson(jdom.toJson());
	connect(ui->btn_RemoteFileBack, &QPushButton::clicked, this, &mainWindow::onbtn_RemoteFileBackclicked);
	connect(ui->Tbw_RemoteFile, &QTableView::doubleClicked, this, &mainWindow::onTbw_Tbw_RemoteFiledoubleClicked);
	connect(ui->btn_RemoteFIleFresh, &QPushButton::clicked, this, &mainWindow::onbtn_RemoteFIleFreshclicked);
	connect(ui->btn_RemoteNewFile, &QPushButton::clicked, this, &mainWindow::onbtn_RemoteNewFileclicked);
	connect(ui->Cox_RemoteFileCox, &QComboBox::currentIndexChanged, this, &mainWindow::onCox_RemoteFileCoxcurrentIndexChanged);
	connect(ui->Tbw_RemoteFile->itemDelegate(), &QAbstractItemDelegate::commitData, this, &mainWindow::onEditFinish);
	//传输
	connect(ui->btn_Transfer, &QPushButton::clicked, this, &mainWindow::onbtn_Transferclicked);
	connect(ui->btn_StartAll, &QPushButton::clicked, this, &mainWindow::onbtn_StartAll);
	connect(ui->btn_CanselAll, &QPushButton::clicked, this, &mainWindow::onbtn_CanselAll);
	connect(ui->btn_PauseAll, &QPushButton::clicked, this, &mainWindow::onbtn_PauseAll);
}

void mainWindow::updateLocalDir(const QString& Path)
{
	LocalModel.clear();
	LocalFileDir.setPath(Path);
	if (ui->Cox_LocalFileCox->findText(Path) == -1)
	{
		ui->Cox_LocalFileCox->addItem(Path);
		ui->Cox_LocalFileCox->setCurrentIndex(ui->Cox_LocalFileCox->count() - 1);
	}
	else
	{
		ui->Cox_LocalFileCox->setCurrentIndex(ui->Cox_LocalFileCox->findText(Path));
	}
	if (ui->Cox_LocalFileCox->count() > 10)
	{
		ui->Cox_LocalFileCox->removeItem(0);
	}
	QDirIterator it(Path, QDir::NoDotAndDotDot | QDir::AllEntries);
	while (it.hasNext())
	{
		QFileInfo info = it.nextFileInfo();
		QList<QStandardItem*> file;
		file.append(new QStandardItem(it.fileName()));
		file.append(new QStandardItem(QString::number(info.size())));
		file.append(new QStandardItem(info.fileTime(QFile::FileModificationTime).toString("yyyy/MM/dd hh::mm")));
		file.append(new QStandardItem(fileType(&info)));
		LocalModel.appendRow(file);
	}
	ui->Tbw_LocalFile->setModel(&LocalModel);
}

void mainWindow::updateRemoteDir(const QString& Path,const QJsonArray& array)
{
	RemoteModel.clear();
	RemoteFileDir.setPath(Path);
	int index = ui->Cox_RemoteFileCox->findText(Path);
	if (index == -1)
	{
		ui->Cox_RemoteFileCox->addItem(Path);
		ui->Cox_RemoteFileCox->setCurrentIndex(ui->Cox_RemoteFileCox->count() - 1);
	}
	else
	{
		ui->Cox_RemoteFileCox->setCurrentIndex(index);
	}
	for (int i = 0; i < array.size(); i++)
	{
		QList<QStandardItem*> file;
		auto jobj = array.at(i).toObject();
		file.append(new QStandardItem(jobj.value("fileName").toString()));
		file.append(new QStandardItem(jobj.value("fileSize").toInt()));
		file.append(new QStandardItem(jobj.value("fileLastModifyTime").toString()));
		file.append(new QStandardItem(jobj.value("fileType").toString()));
		RemoteModel.appendRow(file);
		qInfo() << jobj.value("fileName").toString();
	}
	ui->Tbw_RemoteFile->setModel(&RemoteModel);
}

qint64 mainWindow::SendBinaryMessage(const QByteArray& data)
{
	return m_socket->sendBinaryMessage(data);;
}

qint64 mainWindow::SendTextMessage(const QString& message)
{
	return m_socket->sendTextMessage(message);
}

QString mainWindow::fileType(QFileInfo* info)
{
	if (info->isAlias()) { return "Renameed"; }      //mac
	if (info->isBundle()) { return "bundle"; }     //mac
	if (info->isDir()) { return "folder"; }
	if (info->isExecutable()) { return "exe"; }
	if (info->isFile()) { return "file"; }
	if (info->isHidden()) { return "hidenFile"; }
	if (info->isJunction()) { return "lnk"; }
	if (info->isShortcut()) { return "lnk"; }
	if (info->isSymLink()) { return "lnk"; }
	if (info->isSymbolicLink()) { return "lnk"; }
	return "unknown";
}

void mainWindow::SendJson(const QString& jsonData)
{
	SendTextMessage(jsonData);
}

QByteArray mainWindow::getUpFiles(const QString& path)
{
	QDir dir(path);
	if (dir.cdUp())
	{
		qInfo() << dir;
		QDirIterator it(dir);
		QJsonDocument Jdom;
		QJsonObject Jobj;
		QJsonArray Jarray;
		Jobj.insert("type", RESPONSE_FILE_LIST);
		Jobj.insert("path", dir.canonicalPath());
		while (it.hasNext())
		{
			QFileInfo fileinfo = it.nextFileInfo();
			QJsonObject jobj;
			jobj.insert("fileName", fileinfo.fileName());
			jobj.insert("fileSize", fileinfo.size());
			jobj.insert("fileLastModifyTime", fileinfo.lastModified().toString());
			jobj.insert("fileType", fileType(&fileinfo));
			Jarray.append(jobj);
		}
		Jobj.insert("ResponseFileList", Jarray);
		Jdom.setObject(Jobj);
		return Jdom.toJson(QJsonDocument::Compact);
	}
	else
	{
		printf("no up path!\n");
		return QByteArray();
	}
}

QByteArray mainWindow::getFiles(const QString& path)
{
	QDir dir(path);
	if (dir.exists())
	{
		qInfo() << dir;
		QDirIterator it(dir);
		QJsonDocument Jdom;
		QJsonObject Jobj;
		QJsonArray Jarray;
		Jobj.insert("type", RESPONSE_FILE_LIST);
		Jobj.insert("path", dir.absolutePath());
		while (it.hasNext())
		{
			QFileInfo fileinfo = it.nextFileInfo();
			QJsonObject jobj;
			jobj.insert("fileName", fileinfo.fileName());
			jobj.insert("fileSize", fileinfo.size());
			jobj.insert("fileLastModifyTime", fileinfo.lastModified().toString());
			jobj.insert("fileType", fileType(&fileinfo));
			Jarray.append(jobj);
		}
		Jobj.insert("ResponseFileList", Jarray);
		Jdom.setObject(Jobj);
		return Jdom.toJson(QJsonDocument::Compact);
	}
	else
	{
		printf("no up path!\n");
		return QByteArray();
	}
}

void mainWindow::onbtn_Backclicked()
{
	if (LocalFileDir.cdUp())
	{
		updateLocalDir(LocalFileDir.absolutePath());
	}
}

void mainWindow::onTbw_LocalFiledoubleClicked(const QModelIndex& index)
{
	QString fileName = LocalModel.data(LocalModel.index(index.row(), 0)).toString();
	updateLocalDir(LocalFileDir.absolutePath() + "/" + fileName);
}

void mainWindow::onbtn_Freshclicked()
{
	updateLocalDir(LocalFileDir.absolutePath());
}

void mainWindow::onbtn_NewFileclicked()
{
	QString folder = "New Folder";
	QDir path(ui->Cox_LocalFileCox->currentText());
	if (!path.mkdir(folder))
	{
		qDebug() << "create new folder failed!";
	}
	QList<QStandardItem*> items;
	auto item = new QStandardItem(folder);
	//item->setIcon(m_fileIconProvider.icon(QFileIconProvider::IconType::Folder));

	items.append(item);
	items.append(new QStandardItem(""));
	items.append(new QStandardItem());
	items.append(new QStandardItem("folder"));

	LocalModel.appendRow(items);

	ui->Tbw_LocalFile->scrollToBottom();
	ui->Tbw_LocalFile->edit(item->index());
}

void mainWindow::onCox_LocalFileCoxcurrentIndexChanged(int index)
{
	QDir dir(ui->Cox_LocalFileCox->itemText(index));
	updateLocalDir(dir.absolutePath());
}

void mainWindow::onCox_LocalFilecurrentTextChanged(const QString& text)
{
	int index = ui->Cox_LocalFileCox->findText(text);
	if (index == -1)
	{
		ui->Cox_LocalFileCox->addItem(text);
		ui->Cox_LocalFileCox->setCurrentIndex(ui->Cox_LocalFileCox->count() - 1);
	}
	else
	{
		ui->Cox_LocalFileCox->setCurrentIndex(index);
	}
	if (ui->Cox_LocalFileCox->count() > 10)
	{
		ui->Cox_LocalFileCox->removeItem(0);
	}
}

void mainWindow::ons1()
{
	qInfo() << "ons1";
}

void mainWindow::onbtn_RemoteFileBackclicked()
{
	QJsonDocument jdom;
	QJsonObject jobj;
	jobj.insert("type", FILE_BACK);
	jobj.insert("CurrentRemotePath", ui->Cox_RemoteFileCox->currentText());
	jdom.setObject(jobj);
	SendJson(jdom.toJson(QJsonDocument::Compact));
}

void mainWindow::onTbw_Tbw_RemoteFiledoubleClicked(const QModelIndex& index)
{
	QString filename = RemoteModel.data(index).toString();
	QJsonDocument jdom;
	QJsonObject jobj{ {"type",QJsonValue("FiledoubleClicked")},{"path",QJsonValue(QString(ui->Cox_RemoteFileCox->currentText()+"/"+filename))}};
	jdom.setObject(jobj);
	SendJson(jdom.toJson(QJsonDocument::Compact));
}

void mainWindow::onbtn_RemoteFIleFreshclicked()
{
	QJsonDocument jdom;
	QJsonObject jobj;
	jobj.insert("type", "FileFresh");
	jobj.insert("path", ui->Cox_RemoteFileCox->currentText());
	jdom.setObject(jobj);
	SendJson(jdom.toJson(QJsonDocument::Compact));
}

void mainWindow::onbtn_RemoteNewFileclicked()
{
	QJsonDocument jdom;
	QJsonObject jobj;
	jobj.insert("type", "NewFile");
	jobj.insert("path", ui->Cox_RemoteFileCox->currentText());
	jdom.setObject(jobj);
	SendJson(jdom.toJson(QJsonDocument::Compact));
}

void mainWindow::onCox_RemoteFileCoxcurrentIndexChanged(int index)
{
	QJsonDocument jdom;
	QJsonObject jobj;
	jobj.insert("type", COX_INDEX_CHANGE);
	jobj.insert("path", ui->Cox_RemoteFileCox->currentText());
	qInfo() << "path of coxchanged:" << ui->Cox_RemoteFileCox->currentText();
	jdom.setObject(jobj);
	SendJson(jdom.toJson(QJsonDocument::Compact));
}

void mainWindow::onCox_RemoteFilecurrentTextChanged(const QString& text)
{
}

void mainWindow::onEditFinish(QWidget* editer)
{
	auto NewData = RemoteModel.data(RemoteModel.index(ChangeIndex, 0));
	QJsonDocument jdom;
	QJsonObject jobj;
	jobj.insert("type", EDIT_FINISH);
	jobj.insert("path", ui->Cox_RemoteFileCox->currentText());
	qInfo() << NewData;
	if (NewData.toString() == "New File")
	{
		jobj.insert("NewFileName", NewData.toString());
		RemoteModel.removeRow(ChangeIndex);
		ui->Tbw_RemoteFile->setModel(&RemoteModel);
		jdom.setObject(jobj);
		SendTextMessage(jdom.toJson(QJsonDocument::Compact));
	}
	else
	{
		jobj.insert("NewFileName", NewData.toString());
		jdom.setObject(jobj);
		SendTextMessage(jdom.toJson(QJsonDocument::Compact));
	}
}

void mainWindow::onBinaryMessageReceived(const QByteArray& message)
{
	if (!m_Receivefile->open(QIODevice::WriteOnly | QIODevice::Append))
	{
		qDebug() << "417file open failed!";
	}
	qInfo() << "onBinaryMessageReceived! m_CurrentReceiveSize:"<< m_CurrentReceiveSize;
	m_Receivefile->write(message);
	updateProgressBar(m_transfertoPath, m_CurrentReceiveSize, m_totalReceiveSize,mainWindow::ReceiveState);
	if (m_CurrentReceiveSize >= m_totalReceiveSize)
	{
		m_Receivefile->close();
		m_Receivefile->deleteLater();
		m_CurrentReceiveSize = 0;
		updateLocalDir(ui->Cox_LocalFileCox->currentText());
		onbtn_RemoteFIleFreshclicked();
	}
}

void mainWindow::onTextMessageReceived(const QString& message)
{
	QJsonParseError err;
	auto jdom = QJsonDocument::fromJson(message.toUtf8(), &err);
	if (err.error != QJsonParseError::NoError)
	{
		qWarning() << "json parse error," << err.errorString();
		return;
	}
	//if (jdom["type"].toString() == FILE_INITIAL_PATH)
//{
//	QDir desktopPath(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
//	SendTextMessage(getFiles(desktopPath.absolutePath()));
//}
	if (jdom["type"].toString() == FILE_BACK)
	{
		SendTextMessage(getUpFiles(jdom.object().value("CurrentRemotePath").toString()));
	}
	if (jdom["type"].toString() == FILE_DOUBLE_CLICKED)
	{
		qInfo() << jdom["path"].toString();
		SendTextMessage(getFiles(jdom["path"].toString()));
	}
	if (jdom["type"].toString() == FILE_FREASH)
	{
		SendTextMessage(getFiles(jdom["path"].toString()));
	}
	if (jdom["type"].toString() == FILE_NEW_FILE)
	{
		QDir dir(jdom["path"].toString());
		qInfo() << "360:" << jdom["path"].toString();
		if (dir.mkdir("New File"))
		{
			qInfo() << "mkdir successful!";
			QJsonDocument Jdom = QJsonDocument::fromJson(getFiles(jdom["path"].toString()));
			QJsonObject jobj = Jdom.object();
			jobj.insert("NewFile", QJsonValue(true));
			QDirIterator it(dir);
			int index = 0;
			while (it.hasNext())
			{
				QFileInfo fileinfo = it.nextFileInfo();
				if (fileinfo.fileName() == "New File")
				{
					break;
				}
				index++;
			}
			qInfo() << index;
			jobj.insert("NewFileIndex", index);
			Jdom.setObject(jobj);
			SendTextMessage(Jdom.toJson(QJsonDocument::Compact));
		}
		else
		{
			qInfo() << "Create new file failed!";
		}
	}
	if (jdom["type"].toString() == COX_INDEX_CHANGE)
	{
		QString path = jdom["path"].toString();
		SendJson(getFiles(path));
	}
	if (jdom["type"].toString() == EDIT_FINISH)
	{
		if (jdom["NewFileName"].toString() == "New File")
		{
			QDir dir(jdom["path"].toString());
			qInfo() << dir;
			dir.rmdir("New File");
		}
		else
		{
			QString oldFolderPath = jdom["path"].toString() + "/New File";
			QString newFolderPath = jdom["path"].toString() + "/" + jdom["NewFileName"].toString();
			QDir dir;
			if (!dir.rename(oldFolderPath, newFolderPath))
			{
				qInfo() << "rename false";
			}
		}
	}
	if (jdom["type"].toString() == RESPONSE_FILE_LIST)
	{
		auto Array = jdom["ResponseFileList"].toArray();
		updateRemoteDir(jdom["path"].toString(), Array);
		RemoteModel.setHeaderData(0, Qt::Horizontal, QString::fromUtf8("Name"));
		RemoteModel.setHeaderData(1, Qt::Horizontal, "Size");
		RemoteModel.setHeaderData(2, Qt::Horizontal, "Time");
		RemoteModel.setHeaderData(3, Qt::Horizontal, "Type");
		if (!jdom["NewFile"].isNull() && (jdom["NewFile"].toBool() == true))
		{
			ChangeIndex = jdom["NewFileIndex"].toInt();
			QModelIndex index = RemoteModel.index(ChangeIndex, 0);
			ui->Tbw_RemoteFile->edit(index);
		}
	}
	if (jdom["type"].toString() == PUT)
	{
		m_totalReceiveSize = jdom["fileSize"].toInteger();
		m_fileName = jdom["fileName"].toString();
		m_fileType = jdom["filetype"].toString();
		m_transferPath = jdom["transferPath"].toString();
		m_transfertoPath = jdom["transfertoPath"].toString();
		m_CurrentReceiveSize += jdom["SendSize"].toInteger();
		m_Receivefile = new QFile(m_transfertoPath);
		qInfo() << "has receive file! info:" << "m_totalReceiveSize:" << m_totalReceiveSize << "m_fileName:" << m_fileName << "m_fileType:" << m_fileType
			<< "m_transferPath:" << m_transferPath << "m_Receivefile:" << m_Receivefile << "m_transfertoPath:" << m_transfertoPath << "m_CurrentReceiveSize:"
			<< m_CurrentReceiveSize;
	}
	if (jdom["type"].toString() == REMOTE_PUT)
	{
		QString path = jdom["path"].toString();
		QString rootPath{};
		int lastIndex = path.lastIndexOf("/");
		if (lastIndex != -1)
		{
			rootPath = path.left(lastIndex);
		}
		QDir dir(rootPath);
		m_SendtransferPath = rootPath;
		m_SendtransfertoPath = jdom["toPath"].toString();
		QDirIterator it(dir);
		while (it.hasNext())
		{
			QFileInfo fileInfo = it.nextFileInfo();
			auto fileList = jdom["fileNames"].toArray();
			for (int i = 0; i < fileList.count(); i++)
			{
				if (fileInfo.fileName() == fileList.at(i).toString())
				{
					qInfo() << jdom["path"].toString() + "/" + fileInfo.fileName();
					m_Sendfile = new QFile(path);
					if (!m_Sendfile->open(QIODevice::ReadOnly))
					{
						qInfo() << "556 file open failed";
					}
					m_SendfileName = fileInfo.fileName();
					m_totalSendSize = m_Sendfile->size();
					m_SendfileType = fileType(&fileInfo);
					if (m_Sendfile->size() > 4MB)
					{
						qint64 sendSize = 0;
						while (sendSize < m_totalSendSize)
						{
							QJsonDocument jdom;
							QJsonObject jobj;
							sendSize += 4MB;
							m_CurrentSendSize = sendSize;
							qInfo() << "565m_CurrentSendSize:" << m_CurrentSendSize;
							jobj.insert("type", PUT);
							jobj.insert("fileName", m_SendfileName);
							jobj.insert("filetype", m_SendfileType);
							jobj.insert("transferPath", m_SendtransferPath);
							jobj.insert("fileSize", m_totalSendSize);
							jobj.insert("transfertoPath", m_SendtransfertoPath + "/" + m_SendfileName);
							jobj.insert("SendSize", m_CurrentSendSize);
							jdom.setObject(jobj);
							qInfo() << jdom.toJson(QJsonDocument::Compact);
							SendJson(jdom.toJson(QJsonDocument::Compact));
							SendBinaryMessage(m_Sendfile->read(4MB));
						}
					}
					else
					{
						QJsonDocument jdom;
						QJsonObject jobj;
						jobj.insert("type", PUT);
						jobj.insert("fileName", m_SendfileName);
						jobj.insert("filetype", m_SendfileType);
						jobj.insert("transferPath", m_SendtransferPath);
						jobj.insert("fileSize", m_totalSendSize);
						jobj.insert("transfertoPath", m_SendtransfertoPath + "/" + m_SendfileName);
						jobj.insert("SendSize", m_totalSendSize);
						jdom.setObject(jobj);
						qInfo() << jdom.toJson(QJsonDocument::Compact);
						SendJson(jdom.toJson(QJsonDocument::Compact));
						SendBinaryMessage(m_Sendfile->read(4MB));
					}
					m_Sendfile->deleteLater();
				}
			}
		}
	}
	if (jdom["type"].toString() == CLIENTS)
	{
		qInfo() << "Current con client:" << jdom;
	}
}

void mainWindow::onbtn_Transferclicked()
{
	TransferModel.clear();
	QItemSelectionModel* selectModel = ui->Tbw_LocalFile->selectionModel();
	QItemSelectionModel* selectRemoteModel = ui->Tbw_RemoteFile->selectionModel();
	QModelIndexList selectRows = selectModel->selectedRows();
	QModelIndexList selectRemoteRows = selectRemoteModel->selectedRows();
	if (!selectRows.isEmpty())
	{
		qInfo() << "local select:" << selectRows;
		for (auto& Row : selectRows)
		{
			QList<QStandardItem*> items;
			QString fileName = Row.data().toString();
			QFileInfo fileInfo(ui->Cox_LocalFileCox->currentText() + "/" + fileName);
			qsizetype fileSize = fileInfo.size();
			QDateTime lastModifyTime = fileInfo.fileTime(QFileDevice::FileModificationTime);
			QString filetype = fileType(&fileInfo);
			items.append(new QStandardItem(fileName));
			items.append(new QStandardItem(QString::number(fileSize)));
			items.append(new QStandardItem(lastModifyTime.toString("yyyy/mm/dd")));
			items.append(new QStandardItem(filetype));
			TransferModel.appendRow(items);
		}
		TransferFileDir.setPath(ui->Cox_LocalFileCox->currentText());
		ui->Tbw_State->setModel(&TransferModel);
		selectModel->clearSelection();
		transferState = 1;
	}
	else if(!selectRemoteRows.isEmpty())
	{
		qInfo() << "remote select:" << selectRemoteRows;
		for (auto& Row : selectRemoteRows)
		{
			QList<QStandardItem*> items;
			QString fileName = Row.data().toString();
			QFileInfo fileInfo(ui->Cox_RemoteFileCox->currentText() + "/" + fileName);
			qsizetype fileSize = fileInfo.size();
			QDateTime lastModifyTime = fileInfo.fileTime(QFileDevice::FileModificationTime);
			QString filetype = fileType(&fileInfo);
			items.append(new QStandardItem(fileName));
			items.append(new QStandardItem(QString::number(fileSize)));
			items.append(new QStandardItem(lastModifyTime.toString("yyyy/mm/dd")));
			items.append(new QStandardItem(filetype));
			TransferModel.appendRow(items);
		}
		TransferFileDir.setPath(ui->Cox_RemoteFileCox->currentText());
		ui->Tbw_State->setModel(&TransferModel);
		selectRemoteModel->clearSelection();
		transferState = 2;
	}
	else
	{
		transferState = 0;
		QMessageBox::information(this, "tip", "please select file first", QMessageBox::Apply);
	}
}

void mainWindow::onbtn_StartAll()
{
	qDebug() << "onbtn_StartAll";
	int row = 0;
	QList<QString> fileNames{};
	while (row < TransferModel.rowCount())
	{
		QString fileName = TransferModel.data(TransferModel.index(row++, 0)).toString();
		fileNames.append(fileName);
	}
	for (auto& fileName : fileNames)
	{
		QString path = TransferFileDir.canonicalPath() + "/" + fileName;
		QFileInfo fileInfo(path);
		qDebug() << path;
		if (fileType(&fileInfo) == "folder")
		{
			qDebug() << "Sendfolder!";
			sendFolder(path);
		}
		else
		{	
			qDebug() << "Sendfile!";
			sendFile(path,fileInfo.fileName());
		}
	}
}

void mainWindow::onbtn_PauseAll()
{
}

void mainWindow::onbtn_CanselAll()
{
	TransferModel.clear();
	ui->Tbw_State->setModel(&TransferModel);
}

void mainWindow::sendFile(const QString& path, const QString& fileName)
{
	if (transferState == 1)
	{
		if (TransferModel.rowCount() != 0)
		{
			m_SendfileName = fileName;
			m_SendtransferPath = path;
			m_SendtransfertoPath = ui->Cox_RemoteFileCox->currentText() + "/" + m_SendfileName;
			m_Sendfile = new QFile(m_SendtransferPath);
			if (!m_Sendfile->open(QIODevice::ReadOnly))
			{
				qDebug() << "file open failed!";
			}
			m_totalSendSize = m_Sendfile->size();
			QFileInfo fileinfo(m_SendtransferPath);
			m_SendfileType = fileType(&fileinfo);
			m_CurrentSendSize = 0;
			if (m_totalSendSize > 4MB)
			{
				while (m_CurrentSendSize < m_totalSendSize)
				{
					m_CurrentSendSize += 4MB;
					QJsonDocument jdom;
					QJsonObject jobj;
					jobj.insert("type", PUT);
					jobj.insert("fileName", m_SendfileName);
					jobj.insert("fileSize", m_totalSendSize);
					jobj.insert("fileType", m_SendfileType);
					jobj.insert("transferPath", m_SendtransferPath);
					jobj.insert("transfertoPath", m_SendtransfertoPath);
					jobj.insert("SendSize", m_CurrentSendSize);
					jdom.setObject(jobj);
					qInfo() << "big file seed 748!!!";
					updateProgressBar(m_SendtransfertoPath, m_CurrentSendSize, m_totalSendSize, mainWindow::SendState);
					SendJson(jdom.toJson(QJsonDocument::Compact));
					SendBinaryMessage(m_Sendfile->read(4MB));
				}
			}
			else
			{
				QJsonDocument jdom;
				QJsonObject jobj;
				jobj.insert("type", PUT);
				jobj.insert("fileName", m_SendfileName);
				jobj.insert("fileSize", m_totalSendSize);
				jobj.insert("fileType", m_SendfileType);
				jobj.insert("transferPath", m_SendtransferPath);
				jobj.insert("transfertoPath", m_SendtransfertoPath);
				jobj.insert("SendSize", m_totalSendSize);
				jdom.setObject(jobj);
				updateProgressBar(m_SendtransfertoPath, m_totalSendSize, m_totalSendSize, mainWindow::SendState);
				SendJson(jdom.toJson(QJsonDocument::Compact));
				SendBinaryMessage(m_Sendfile->read(4MB));
			}
			
		}
		else
		{
			QMessageBox::information(this, "Tip", "No Selected file!", QMessageBox::Apply);
		}
	}
	else if (transferState == 2)
	{
		if (TransferModel.rowCount() != 0)
		{
			QJsonDocument jdom;
			QJsonObject jobj;
			QJsonArray jarray;
			jobj.insert("type", REMOTE_PUT);
			jobj.insert("path", path);
			jobj.insert("toPath", ui->Cox_LocalFileCox->currentText());
			jarray.append(fileName);
			jobj.insert("fileNames", jarray);
			jdom.setObject(jobj);
			qInfo() << jdom.toJson(QJsonDocument::Compact);
			SendJson(jdom.toJson(QJsonDocument::Compact));
		}
		else
		{
			QMessageBox::information(this, "Tip", "No Selected file!", QMessageBox::Apply);
		}
	}
}

void mainWindow::sendFolder(const QString& path)
{
	qInfo() << "sendfolder!";
	int row = 0;
	QList<QString> fileNames{};
	QDirIterator dir(path);
	while (dir.hasNext())
	{
		QFileInfo fileInfo = dir.nextFileInfo();
		fileNames.append(fileInfo.fileName());
	}
	for (int i = 2; i < fileNames.count(); i++)
	{
		QString Path = path + "/" + fileNames.at(i);
		QFileInfo fileInfo(Path);
		if (fileType(&fileInfo) == "folder")
		{
			qInfo() << "798" << Path;
			sendFolder(Path);
		}
		else
		{
			sendFile(Path,fileInfo.fileName());
		}
	}
}

void mainWindow::updateProgressBar(const QString& fileName, const qint64 currentSendSize, const qint64 totalSendSize, const mainWindow::ProgressBar State)
{
	int lastIndex =  fileName.lastIndexOf('/');
	qInfo() << "lastIndexlastIndexlastIndexlastIndex:" << lastIndex;
	QString FileName = fileName.mid(lastIndex + 1, fileName.count() - 1);
	if (State == mainWindow::ProgressBar::SendState)
	{
		ui->Pb_SendFileName->setText(QString("Sending... file name:%1").arg(FileName));
	}
	else
	{
		ui->Pb_SendFileName->setText(QString("Receiving... file name:%1").arg(FileName));
	}
	int value = (totalSendSize > 0) ? static_cast<int>((currentSendSize * 100) / totalSendSize) : 0;
	value = (value > 100) ? 100 : value; 
	ui->Pb_Progressbar->setValue(value);
	if (currentSendSize >= totalSendSize)
	{
		// 使用非模态消息框
		QMessageBox* msgBox = new QMessageBox(this);
		msgBox->setIcon(QMessageBox::Information);
		msgBox->setWindowTitle("Tip");
		msgBox->setText((State == mainWindow::ProgressBar::SendState) ? "Send successful!" : QString("Has new file received from Peer computer!file path:%1")
		.arg(fileName));
		QPushButton* applyBtn = msgBox->addButton(QMessageBox::Apply);
		connect(applyBtn, &QPushButton::clicked, this, [=]()
			{
				ui->Pb_Progressbar->setValue(0);
				ui->Pb_SendFileName->setText("Stand by...");
			});
		msgBox->show();
	}	
}
