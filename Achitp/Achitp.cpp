#include "Achitp.h"
#include "ui_Achitp.h"

#include <QDebug>
#include <QSignalMapper>
#include <QMessageBox>

Achitp::Player state2player(Hole::State state) {
    switch (state) {
        case Hole::Red:
            return Achitp::RedPlayer;
        case Hole::Blue:
            return Achitp::BluePlayer;
        default:
            Q_UNREACHABLE();
    }
}

Hole::State player2state(Achitp::Player player) {
    return player == Achitp::RedPlayer ? Hole::Red : Hole::Blue;
}

Achitp::Achitp(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::Achitp),
      m_player(Achitp::RedPlayer),
      m_phase(Achitp::Drop),
      m_dropCount(0),
      m_selected(nullptr)
{
    ui->setupUi(this);

    this->adjustSize();
    this->setFixedSize(this->size());

    QObject::connect(ui->actionExit, SIGNAL(triggered(bool)), qApp, SLOT(quit()));
    QObject::connect(ui->actionNew, SIGNAL(triggered(bool)), this, SLOT(NewGame()));
    QObject::connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(About()));
    QObject::connect(this, SIGNAL(gameOver(Player)), this, SLOT(showGameOver(Player)));
    QObject::connect(this, SIGNAL(gameOver(Player)), this, SLOT(NewGame()));

    QSignalMapper* map = new QSignalMapper(this);

    for (int i = 0; i < 9; i++) {
        int r = i/3;
        int c = i%3;

        Hole* hole = this->findChild<Hole*>(QString("hole%1%2").arg(r).arg(c));
        Q_ASSERT(hole != 0);

        m_hole[i] = hole;
        map->setMapping(hole, i);
        QObject::connect(hole, SIGNAL(clicked(bool)), map, SLOT(map()));
    }
    QObject::connect(map, SIGNAL(mapped(int)), this, SLOT(play(int)));
    this->updateStatusBar();

}

Achitp::~Achitp()
{
    delete ui;
}

Hole* Achitp::holeAt(int row, int col) const {
    if (row >= 0 && row < 3 &&
        col >= 0 && col < 3) {
        int id = row * 3 + col;
        return m_hole[id];
    } else {
        return 0;
    }
}

void Achitp::drop(Hole* hole){

    if (hole->state() == Hole::Empty) {
            hole->setState(player2state(m_player));

            if(isGameOver(hole)){
                emit gameOver(m_player);
            }else{
                ++m_dropCount;
                if (m_dropCount == 6)
                    m_phase = Achitp::Move;

                this->switchPlayer();
            }
        }

}

void Achitp::move(Hole* hole){
    QPair<Hole*,Hole*>* movement = nullptr;

    if (hole->state() == Hole::Selectable) {
        Q_ASSERT(m_selected != 0);
        movement = new QPair<Hole*,Hole*>(m_selected, hole);
    } else {
        if (hole->state() == player2state(m_player)) {
            QList<Hole*> selectable = this->findSelectable(hole);
            if (selectable.count() == 1) {
                movement = new QPair<Hole*,Hole*>(hole, selectable.at(0));
            } else if (selectable.count() > 1) {
                this->clearSelectable();
                foreach (Hole* tmp, selectable)
                    tmp->setState(Hole::Selectable);

                m_selected = hole;
            }
        }
    }

    if (movement != nullptr) {
        this->clearSelectable();
        m_selected = 0;

        Q_ASSERT(movement->first->state() == player2state(m_player));
        Q_ASSERT(movement->second->state() == Hole::Empty);

        movement->first->setState(Hole::Empty);
        movement->second->setState(player2state(m_player));

        if(isGameOver(movement->second)){
            emit gameOver(m_player);
        }else{
            this->switchPlayer();

        delete movement;
        }
    }
}

bool isSelectable(Hole* hole) {
    return hole != nullptr &&
            (hole->state() == Hole::Empty ||
             hole->state() == Hole::Selectable);
}

void Achitp::clearSelectable() {
    for (int i = 0; i < 9; i++) {
        Hole* hole = m_hole[i];
        if (hole->state() == Hole::Selectable)
            hole->setState(Hole::Empty);
    }
}

