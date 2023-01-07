#pragma once

#include <QDirIterator>
#include <QGraphicsScene>
#include <QJsonObject>
#include <QMouseEvent>
#include <QStyle>
#include <QUndoCommand>
#include <QUndoStack>
#include <QWidget>

#include "celview.h"
#include "d1pal.h"
#include "d1palhits.h"
#include "d1trn.h"
#include "levelcelview.h"

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

class EditColorsCommand : public QObject, public QUndoCommand {
    Q_OBJECT

public:
    explicit EditColorsCommand(D1Pal *, quint8, quint8, QColor, QColor, QUndoCommand *parent = nullptr);
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

class EditTranslationsCommand : public QObject, public QUndoCommand {
    Q_OBJECT

public:
    explicit EditTranslationsCommand(D1Trn *, quint8, quint8, QList<quint8>, QUndoCommand *parent = nullptr);
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

class ClearTranslationsCommand : public QObject, public QUndoCommand {
    Q_OBJECT

public:
    explicit ClearTranslationsCommand(D1Trn *, quint8, quint8, QUndoCommand *parent = nullptr);
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

class PaletteWidget : public QWidget {
    Q_OBJECT

public:
    explicit PaletteWidget(QJsonObject *configuration, QUndoStack *undoStack, QString title);
    ~PaletteWidget();

    void setPal(D1Pal *p);
    void setTrn(D1Trn *t);
    bool isTrnWidget();

    void initialize(D1Pal *p, CelView *c, D1PalHits *ph);
    void initialize(D1Pal *p, LevelCelView *lc, D1PalHits *ph);

    void initialize(D1Pal *p, D1Trn *t, CelView *c, D1PalHits *ph);
    void initialize(D1Pal *p, D1Trn *t, LevelCelView *lc, D1PalHits *ph);

    void initializeUi();
    void initializePathComboBox();
    void initializeDisplayComboBox();

    void reloadConfig();
    void selectColor(quint8);
    void checkTranslationsSelection(QList<quint8>);

    void addPath(QString, QString);
    void removePath(QString);
    void selectPath(QString);
    QString getSelectedPath();

    // color selection handlers
    void startColorSelection(int colorIndex);
    void changeColorSelection(int colorIndex);
    void finishColorSelection();

    // Display functions
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

    void on_actionUndo_triggered();
    void on_actionRedo_triggered();

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
    QJsonObject *configuration;
    QUndoStack *undoStack;
    Ui::PaletteWidget *ui;
    bool isLevelCel;
    bool isTrn;

    CelView *celView;
    LevelCelView *levelCelView;

    PaletteScene *scene;

    QColor paletteDefaultColor = Qt::magenta;

    QColor selectionBorderColor = Qt::red;
    quint8 selectedFirstColorIndex = 0;
    quint8 selectedLastColorIndex = 0;

    bool pickingTranslationColor = false;
    bool temporarilyDisplayingAllColors = false;

    QPointer<D1Pal> pal;
    QPointer<D1Trn> trn;

    D1PalHits *palHits;

    QMap<QString, QString> paths;
};
