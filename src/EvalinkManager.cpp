#include "EvalinkManager.hpp"
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QUrl>
#include <QNetworkRequest>

EvalinkManager& EvalinkManager::instance() {
    static EvalinkManager s_instance;
    return s_instance;
}

EvalinkManager::EvalinkManager() {
    m_net = new QNetworkAccessManager(this);
    connect(m_net, &QNetworkAccessManager::finished, this, &EvalinkManager::onRpcReply);

    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, &EvalinkManager::onPollTimer);
    m_pollTimer->start(1000); // Poll every second

    // Initial check and daemon ensure
    ensureDaemonRunning();
    refresh();
}

bool EvalinkManager::isEvalinkInstalled() const {
    QStringList candidates = {
        QDir::homePath() + "/.local/bin/evalink",
        "/usr/local/bin/evalink",
        "/usr/bin/evalink"
    };
    for (const QString& path : candidates) {
        if (QFileInfo::exists(path) && QFileInfo(path).isExecutable()) {
            return true;
        }
    }
    return false;
}

void EvalinkManager::ensureDaemonRunning() {
    if (m_daemonStartAttempted) return;
    m_daemonStartAttempted = true;

    QString evalinkBin = QDir::homePath() + "/.local/bin/evalink";
    if (QFileInfo::exists(evalinkBin) && QFileInfo(evalinkBin).isExecutable()) {
        QProcess::startDetached(evalinkBin, {"daemon", "start"});
    } else {
        // Fallback: aria2c directly
        QProcess::startDetached("aria2c", {
            "--enable-rpc",
            "--rpc-listen-all=false",
            "--rpc-allow-origin-all",
            "--max-connection-per-server=16",
            "--split=16",
            "--min-split-size=1M",
            "--daemon"
        });
    }
}

QString EvalinkManager::formatBytes(qint64 bytes) {
    if (bytes <= 0) return "0 B";
    if (bytes < 1024) return QString("%1 B").arg(bytes);
    if (bytes < 1024 * 1024) return QString("%1 KB").arg(QString::number(bytes / 1024.0, 'f', 1));
    if (bytes < 1024 * 1024 * 1024) return QString("%1 MB").arg(QString::number(bytes / (1024.0 * 1024.0), 'f', 1));
    return QString("%1 GB").arg(QString::number(bytes / (1024.0 * 1024.0 * 1024.0), 'f', 2));
}

QString EvalinkManager::formatSpeed(qint64 bytesPerSec) {
    if (bytesPerSec <= 0) return "0 B/s";
    return QString("%1/s").arg(formatBytes(bytesPerSec));
}

QString EvalinkManager::formatEta(qint64 remainingBytes, qint64 speedBytesPerSec) {
    if (speedBytesPerSec <= 0 || remainingBytes <= 0) return "--";
    qint64 seconds = remainingBytes / speedBytesPerSec;
    if (seconds < 60) return QString("%1s").arg(seconds);
    if (seconds < 3600) return QString("%1m %2s").arg(seconds / 60).arg(seconds % 60);
    return QString("%1h %2m").arg(seconds / 3600).arg((seconds % 3600) / 60);
}

void EvalinkManager::setPollingEnabled(bool enabled) {
    if (enabled && !m_pollTimer->isActive()) {
        m_pollTimer->start(1000);
        refresh();
    } else if (!enabled && m_pollTimer->isActive()) {
        m_pollTimer->stop();
    }
}

void EvalinkManager::refresh() {
    sendMulticall();
}

void EvalinkManager::onPollTimer() {
    sendMulticall();
}

void EvalinkManager::sendRpcRequest(const QString& method, const QJsonArray& params, const QString& reqId) {
    QJsonObject obj;
    obj["jsonrpc"] = "2.0";
    obj["id"] = reqId.isEmpty() ? method : reqId;
    obj["method"] = method;
    obj["params"] = params;

    QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    QUrl url(QString("http://%1:%2/jsonrpc").arg(m_rpcHost).arg(m_rpcPort));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_net->post(request, data);
}

void EvalinkManager::sendMulticall() {
    QJsonObject obj;
    obj["jsonrpc"] = "2.0";
    obj["id"] = "poll_multicall";
    obj["method"] = "system.multicall";

    QJsonArray calls;

    QJsonObject callStat;
    callStat["methodName"] = "aria2.getGlobalStat";
    callStat["params"] = QJsonArray();
    calls.append(callStat);

    QJsonObject callActive;
    callActive["methodName"] = "aria2.tellActive";
    callActive["params"] = QJsonArray();
    calls.append(callActive);

    QJsonObject callWaiting;
    callWaiting["methodName"] = "aria2.tellWaiting";
    callWaiting["params"] = QJsonArray{0, 20};
    calls.append(callWaiting);

    QJsonObject callStopped;
    callStopped["methodName"] = "aria2.tellStopped";
    callStopped["params"] = QJsonArray{0, 20};
    calls.append(callStopped);

    QJsonArray params;
    params.append(calls);
    obj["params"] = params;

    QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    QUrl url(QString("http://%1:%2/jsonrpc").arg(m_rpcHost).arg(m_rpcPort));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_net->post(request, data);
}

