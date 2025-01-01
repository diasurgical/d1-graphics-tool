#pragma once

#include <memory>

#include <QDirIterator>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QStyle>
#include <QWidget>

#include "d1formats/d1trn.h"
#include "palette/d1pal.h"
#include "palette/d1palhits.h"
#include "undostack/undostack.h"
#include "views/celview.h"
#include "views/levelcelview.h"

#define PALETTE_WIDTH 192
#define PALETTE_COLORS_PER_LINE 16
#define PALETTE_COLOR_SPACING 1
#define PALETTE_SELECTION_WIDTH 2

enum class PWIDGET_CALLBACK_TYPE {
    PWIDGET_CALLBACK_NEW,
    PWIDGET_CALLBACK_OPEN,
    PWIDGET_CALLBACK_SAVE,
    PWIDGET_CALLBACK_SAVEAS,
    PWIDGET_CALLBACK_CLOSE,
};

namespace Ui {
class PaletteScene;
class PaletteWidget;
class EditColorsCommand;
class EditTranslationsCommand;
} // namespace Ui

class EditColorsCommand : public QObject, public Command {
    Q_OBJECT

public:
    explicit EditColorsCommand(D1Pal *, quint8, quint8, QColor, QColor);
    ~EditColorsCommand() = default;

    void undo() override;
    void redo() override;

signals:
    void modified();

private:
    QPointer<D1Pal> pal;
    quint8 startColorIndex;
    quint8 endColorIndex;
    QList<QColor> initialColors;
    QColor newColor;
    QColor endColor;
};

class EditTranslationsCommand : public QObject, public Command {
    Q_OBJECT

public:
    explicit EditTranslationsCommand(D1Trn *, quint8, quint8, QList<quint8>);
    ~EditTranslationsCommand() = default;

    void undo() override;
    void redo() override;

signals:
    void modified();

private:
    QPointer<D1Trn> trn;
    quint8 startColorIndex;
    quint8 endColorIndex;
    QList<quint8> initialTranslations;
    QList<quint8> newTranslations;
};

class ClearTranslationsCommand : public QObject, public Command {
    Q_OBJECT

public:
    explicit ClearTranslationsCommand(D1Trn *, quint8, quint8);
    ~ClearTranslationsCommand() = default;

    void undo() override;
    void redo() override;

signals:
    void modified();

private:
    QPointer<D1Trn> trn;
    quint8 startColorIndex;
    quint8 endColorIndex;
    QList<quint8> initialTranslations;
};

class PaletteScene : public QGraphicsScene {
    Q_OBJECT

public:
    PaletteScene(QWidget *view);

private slots:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

signals:
    void framePixelClicked(quint16, quint16);

private:
    QWidget *view;
};

using PaletteFileInfo = struct PaletteFileInfo {
    QString name;   // e.g., "palette" or "translation"
    QString suffix; // e.g., ".pal" or ".trn"
};

enum class PaletteType : std::uint8_t {
    Palette = 0,
    Translation = 1,
    UniqTranslation = 2
};

class PaletteWidget : public QWidget {
    Q_OBJECT

public:
    explicit PaletteWidget(std::shared_ptr<UndoStack> undoStack, QString title);
    ~PaletteWidget();

    void setPal(const QString &path);
    void setTrn(const QString &path);
    bool isTrnWidget();

    void initialize(D1Pal *p, CelView *c, D1PalHits *ph, PaletteType palType);
    void initialize(D1Pal *p, LevelCelView *lc, D1PalHits *ph, PaletteType palType);

    void initialize(D1Pal *p, D1Trn *t, CelView *c, D1PalHits *ph, PaletteType palType);
    void initialize(D1Pal *p, D1Trn *t, LevelCelView *lc, D1PalHits *ph, PaletteType palType);

    void initializeUi();
    void initializePathComboBox();
    void initializeDisplayComboBox();

    void reloadConfig();
    void selectColor(quint8);
    void checkTranslationsSelection(QList<quint8>);

    void addPath(const QString &, const QString &, D1Pal *pal);
    void removePath(const QString &);
    void selectPath(const QString &);

    QString getWidgetsDefaultPath() const;
    QString getSelectedPath() const;

    // color selection handlers
    void startColorSelection(int colorIndex);
    void changeColorSelection(int colorIndex);
    void finishColorSelection();
    [[nodiscard]] D1Pal *pal() const
    {
        return m_pal;
    };
    [[nodiscard]] D1Trn *trn() const
    {
        return m_trn;
    };

    void save();
    void newOrSaveAsFile(PWIDGET_CALLBACK_TYPE action);

    bool loadPalette(const QString &filepath);
    void openPalette();

    bool isOkToQuit();

    void closePalette();

    void setTrnPalette(D1Pal *pal);
    // Display functions
    bool displayColor(int index);
    void displayColors();
    void displaySelection();
    void temporarilyDisplayAllColors();
    void displayInfo(QString);
    void clearInfo();
    void displayBorder();
    void clearBorder();

    void refreshPathComboBox();
    void refreshColorLineEdit();
    void refreshIndexLineEdit();
    void refreshTranslationIndexLineEdit();

    void modify();
    void refresh();

signals:
    void pathSelected(QString);
    void colorsSelected(QList<quint8>);

    void displayAllRootColors();
    void displayRootInformation(QString);
    void clearRootInformation();
    void displayRootBorder();
    void clearRootBorder();

    void modified();
    void refreshed();

private:
    [[nodiscard]] PaletteFileInfo paletteFileInfo() const;
    void performSave(const QString &palFilePath, const PaletteFileInfo &fileInfo);
    QPushButton *addButton(QStyle::StandardPixmap type, QString tooltip, void (PaletteWidget::*callback)(void));

public slots:
    void ShowContextMenu(const QPoint &pos);

private slots:
    // Due to a bug in Qt these functions can not follow the naming conventions
    // if they follow, the application is going to vomit warnings in the background (maybe only in debug mode)
    void on_newPushButtonClicked();
    void on_openPushButtonClicked();
    void on_savePushButtonClicked();
    void on_saveAsPushButtonClicked();
    void on_closePushButtonClicked();

    void actionUndo_triggered();
    void actionRedo_triggered();

    void on_pathComboBox_activated(int index);
    void on_displayComboBox_activated(int index);
    void on_colorLineEdit_returnPressed();
    void on_colorPickPushButton_clicked();
    void on_colorClearPushButton_clicked();
    void on_translationIndexLineEdit_returnPressed();
    void on_translationPickPushButton_clicked();
    void on_translationClearPushButton_clicked();
    void on_monsterTrnPushButton_clicked();

private:
    std::shared_ptr<UndoStack> undoStack;
    Ui::PaletteWidget *ui;

    CelView *celView;
    LevelCelView *levelCelView;

    PaletteScene *scene;

    QColor paletteDefaultColor = Qt::magenta;

    QColor selectionBorderColor = Qt::red;
    quint8 selectedFirstColorIndex = 0;
    quint8 selectedLastColorIndex = 0;

    bool pickingTranslationColor = false;
    bool temporarilyDisplayingAllColors = false;

    D1Pal *m_pal;
    D1Trn *m_trn;

    PaletteType m_paletteType;

    D1PalHits *palHits;

    std::map<QString, std::pair<QString, D1Pal *>> m_palettes_map;
};
