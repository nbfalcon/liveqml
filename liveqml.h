#ifndef LIVEQML_H
#define LIVEQML_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QFileSystemWatcher>
#include <QString>
#include <QVariantMap>
#include <QDir>
#include <unordered_set>
#include <unordered_map>

namespace qtx {

class LiveQML : public QObject, private QQmlAbstractUrlInterceptor
{
    Q_OBJECT
public:
    explicit LiveQML(QQmlEngine *engine, QObject *parent = nullptr);

    void setInitialProperties(const QVariantMap &initialProperties) {
        this->initialProperties = initialProperties;
    }
    void setBaseDirectory(const QDir importPath) {
        this->importPath = importPath;
    }

    void load(const QString &path);

signals:

private slots:
    void handleFileChanged(const QString &path);

private:
    void handleObjectCreated(QObject *object, const QString &url);
    void doLoad(const QString &theUrl);

    QUrl intercept(const QUrl &path, DataType type) override;

private:
    QQmlEngine *engine;
    QVariantMap initialProperties;
    QDir importPath;

    // QML files we should reload
    std::vector<QString> roots;
    // QML files which are alreay loaded
    std::unordered_map<QString, QObject *> loadedObjects;
    QFileSystemWatcher fileWatcher;
};

}

#endif // LIVEQML_H
