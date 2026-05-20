#pragma once

#include <QWidget>
#include <QPixmap>
#include <QRectF>

class GameLauncher : public QWidget {
    Q_OBJECT
public:
    explicit GameLauncher(QWidget* parent = nullptr);

signals:
    void gameSelected(int index);

protected:
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void leaveEvent(QEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private:
    struct Card {
        QString title;
        QString subtitle;
        QPixmap icon;
        QRectF rect;
        qreal hover = 0.0;
    };

    Card m_cards[2];
    int m_hoveredIndex = -1;

    void layoutCards();
    void launchGame(int index);
};
