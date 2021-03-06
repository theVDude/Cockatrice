#ifndef SERVER_LOGGER_H
#define SERVER_LOGGER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QStringList>

class QSocketNotifier;
class QFile;
class ServerSocketInterface;

class ServerLogger : public QObject {
	Q_OBJECT
public:
	ServerLogger(const QString &logFileName, QObject *parent = 0);
	~ServerLogger();
	static void hupSignalHandler(int unused);
public slots:
	void logMessage(QString message, ServerSocketInterface *ssi = 0);
private slots:
#ifdef Q_OS_UNIX
	void handleSigHup();
#endif
	void flushBuffer();
signals:
	void sigFlushBuffer();
private:
	static int sigHupFD[2];
	QSocketNotifier *snHup;
	static QFile *logFile;
	bool flushRunning;
	QStringList buffer;
	QMutex bufferMutex;
};

class ServerLoggerThread : public QThread {
	Q_OBJECT
private:
	QString fileName;
	ServerLogger *logger;
	QWaitCondition initWaitCondition;
protected:
	void run();
public:
	ServerLoggerThread(const QString &_fileName, QObject *parent = 0);
	~ServerLoggerThread();
	ServerLogger *getLogger() const { return logger; }
	void waitForInit();
};

#endif
