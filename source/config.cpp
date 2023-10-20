#include "config.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

static QJsonObject theConfig;
QString Config::jsonFilePath;

void Config::loadConfiguration()
{
    // create directories on the path if they do not exist
    if (!Config::createDirectoriesOnPath()) {
        qDebug() << "Couldn't resolve path for the config file. Configuration file won't be loaded.";
        return;
    }

    // add filename to the absolute path
    jsonFilePath += "/D1GraphicsTool.config.json";

    bool configurationModified = false;

    // If configuration file exists load it otherwise create it
    if (QFile::exists(jsonFilePath)) {
        QFile loadJson(jsonFilePath);
        loadJson.open(QIODevice::ReadOnly);
        QJsonDocument loadJsonDoc = QJsonDocument::fromJson(loadJson.readAll());
        theConfig = loadJsonDoc.object();
        loadJson.close();
    }

    if (!theConfig.contains("LastFilePath")) {
        theConfig.insert("LastFilePath", jsonFilePath);
        configurationModified = true;
    }
    if (!theConfig.contains("PaletteDefaultColor")) {
        theConfig.insert("PaletteDefaultColor", "#FF00FF");
        configurationModified = true;
    }
    if (!theConfig.contains("PaletteSelectionBorderColor")) {
        theConfig.insert("PaletteSelectionBorderColor", "#FF0000");
        configurationModified = true;
    }

    if (configurationModified) {
        Config::storeConfiguration();
    }
}

void Config::storeConfiguration()
{
    QFile saveJson(jsonFilePath);
    saveJson.open(QIODevice::WriteOnly);
    QJsonDocument saveDoc(theConfig);
    saveJson.write(saveDoc.toJson());
    saveJson.close();
}

bool Config::createDirectoriesOnPath()
{
    jsonFilePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

    return QDir().mkpath(jsonFilePath);
}

QJsonValue Config::value(const QString &name)
{
    return theConfig.value(name);
}

void Config::insert(const QString &key, const QJsonValue &value)
{
    theConfig.insert(key, value);
}
