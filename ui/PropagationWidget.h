#ifndef QLOG_UI_PROPAGATIONWIDGET_H
#define QLOG_UI_PROPAGATIONWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QBoxLayout>

class QDockWidget;

/**
 * @brief Dockable widget showing best HF band per target region.
 *
 * Displays corridor "chips" — one per target region (AF, SA, NA, AS, OC, PAC)
 * with a colored band pill and border indicating DX Index quality.
 *
 * Connects to PropagationData::dataUpdated() to refresh automatically.
 * Auto-detects horizontal vs vertical dock orientation for layout.
 */
class PropagationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PropagationWidget(QWidget *parent = nullptr);
    ~PropagationWidget() override;

public slots:
    void updateDisplay();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool event(QEvent *event) override;

private:
    // Layout
    QBoxLayout *chipLayout = nullptr;
    QLabel     *titleLabel = nullptr;
    QLabel     *statusLabel = nullptr;

    // Chip rendering
    struct ChipWidget
    {
        QWidget *container = nullptr;
        QLabel  *regionLabel = nullptr;
        QLabel  *bandLabel = nullptr;
    };

    QList<ChipWidget> chips;

    void buildLayout();
    void rebuildChips();
    void updateTitle();
    bool isHorizontal() const;

    // Region display order
    static const QStringList &corridorOrder();

    // Styling helpers
    static QString bandColor(const QString &band);
    static QString borderColor(int index);
    static QString chipStyleSheet(int index, const QString &band);
    static QString bandPillStyleSheet(const QString &band);
};

#endif // QLOG_UI_PROPAGATIONWIDGET_H
