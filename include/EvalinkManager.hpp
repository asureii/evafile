#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QSet>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

struct EvalinkTask {
    QString gid;
    QString name;
    QString dir;
    QString status; // "active", "waiting", "paused", "complete", "error", "removed"
    qint64 totalBytes = 0;
    qint64 completedBytes = 0;
    qint64 downloadSpeed = 0;
    int percent = 0;
    QString eta;
    QString errorMessage;
};

class EvalinkManager : public QObject {
    Q_OBJECT
public:
    static EvalinkManager& instance();

    bool isEvalinkInstalled() const;
    void ensureDaemonRunning();

    void addDownload(const QString& url, const QString& destinationDir, const QString& customFilename = QString());
    void pauseDownload(const QString& gid);
    void resumeDownload(const QString& gid);
    void cancelDownload(const QString& gid);
    void purgeCompleted();

    void refresh();
    void setPollingEnabled(bool enabled);

    const QList<EvalinkTask>& tasks() const { return m_tasks; }
    qint64 globalDownloadSpeed() const { return m_globalDownloadSpeed; }
    int activeCount() const { return m_activeCount; }

    static QString formatBytes(qint64 bytes);
    static QString formatSpeed(qint64 bytesPerSec);

signals:
    void tasksUpdated(const QList<EvalinkTask>& tasks, qint64 globalSpeed, int activeCount);
    void downloadCompleted(const QString& name, const QString& dir);
    void downloadError(const QString& name, const QString& error);

private slots:
    void onPollTimer();
    void onRpcReply(QNetworkReply* reply);

private:
    EvalinkManager();
    void sendRpcRequest(const QString& method, const QJsonArray& params, const QString& reqId = QString());
    void sendMulticall();
    void parseMulticallResponse(const QJsonArray& results);
    static QString formatEta(qint64 remainingBytes, qint64 speedBytesPerSec);

    QNetworkAccessManager* m_net;
    QTimer* m_pollTimer;
    QList<EvalinkTask> m_tasks;
    qint64 m_globalDownloadSpeed = 0;
    int m_activeCount = 0;
    int m_rpcPort = 6800;
    QString m_rpcHost = "127.0.0.1";
    QSet<QString> m_notifiedCompletedGids;
    bool m_daemonStartAttempted = false;
};