QList<Hole*> Achitp::findSelectable(Hole* hole){

    QList<Hole*> list;

    Hole* left = this->holeAt(hole->row() - 1, hole->col());
    if (isSelectable(left))
        list << left;

    Hole* up = this->holeAt(hole->row(), hole->col() - 1);
    if (isSelectable(up))
        list << up;

    Hole* right = this->holeAt(hole->row() + 1, hole->col());
    if (isSelectable(right))
        list << right;

    Hole* bottom = this->holeAt(hole->row(), hole->col() + 1);
    if (isSelectable(bottom))
        list << bottom;

    Hole* AllUp = holeAt(0, 1);
    Hole* AllDown = holeAt(2, 1);
    Hole* AllLeft = holeAt(1, 0);
    Hole* AllRight = holeAt(1, 2);
    Hole* now = this->holeAt(hole->row(), hole->col());

    Hole* upleft = this->holeAt(hole->row() - 1, hole->col() - 1);
    if (isSelectable(upleft) && now != AllUp && now != AllDown && now != AllLeft && now != AllRight)
        list << upleft;

    Hole* upright = this->holeAt(hole->row() + 1, hole->col() - 1);
    if (isSelectable(upright) && now != AllUp && now != AllDown && now != AllLeft && now != AllRight)
        list << upright;

    Hole* bottomleft = this->holeAt(hole->row() - 1, hole->col() + 1);
    if (isSelectable(bottomleft) && now != AllUp && now != AllDown && now != AllLeft && now != AllRight)
        list << bottomleft;

    Hole* bottomright = this->holeAt(hole->row() + 1, hole->col() + 1);
    if (isSelectable(bottomright) && now != AllUp && now != AllDown && now != AllLeft && now != AllRight)
        list << bottomright;

    return list;
}

void Achitp::switchPlayer(){
    m_player = m_player == Achitp::RedPlayer ? Achitp::BluePlayer : Achitp::RedPlayer;
    this->updateStatusBar();
}

void Achitp::play(int i){

    Hole* hole = m_hole[i];
    Q_ASSERT(hole != 0);
    if(m_phase == Achitp::Drop){
        drop(hole);
    }else if(m_phase == Achitp::Move){
        move(hole);
    }else{
        Q_UNREACHABLE();
    }
}

bool Achitp::checkRow(Player player, int col){
    Hole::State currentstate = player2state(player);
    for(int r = 0; r < 3; r++){
        Hole* newState = this->holeAt(r, col);
        Q_ASSERT(newState != 0);

        switch (newState->state()) {
            case Hole::Red:
            case Hole::Blue:
                if (currentstate != newState->state())
                    return false;

                    break;
                default:
                    return false;
                }
    }
    return true;
}

bool Achitp::checkCol(Player player, int row){
    Hole::State currentstate = player2state(player);
    for(int c = 0; c < 3; c++){
        Hole* newState = this->holeAt(row, c);
        Q_ASSERT(newState != 0);

        switch (newState->state()) {
            case Hole::Red:
            case Hole::Blue:
                if (currentstate != newState->state())
                    return false;

                    break;
                default:
                    return false;
                }
    }
    return true;
}

bool Achitp::checkMainDiagonal(Player player){
    Hole::State currentstate = player2state(player);
    for(int rc = 0; rc < 3; rc++){
        Hole* newState = this->holeAt(rc, rc);
        Q_ASSERT(newState != 0);

        switch (newState->state()) {
            case Hole::Red:
            case Hole::Blue:
                if (currentstate != newState->state())
                    return false;

                    break;
                default:
                    return false;
                }
    }
    return true;
}

bool Achitp::checkSideDiagonal(Player player){
    Hole::State currentstate = player2state(player);
    int c = 2;
    for(int r = 0; r < 3; r++){
        Hole* newState = this->holeAt(r, c);
        Q_ASSERT(newState != 0);

        switch (newState->state()) {
            case Hole::Red:
            case Hole::Blue:
                if (currentstate != newState->state())
                    return false;

                    break;
                default:
                    return false;
                }
         c--;
    }
    return true;
}

void Achitp::NewGame(){

    for(int i = 0; i < 9; i++){
        Hole* hole = m_hole[i];
        Q_ASSERT(hole != 0);

        hole->NewGame();

        m_dropCount = 0;
        m_player = Achitp::RedPlayer;
        m_phase = Achitp::Drop;
        m_selected = nullptr;
        this->updateStatusBar();
    }

}

void Achitp::About(){
    QMessageBox::information(this, "About", "Achi Game\n\nBruno Rocha Ribeiro: \nbrunor.ribeiro96@gmail.com\n\nLucas Machado de Oliveira Andrade: \nandradelucasmo@gmail.com");
}

void Achitp::updateStatusBar() {
    QString player(m_player == Achitp::RedPlayer ? "Red" : "Blue");
    QString phase(m_phase == Achitp::Drop ? "drop" : "move");

    ui->statusbar->showMessage(tr("%1 phase: %2Player, %3 your piece").arg(phase).arg(player).arg(phase));
}

bool Achitp::isGameOver(Hole* hole) {
    Achitp::Player player = state2player(hole->state());
    return this->checkRow(player, hole->col()) || this->checkCol(player, hole->row()) || this->checkMainDiagonal(player) || this->checkSideDiagonal(player);
}

void Achitp::showGameOver(Player player){

    switch (player) {
            case Achitp::RedPlayer:
                QMessageBox::information(this, "YOU WIN", "CONGRATULATIONS\nRED PLAYER!!!");
                break;
            case Achitp::BluePlayer:
                QMessageBox::information(this, "YOU WIN", "CONGRATULATIONS\nBLUE PLAYER!!!");
                break;
            default:
                Q_UNREACHABLE();
        }
}
