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

/**
 * @brief Loads current configuration from the config .json file
 *
 *  This function loads current configuration from the config .json file.
 *  It inserts specific values mapping them to keys, and creates path for the
 *  config if user has deleted it, or runs app for the first time.
 */
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

/**
 * @brief Stores current configuration in the config .json file
 */
void Config::storeConfiguration()
{
    QFile saveJson(jsonFilePath);
    saveJson.open(QIODevice::WriteOnly);
    QJsonDocument saveDoc(theConfig);
    saveJson.write(saveDoc.toJson());
    saveJson.close();
}

/**
 * @brief Creates directories on certain path
 *
 * This function creates directories on certain path. Path location depends on
 * if user is working on Windows or Mac/Linux.
 *
 * If the user works on Windows, it will save it under AppData/.config/[...].
 * On any other OS it will save it under /home/user/.config/diasurgical/[...]
 *
 * @return Returns true if path has been created or already existed - false otherwise
 */
bool Config::createDirectoriesOnPath()
{
    jsonFilePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

    return QDir().mkpath(jsonFilePath);
}

/**
 * @brief Retrieves value from .json config file
 *
 * This function retrieves value from .json config file by the value
 * specified by name parameter.
 *
 * @return Returns QJsonValue containing value of the parameter specified
 * by "name" key, otherwise if not found - returns QJsonValue::Undefined
 */
QJsonValue Config::value(const QString &name)
{
    return theConfig.value(name);
}

/**
 * @brief Inserts value in .json config file
 *
 * This function inserts value into .json config file, mapping it with
 * the key specified in parameters.
 */
void Config::insert(const QString &key, const QJsonValue &value)
{
    theConfig.insert(key, value);
}
