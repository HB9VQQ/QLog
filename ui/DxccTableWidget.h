#ifndef QLOG_UI_DXCCTABLEWIDGET_H
#define QLOG_UI_DXCCTABLEWIDGET_H

#include <QWidget>
#include <QTableView>
#include <QMenu>
#include <QSettings>
#include "data/Band.h"

class DxccTableModel;

class DxccTableWidget : public QTableView
{
    Q_OBJECT
public:
    explicit DxccTableWidget(QWidget *parent = nullptr);

public slots:
    void clear();
    void setDxcc(int dxcc, const Band &highlightedBand);
    void setDxCallsign(const QString &dxCallsign, const Band &band);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void updateDxTable(const QString &condition,
                       const QVariant &conditionValue,
                       const Band &highlightedBand);

    QStringList hiddenModes() const;
    void setHiddenModes(const QStringList &modes);

    DxccTableModel* dxccTableModel;

    // Cache last query params for refresh after mode toggle
    QString lastCondition;
    QVariant lastConditionValue;
    Band lastHighlightedBand;
    bool hasLastQuery;
};

#endif // QLOG_UI_DXCCTABLEWIDGET_H
