#pragma once

#include <QDirIterator>
#include <QGraphicsScene>
#include <QJsonObject>
#include <QUndoCommand>
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

namespace Ui {
class PaletteWidget;
class EditColorsCommand;
class EditTranslationsCommand;
} // namespace Ui

class EditColorsCommand : public QObject, public QUndoCommand {
    Q_OBJECT

public:
    explicit EditColorsCommand(D1Pal *, quint8, quint8, QColor, QUndoCommand *parent = nullptr);
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

class PaletteWidget : public QWidget {
    Q_OBJECT

public:
    explicit PaletteWidget(QJsonObject *configuration, QWidget *parent, QString title, bool isTrn);
    ~PaletteWidget();

    void setPal(D1Pal *p);
    void setTrn(D1Trn *t);

    void initialize(D1Pal *p, CelView *c, D1PalHits *ph);
    void initialize(D1Pal *p, LevelCelView *lc, D1PalHits *ph);

    void initialize(D1Pal *p, D1Trn *t, CelView *c, D1PalHits *ph);
    void initialize(D1Pal *p, D1Trn *t, LevelCelView *lc, D1PalHits *ph);

    void initializeUi();
    void initializePathComboBox();
    void initializeDisplayComboBox();

    void reloadConfig();
    void selectColor(quint8);
    void selectColors();
    void checkTranslationsSelection(QList<quint8>);

    QString getPath(QString);
    void setPath(QString, QString);
    void addPath(QString, QString);
    void removePath(QString);
    void selectPath(QString);
    QString getSelectedPath();

    // Coordinates functions
    QRectF getColorCoordinates(quint8);
    quint8 getColorIndexFromCoordinates(QPointF);

    // Mouse event filter
    bool eventFilter(QObject *, QEvent *);

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

    void sendEditingCommand(QUndoCommand *);

    void modified();
    void refreshed();

private slots:
    void pathComboBox_currentTextChanged(const QString &arg1);
    void displayComboBox_currentTextChanged(const QString &arg1);
    void on_colorLineEdit_returnPressed();
    void on_colorPickPushButton_clicked();
    void on_colorClearPushButton_clicked();
    void on_translationIndexLineEdit_returnPressed();
    void on_translationPickPushButton_clicked();
    void on_translationClearPushButton_clicked();
    void on_monsterTrnPushButton_clicked();

private:
    QJsonObject *configuration;

    Ui::PaletteWidget *ui;
    bool isLevelCel = false;
    bool isTrn = false;

    CelView *celView;
    LevelCelView *levelCelView;

    QGraphicsScene *scene = new QGraphicsScene(0, 0, PALETTE_WIDTH, PALETTE_WIDTH);

    QColor paletteDefaultColor = Qt::magenta;

    QColor selectionBorderColor = Qt::red;
    quint8 selectedFirstColorIndex = 0;
    quint8 selectedLastColorIndex = 0;

    bool pickingTranslationColor = false;
    bool temporarilyDisplayingAllColors = false;

    QPointer<D1Pal> pal;
    QPointer<D1Trn> trn;

    D1PalHits *palHits;

    bool buildingPathComboBox = false;
    QMap<QString, QString> paths;

    bool buildingDisplayComboBox;
};
