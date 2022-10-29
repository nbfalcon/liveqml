#include "liveqml.h"
#include <QFileInfo>
#include <QDir>
#include <QWindow>
#include <QQmlComponent>
#include <QThread>

namespace qtx {

LiveQML::LiveQML(QQmlEngine *engine, QObject *parent)
    : QObject{parent}, engine{engine}, fileWatcher{}
{
    QObject::connect(&fileWatcher, &QFileSystemWatcher::fileChanged, this, &LiveQML::handleFileChanged);
    engine->addUrlInterceptor(this);
}

void LiveQML::load(const QString &path)
{
    auto file = importPath.absoluteFilePath(path);

    fileWatcher.addPath(file);
    roots.push_back(file);
    qDebug() << "(LIVEQML) Loading (first time):" << file;
    doLoad(file);
}

void LiveQML::handleFileChanged(const QString &qmlFile)
{
    // Editors first delete the file, then write it (QtCreator)
    // https://stackoverflow.com/a/30076119
    if (!fileWatcher.files().contains(qmlFile)) {
        QThread::msleep(1);
        fileWatcher.addPath(qmlFile);
    }

    engine->clearComponentCache();
    for (const auto &root : roots) {
        doLoad(root);
    }
}

void LiveQML::handleObjectCreated(QObject *object, const QString &filePath)
{
    if (object != nullptr) {
        auto alreadyExists = loadedObjects.find(filePath);
        if (alreadyExists != loadedObjects.end()) {
            auto *oldObjectIsAWindow = qobject_cast<QWindow*>(alreadyExists->second);
            if (oldObjectIsAWindow) {
                auto *newObjectIsAlsoAWindow = qobject_cast<QWindow*>(object);
                if (newObjectIsAlsoAWindow) {
                    newObjectIsAlsoAWindow->setScreen(oldObjectIsAWindow->screen());
                    newObjectIsAlsoAWindow->setGeometry(oldObjectIsAWindow->geometry());
                }

                qDebug() << "(LIVEQML) Shutting down:" << filePath;
                oldObjectIsAWindow->close();
                oldObjectIsAWindow->deleteLater();
            }
            qDebug() << "(LIVEQML) Successfully reloaded:" << filePath;
        } else {
            qDebug() << "(LIVEQML) Successfully loaded (first time):" << filePath;
        }
        loadedObjects[filePath] = object;
    } else {
        qDebug() << "(LIVEQML) Failed to reload:" << filePath;
    }
    qDebug().noquote().nospace() << "\n";
}

void LiveQML::doLoad(const QString &filePath)
{
    QFile asFile(filePath);
    asFile.open(QFile::ReadOnly);
    auto bytes = asFile.readAll();

    if (bytes.isEmpty()) {
        if (asFile.error() != QFile::NoError) {
            qDebug("(LIVEQML): Failed to load: file is empty");
        } else {
            qDebug() << "(LIVEQML) Failed to load: read error:" << asFile.errorString();
        }
        return;
    }

    QQmlComponent componentToBeLoaded(engine);
    componentToBeLoaded.setData(bytes, "file://" + filePath);
    QObject *component = nullptr;
    if (!componentToBeLoaded.isError()) {
        component = componentToBeLoaded.createWithInitialProperties(initialProperties);
    }
    handleObjectCreated(component, filePath);
}

QUrl LiveQML::intercept(const QUrl &path, DataType)
{
    if (path.isLocalFile()) {
        qDebug() << "(LIVEQML) Dependency:" << path;
        auto filePath = path.toLocalFile();
        fileWatcher.addPath(filePath);
    }
    return path;
}

//void LiveQML::handleObjectCreationFailed(const QUrl &url)
//{
//    auto urlAsStr = url.toString();
//    if (urlsWeAreWatching.find(urlAsStr) != urlsWeAreWatching.end()) {
//        qDebug() << "(LIVEQML) Failed to load:" << url;
//    }
//}

}