void EvalinkManager::onRpcReply(QNetworkReply* reply) {
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        if (!m_daemonStartAttempted) {
            ensureDaemonRunning();
        }
        return;
    }

    QByteArray respData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(respData);
    if (!doc.isObject()) return;

    QJsonObject root = doc.object();
    QString id = root.value("id").toString();

    if (id == "poll_multicall" && root.contains("result")) {
        QJsonArray result = root.value("result").toArray();
        parseMulticallResponse(result);
    } else if (id == "add_download" || id == "action_cmd" || id == "purge_cmd") {
        refresh();
    }
}

void EvalinkManager::parseMulticallResponse(const QJsonArray& results) {
    if (results.size() < 4) return;

    // 0: GlobalStat
    QJsonArray statArr = results[0].toArray();
    if (!statArr.isEmpty()) {
        QJsonObject statObj = statArr[0].toObject();
        m_globalDownloadSpeed = statObj.value("downloadSpeed").toString().toLongLong();
        m_activeCount = statObj.value("numActive").toString().toInt();
    }

    QList<EvalinkTask> newTasks;

    auto parseTaskArray = [&](const QJsonArray& arr) {
        for (const auto& itemVal : arr) {
            QJsonObject item = itemVal.toObject();
            EvalinkTask t;
            t.gid = item.value("gid").toString();
            t.status = item.value("status").toString();
            t.totalBytes = item.value("totalLength").toString().toLongLong();
            t.completedBytes = item.value("completedLength").toString().toLongLong();
            t.downloadSpeed = item.value("downloadSpeed").toString().toLongLong();
            t.dir = item.value("dir").toString();
            t.errorMessage = item.value("errorMessage").toString();

            if (t.totalBytes > 0) {
                t.percent = static_cast<int>((t.completedBytes * 100) / t.totalBytes);
                qint64 remaining = t.totalBytes - t.completedBytes;
                t.eta = formatEta(remaining, t.downloadSpeed);
            } else {
                t.percent = (t.status == "complete") ? 100 : 0;
                t.eta = "--";
            }

            // Extract file name
            QJsonArray files = item.value("files").toArray();
            if (!files.isEmpty()) {
                QJsonObject firstFile = files[0].toObject();
                QString filePath = firstFile.value("path").toString();
                if (!filePath.isEmpty()) {
                    t.name = QFileInfo(filePath).fileName();
                } else {
                    QJsonArray uris = firstFile.value("uris").toArray();
                    if (!uris.isEmpty()) {
                        QString uri = uris[0].toObject().value("uri").toString();
                        t.name = QFileInfo(QUrl(uri).path()).fileName();
                        if (t.name.isEmpty()) t.name = uri;
                    }
                }
            }

            if (item.contains("bittorrent")) {
                QJsonObject bt = item.value("bittorrent").toObject();
                if (bt.contains("info")) {
                    QString btName = bt.value("info").toObject().value("name").toString();
                    if (!btName.isEmpty()) t.name = btName;
                }
            }

            if (t.name.isEmpty()) {
                t.name = "Download (" + t.gid + ")";
            }

            // Notification on completion
            if (t.status == "complete" && !m_notifiedCompletedGids.contains(t.gid)) {
                m_notifiedCompletedGids.insert(t.gid);
                emit downloadCompleted(t.name, t.dir);
            } else if (t.status == "error" && !m_notifiedCompletedGids.contains(t.gid)) {
                m_notifiedCompletedGids.insert(t.gid);
                emit downloadError(t.name, t.errorMessage);
            }

            newTasks.append(t);
        }
    };

    // 1: Active, 2: Waiting, 3: Stopped
    if (!results[1].toArray().isEmpty()) parseTaskArray(results[1].toArray());
    if (!results[2].toArray().isEmpty()) parseTaskArray(results[2].toArray());
    if (!results[3].toArray().isEmpty()) parseTaskArray(results[3].toArray());

    m_tasks = newTasks;
    emit tasksUpdated(m_tasks, m_globalDownloadSpeed, m_activeCount);
}

void EvalinkManager::addDownload(const QString& url, const QString& destinationDir, const QString& customFilename) {
    if (url.trimmed().isEmpty()) return;

    // Use aria2.addUri JSON-RPC
    QJsonArray uris;
    uris.append(url.trimmed());

    QJsonObject options;
    if (!destinationDir.isEmpty()) {
        options["dir"] = destinationDir;
    }
    if (!customFilename.trimmed().isEmpty()) {
        options["out"] = customFilename.trimmed();
    }

    QJsonArray params;
    params.append(uris);
    params.append(options);

    sendRpcRequest("aria2.addUri", params, "add_download");
}

void EvalinkManager::pauseDownload(const QString& gid) {
    QJsonArray params;
    params.append(gid);
    sendRpcRequest("aria2.pause", params, "action_cmd");
}

void EvalinkManager::resumeDownload(const QString& gid) {
    QJsonArray params;
    params.append(gid);
    sendRpcRequest("aria2.unpause", params, "action_cmd");
}

void EvalinkManager::cancelDownload(const QString& gid) {
    QJsonArray params;
    params.append(gid);
    sendRpcRequest("aria2.remove", params, "action_cmd");
    sendRpcRequest("aria2.removeDownloadResult", params, "action_cmd");
}

void EvalinkManager::purgeCompleted() {
    QJsonArray params;
    sendRpcRequest("aria2.purgeDownloadResult", params, "purge_cmd");
}
