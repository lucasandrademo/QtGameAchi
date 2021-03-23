#ifndef ACHITP_H
#define ACHITP_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class Achitp; }
QT_END_NAMESPACE

class Hole;

class Achitp : public QMainWindow
{
    Q_OBJECT

public:
    enum Player {
      RedPlayer,
      BluePlayer
    };
    Q_ENUM(Player)

    enum Phase {
      Drop,
      Move
    };
    Q_ENUM(Phase)

    Achitp(QWidget *parent = nullptr);
    virtual ~Achitp();

    Hole* holeAt(int row, int col) const;

signals:
    void gameOver(Player player);

private:
    Ui::Achitp *ui;
    Hole* m_hole[9];
    Player m_player;
    Phase m_phase;
    int m_dropCount;
    Hole* m_selected;

    void drop(Hole* hole);
    void move(Hole* hole);
    void clearSelectable();
    QList<Hole*> findSelectable(Hole* hole);

    void switchPlayer();
    bool checkRow(Player player, int col);
    bool checkCol(Player player, int row);
    bool checkMainDiagonal(Player player);
    bool checkSideDiagonal(Player player);
    bool isGameOver(Hole* hole);

private slots:
    void play(int i);
    void NewGame();
    void About();
    void updateStatusBar();
    void showGameOver(Player player);
};
#endif // ACHITP_H
