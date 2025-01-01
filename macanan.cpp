#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Position {
public:
    int x, y;
    
    Position(int x, int y) : x(x), y(y) {}

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

class Piece {
public:
    string pieceType; 
    Position position;

    Piece(string type, Position pos)
        : pieceType(type), position(pos) {}
};

class MacananGameState {
private:
    const int BOARD_WIDTH = 9; 
    const int BOARD_HEIGHT = 7; 
    vector<Piece> pieces;
    string currentTurn;    
    int remainingUwong;          
    int remainingMacan;
    string gameStatus;     
    string winner;          

    bool playable[7][9] = {
        {false, false, true,  true,  true,  true,  true,  false, false},
        {true,  false, true,  true,  true,  true,  true,  false, true },
        {false, true,  false, false, false, false, false, true,  false},
        {true,  true,  true,  true,  true,  true,  true,  true,  true },
        {false, true,  false, false, false, false, false, true,  false},
        {true,  false, true,  true,  true,  true,  true,  false, true },
        {false, false, true,  true,  true,  true,  true,  false, false}
    };

public:
    MacananGameState()
        : currentTurn("macan"),
          remainingUwong(8),
          gameStatus("PLACING"),
          winner(""),
          remainingMacan(2)
          {}


    bool addPiece(const string& pieceType, int x, int y) {
        bool valid = false;
        if (!isPlayablePosition(x, y)) {
            cout << "Invalid position! (" << x << ", " << y << ") is not playable." << endl;
            return valid;
        }

        Position newPos(x, y);

        if (isPositionOccupied(newPos)) {
            cout << "Position already occupied at (" << x << ", " << y << ")." << endl;
            return valid;
        }



        if (pieceType == "uwong" && remainingUwong > 0) {
            remainingUwong--;
            pieces.emplace_back(pieceType, newPos);
            valid = true;
        }else if(pieceType == "macan" && remainingMacan > 0){
            remainingMacan--;
            pieces.emplace_back(pieceType, newPos);
            valid = true;
        }

        return valid;
    }

    bool isPlayablePosition(int x, int y) const {
        if (x < 0 || x >= BOARD_WIDTH || y < 0 || y >= BOARD_HEIGHT) {
            return false;
        }
        return playable[y][x];
    }

    bool isPositionOccupied(const Position& pos) const {
        for (const auto& piece : pieces) {
            if (piece.position == pos) {
                return true;
            }
        }
        return false;
    }

    void printBoard() const {
        char board[BOARD_HEIGHT][BOARD_WIDTH] = {
            {' ', ' ', '.', '.', '.', '.', '.', ' ', ' '},
            {'.', ' ', '.', '.', '.', '.', '.', ' ', '.'},
            {' ', '.', ' ', ' ', ' ', ' ', ' ', '.', ' '},
            {'.', '.', '.', '.', '.', '.', '.', '.', '.'},
            {' ', '.', ' ', ' ', ' ', ' ', ' ', '.', ' '},
            {'.', ' ', '.', '.', '.', '.', '.', ' ', '.'},
            {' ', ' ', '.', '.', '.', '.', '.', ' ', ' '}
        };

        for (const auto& piece : pieces) {
            if (piece.pieceType == "macan") {
                board[piece.position.y][piece.position.x] = 'M';
            } else if (piece.pieceType == "uwong") {
                board[piece.position.y][piece.position.x] = 'U';
            }
        }
        cout << "=== BOARD ===" << endl;
        for (int y = 0; y < BOARD_HEIGHT; y++) {
            for (int x = 0; x < BOARD_WIDTH; x++) {
                if(board[y][x]=='.'){
                    cout<<"("<<x<<","<<y<<")";
                }else{
                    cout << "  "<<  board[y][x] << "  ";
                }
            }
            cout << endl;
        }
        cout << "=============" << endl;
    }

    void switchTurn() {
        currentTurn = (currentTurn == "macan") ? "uwong" : "macan";
    }

    string getCurrentTurn() const {
        return currentTurn;
    }
    void moveOneStep(int fromX, int fromY, int toX, int toY) {
        for(auto &piece : pieces) {
            if(piece.position.x == fromX && piece.position.y == fromY) {
                piece.position.x = toX;
                piece.position.y = toY;
                break;
            }
        }
    }
};

int main() {
    MacananGameState game;
    string pieceType;
    int x, y;

    game.printBoard();

    while (true) {
        cout << "Current turn: " << game.getCurrentTurn() << endl;
        bool input =true;
        while(input){
            cout << "Enter position (x y): ";
            cin >> x >> y;

            if (game.addPiece(game.getCurrentTurn(), x, y)) {
                game.printBoard();
                game.switchTurn();
                input=false;
            }else{

            }
        }
    }

    cout << "Game Over!" << endl;
    return 0;
}
