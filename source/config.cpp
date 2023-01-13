#include "config.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

static QJsonObject theConfig;

void Config::loadConfiguration()
{
    QString jsonFilePath = QCoreApplication::applicationDirPath() + "/D1GraphicsTool.config.json";
    bool configurationModified = false;

    // If configuration file exists load it otherwise create it
    if (QFile::exists(jsonFilePath)) {
        QFile loadJson(jsonFilePath);
        loadJson.open(QIODevice::ReadOnly);
        QJsonDocument loadJsonDoc = QJsonDocument::fromJson(loadJson.readAll());
        theConfig = loadJsonDoc.object();
        loadJson.close();

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
    } else {
        theConfig.insert("LastFilePath", jsonFilePath);
        theConfig.insert("PaletteDefaultColor", "#FF00FF");
        theConfig.insert("PaletteSelectionBorderColor", "#FF0000");
        configurationModified = true;
    }

    if (configurationModified) {
        Config::storeConfiguration();
    }
}

void Config::storeConfiguration()
{
    QString jsonFilePath = QCoreApplication::applicationDirPath() + "/D1GraphicsTool.config.json";

    QFile saveJson(jsonFilePath);
    saveJson.open(QIODevice::WriteOnly);
    QJsonDocument saveDoc(theConfig);
    saveJson.write(saveDoc.toJson());
    saveJson.close();
}

QJsonValue Config::value(const QString &name)
{
    return theConfig.value(name);
}

void Config::insert(const QString &key, const QJsonValue &value)
{
    theConfig.insert(key, value);
}
