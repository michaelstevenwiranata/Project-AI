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
        string macanPhase;  // "PLACING" atau "MOVING"
        string uwongPhase;  // "PLACING" atau "MOVING"
        string humanPlayer;    // "macan" atau "uwong"
        string aiPlayer;       // "macan" atau "uwong"

        bool playable[7][9] = {
            {false, false, true,  true,  true,  true,  true,  false, false},
            {true,  false, true,  true,  true,  true,  true,  false, true },
            {false, true,  false, false, false, false, false, true,  false},
            {true,  true,  true,  true,  true,  true,  true,  true,  true },
            {false, true,  false, false, false, false, false, true,  false},
            {true,  false, true,  true,  true,  true,  true,  false, true },
            {false, false, true,  true,  true,  true,  true,  false, false}
        };

        // Helper function untuk mengecek apakah ada Uwong yang bisa dimakan
        bool canEatUwong(const Position& macanPos) const {
            // Arah gerakan yang mungkin (horizontal, vertikal, dan diagonal)
            const int directions[8][2] = {
                {0, 2}, {2, 0}, {0, -2}, {-2, 0},  // horizontal & vertikal
                {2, 2}, {2, -2}, {-2, 2}, {-2, -2}  // diagonal
            };

            for (const auto& dir : directions) {
                int midX = macanPos.x + dir[0]/2;
                int midY = macanPos.y + dir[1]/2;
                int targetX = macanPos.x + dir[0];
                int targetY = macanPos.y + dir[1];

                // Cek apakah posisi target valid
                if (!isPlayablePosition(targetX, targetY)) continue;

                // Cek apakah ada Uwong di tengah dan posisi target kosong
                bool uwongInMiddle = false;
                bool targetEmpty = true;

                for (const auto& piece : pieces) {
                    if (piece.position.x == midX && piece.position.y == midY && piece.pieceType == "uwong") {
                        uwongInMiddle = true;
                    }
                    if (piece.position.x == targetX && piece.position.y == targetY) {
                        targetEmpty = false;
                    }
                }

                if (uwongInMiddle && targetEmpty) return true;
            }
            return false;
        }

        // Menghitung skor posisi berdasarkan jarak dari tengah
        int calculatePositionScore(const Position& pos) const {
            // Pusat board
            const int centerX = BOARD_WIDTH / 2;
            const int centerY = BOARD_HEIGHT / 2;
            
            // Semakin dekat ke tengah, semakin tinggi skornya
            return 10 - (abs(pos.x - centerX) + abs(pos.y - centerY));
        }
        

        // Helper function untuk mendapatkan semua gerakan yang mungkin dari suatu posisi
        vector<Position> getValidMoves(const Position& pos) const {
            vector<Position> validMoves;
            
            // Arah gerakan yang mungkin (horizontal, vertikal, dan diagonal)
            const int directions[8][2] = {
                {0, 1}, {1, 0}, {0, -1}, {-1, 0},  // horizontal & vertikal
                {1, 1}, {1, -1}, {-1, 1}, {-1, -1}  // diagonal
            };

            for (const auto& dir : directions) {
                int newX = pos.x + dir[0];
                int newY = pos.y + dir[1];
                
                // Cek apakah posisi baru valid dan bisa dimainkan
                if (isPlayablePosition(newX, newY) && !isPositionOccupied(Position(newX, newY))) {
                    validMoves.emplace_back(newX, newY);
                }
            }

            return validMoves;
        }

        // Menghitung skor untuk Uwong
        int evaluateUwongPosition() const {
            int score = 0;
            vector<Position> uwongPositions;
            vector<Position> macanPositions;

            // Kumpulkan posisi semua piece
            for (const auto& piece : pieces) {
                if (piece.pieceType == "uwong") {
                    uwongPositions.push_back(piece.position);
                } else {
                    macanPositions.push_back(piece.position);
                }
            }

            // Bonus untuk jumlah Uwong yang tersisa
            score += uwongPositions.size() * 100;

            // Bonus untuk formasi berkelompok (saling melindungi)
            for (const auto& uwong1 : uwongPositions) {
                for (const auto& uwong2 : uwongPositions) {
                    if (uwong1 == uwong2) continue;
                    int distance = abs(uwong1.x - uwong2.x) + abs(uwong1.y - uwong2.y);
                    if (distance == 1) score += 20; // Bonus untuk Uwong yang berdekatan
                }
            }

            // Penalti untuk Uwong yang terlalu dekat dengan Macan
            for (const auto& uwong : uwongPositions) {
                for (const auto& macan : macanPositions) {
                    int distance = abs(uwong.x - macan.x) + abs(uwong.y - macan.y);
                    if (distance == 1) score -= 30; // Penalti jika terlalu dekat dengan Macan
                }
            }

            return score;
        }

        // Menghitung skor untuk Macan
        int evaluateMacanPosition() const {
            int score = 0;
            Position macanPos(-1, -1);
            vector<Position> uwongPositions;

            // Kumpulkan posisi semua piece
            for (const auto& piece : pieces) {
                if (piece.pieceType == "macan") {
                    macanPos = piece.position;
                } else {
                    uwongPositions.push_back(piece.position);
                }
            }

            // Bonus tinggi jika bisa memakan Uwong
            if (canEatUwong(macanPos)) {
                score += 1000;
            }

            // Bonus untuk posisi strategis di tengah
            score += calculatePositionScore(macanPos) * 5;

            // Hitung jarak ke Uwong terdekat
            int minDistanceToUwong = 999;
            for (const auto& uwongPos : uwongPositions) {
                int distance = abs(macanPos.x - uwongPos.x) + abs(macanPos.y - uwongPos.y);
                minDistanceToUwong = min(minDistanceToUwong, distance);
            }

            // Penalti jika terlalu dekat dengan banyak Uwong
            if (minDistanceToUwong == 1) {
                score -= 50;
            }

            return score;
        }

    public:
        MacananGameState()
            : currentTurn("macan"),
            remainingUwong(8),
            remainingMacan(2),
            macanPhase("PLACING"),
            uwongPhase("PLACING"),
            winner(""),
            humanPlayer(""),
            aiPlayer("")
        {}

        void setPlayers(const string& human) {
            humanPlayer = human;
            aiPlayer = (human == "macan") ? "uwong" : "macan";
        }

        bool isAITurn() const {
            return currentTurn == aiPlayer;
        }

        // Evaluasi board untuk AI
        int evaluateBoard() const {
            if (aiPlayer == "macan") {
                return evaluateMacanPosition() - evaluateUwongPosition();
            } else {
                return evaluateUwongPosition() - evaluateMacanPosition();
            }
        }

        // AI membuat gerakan
        void makeAIMove() {
            if (getGamePhase() == "PLACING") {
                makeAIPlacement();
                
            } else {
                makeAIMovement();
            }
        }

        string getGamePhase() const {
            return currentTurn == "macan" ? macanPhase : uwongPhase;
        }

        bool canMove(const string& playerType) const {
            if (playerType == "macan") {
                return macanPhase == "MOVING";
            } else {  // uwong
                return uwongPhase == "MOVING";
            }
        }

        bool addPiece(const string& pieceType, int x, int y) {
            // Cek apakah pemain sudah dalam fase MOVING
            if ((pieceType == "macan" && macanPhase == "MOVING") ||
                (pieceType == "uwong" && uwongPhase == "MOVING")) {
                cout << "Cannot place pieces after completing placement phase!" << endl;
                return false;
            }

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
                
                // Check if Uwong placement is complete
                if (remainingUwong == 0) {
                    uwongPhase = "MOVING";
                    cout << "Uwong placement complete. Uwong can now move!" << endl;
                }
            } else if (pieceType == "macan" && remainingMacan > 0) {
                remainingMacan--;
                pieces.emplace_back(pieceType, newPos);
                valid = true;
                
                // Check if Macan placement is complete
                if (remainingMacan == 0) {
                    macanPhase = "MOVING";
                    cout << "Macan placement complete. Macan can now move!" << endl;
                }
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
            for (int y = 0; y < BOARD_HEIGHT; y++) {
                for (int x = 0; x < BOARD_WIDTH; x++) {
                    if(board[y][x]=='.'){
                        cout<<"("<<x<<","<<y<<")";
                    }else if(board[y][x]==' '){
                        cout<<"     ";
                    }else{
                        cout <<board[y][x] <<"("<<x<<","<<y<<")";
                    }
                }
                cout << endl;
            }
        }

        void switchTurn() {
            currentTurn = (currentTurn == "macan") ? "uwong" : "macan";
        }

        string getCurrentTurn() const {
            return currentTurn;
        }
        
        

        int evaluatePosition() {
            int score = 0;
            Position macanPos(-1, -1);
            vector<Position> uwongPositions;

            // Mengumpulkan posisi semua piece
            for (const auto& piece : pieces) {
                if (piece.pieceType == "macan") {
                    macanPos = piece.position;
                } else if (piece.pieceType == "uwong") {
                    uwongPositions.push_back(piece.position);
                }
            }

            // Jika bisa memakan Uwong, berikan skor tinggi
            if (canEatUwong(macanPos)) {
                score += 1000;
            }

            // Nilai posisi strategis (dekat tengah)
            score += calculatePositionScore(macanPos) * 5;

            // Hitung jarak aman dari Uwong
            int minDistanceToUwong = 999;
            for (const auto& uwongPos : uwongPositions) {
                int distance = abs(macanPos.x - uwongPos.x) + abs(macanPos.y - uwongPos.y);
                minDistanceToUwong = min(minDistanceToUwong, distance);
            }

            // Jika terlalu dekat dengan banyak Uwong, kurangi skor
            if (minDistanceToUwong == 1) {
                score -= 50;
            }

            return score;
        }

        bool movePiece(const Position& from, const Position& to) {
            // Cek apakah pemain sudah bisa bergerak
            string currentPieceType = "";
            for (const auto& piece : pieces) {
                if (piece.position == from) {
                    currentPieceType = piece.pieceType;
                    break;
                }
            }

            if (currentPieceType == "macan" && macanPhase != "MOVING") {
                cout << "Macan must complete placement phase first!" << endl;
                return false;
            } else if (currentPieceType == "uwong" && uwongPhase != "MOVING") {
                cout << "Uwong must complete placement phase first!" << endl;
                return false;
            }

            // Cek apakah ada pion di posisi awal
            int pieceIndex = -1;
            for (size_t i = 0; i < pieces.size(); i++) {
                if (pieces[i].position == from) {
                    pieceIndex = i;
                    break;
                }
            }
            
            if (pieceIndex == -1) {
                cout << "No piece at starting position!" << endl;
                return false;
            }

            // Cek apakah gerakan valid
            vector<Position> validMoves = getValidMoves(from);
            bool isValidMove = false;
            for (const auto& move : validMoves) {
                if (move == to) {
                    isValidMove = true;
                    break;
                }
            }

            // Cek apakah ini gerakan memakan untuk Macan
            if (!isValidMove && pieces[pieceIndex].pieceType == "macan") {
                // Cek gerakan makan (2 langkah)
                int dx = to.x - from.x;
                int dy = to.y - from.y;
                
                // Harus bergerak 2 langkah
                if (abs(dx) == 2 || abs(dy) == 2) {
                    Position middle(from.x + dx/2, from.y + dy/2);
                    
                    // Cek apakah ada Uwong di tengah
                    bool uwongInMiddle = false;
                    int uwongIndex = -1;
                    for (size_t i = 0; i < pieces.size(); i++) {
                        if (pieces[i].position == middle && pieces[i].pieceType == "uwong") {
                            uwongInMiddle = true;
                            uwongIndex = i;
                            break;
                        }
                    }

                    if (uwongInMiddle && !isPositionOccupied(to) && isPlayablePosition(to.x, to.y)) {
                        // Hapus Uwong yang dimakan
                        pieces.erase(pieces.begin() + uwongIndex);
                        isValidMove = true;
                    }
                }
            }

            if (!isValidMove) {
                cout << "Invalid move!" << endl;
                return false;
            }

            // Lakukan gerakan
            pieces[pieceIndex].position = to;
            return true;
        }

        // Helper function untuk menampilkan gerakan yang mungkin
        void showPossibleMoves(const Position& pos) const {
            vector<Position> moves = getValidMoves(pos);
            cout << "Possible moves from (" << pos.x << "," << pos.y << "):" << endl;
            for (const auto& move : moves) {
                cout << "-> (" << move.x << "," << move.y << ")" << endl;
            }
        }

        // Helper function untuk menampilkan status fase
        void printGameStatus() const {
            cout << "Current turn: " << currentTurn << endl;
            cout << "Macan phase: " << macanPhase << 
                " (Remaining: " << remainingMacan << ")" << endl;
            cout << "Uwong phase: " << uwongPhase << 
                " (Remaining: " << remainingUwong << ")" << endl;
        }

    private:
        void makeAIPlacement() {
            // Implementasi sederhana: cari posisi kosong yang valid dengan skor tertinggi
            int bestScore = -999999;
            Position bestPos(-1, -1);

            for (int y = 0; y < BOARD_HEIGHT; y++) {
                for (int x = 0; x < BOARD_WIDTH; x++) {
                    if (isPlayablePosition(x, y) && !isPositionOccupied(Position(x, y))) {
                        // Coba tempatkan piece
                        addPiece(aiPlayer, x, y);
                        int score = evaluateBoard();
                        
                        // Undo placement
                        pieces.pop_back();
                        if (aiPlayer == "macan") remainingMacan++;
                        else remainingUwong++;

                        if (score > bestScore) {
                            bestScore = score;
                            bestPos = Position(x, y);
                        }
                    }
                }
            }

            if (bestPos.x != -1) {
                addPiece(aiPlayer, bestPos.x, bestPos.y);
                cout << "AI places " << aiPlayer << " at (" << bestPos.x << "," << bestPos.y << ")" << endl;
            }
        }

        void makeAIMovement() {
            // Implementasi sederhana: cari gerakan dengan skor tertinggi
            int bestScore = -999999;
            Position bestFrom(-1, -1);
            Position bestTo(-1, -1);

            // Cari semua piece milik AI
            for (const auto& piece : pieces) {
                if (piece.pieceType == aiPlayer) {
                    vector<Position> validMoves = getValidMoves(piece.position);
                    
                    for (const auto& move : validMoves) {
                        // Coba gerakan
                        Position originalPos = piece.position;
                        movePiece(piece.position, move);
                        int score = evaluateBoard();
                        
                        // Undo gerakan
                        for (auto& p : pieces) {
                            if (p.position == move) {
                                p.position = originalPos;
                                break;
                            }
                        }

                        if (score > bestScore) {
                            bestScore = score;
                            bestFrom = piece.position;
                            bestTo = move;
                        }
                    }
                }
            }

            if (bestFrom.x != -1) {
                movePiece(bestFrom, bestTo);
                cout << "AI moves " << aiPlayer << " from (" << bestFrom.x << "," << bestFrom.y 
                     << ") to (" << bestTo.x << "," << bestTo.y << ")" << endl;
            }
        }
    };

    int main() {
        MacananGameState game;
        string playerChoice;
        
        // Pilih pemain
        while (playerChoice != "macan" && playerChoice != "uwong") {
            cout << "Choose your player (macan/uwong): ";
            cin >> playerChoice;
        }
        
        game.setPlayers(playerChoice);
        
        while (true) {
            game.printBoard();
            game.printGameStatus();

            if (game.isAITurn()) {
                cout << "AI's turn" << endl;
                game.makeAIMove();
                game.switchTurn();
            } else {
                string currentPhase = game.getGamePhase();
                
                if (currentPhase == "PLACING") {
                    int x, y;
                    bool validPlacement = false;
                    while (!validPlacement) {
                        cout << "Enter position to place (x y): ";
                        cin >> x >> y;
                        validPlacement = game.addPiece(playerChoice, x, y);
                    }
                } else {  // MOVING phase
                    string command;
                    cout << "Enter command (move/quit): ";
                    cin >> command;

                    if (command == "quit") break;
                    
                    if (command == "move") {
                        int fromX, fromY, toX, toY;
                        cout << "Enter start position (x y): ";
                        cin >> fromX >> fromY;
                        cout << "Enter destination position (x y): ";
                        cin >> toX >> toY;

                        if (game.movePiece(Position(fromX, fromY), Position(toX, toY))) {
                            cout << "Move successful!" << endl;
                        }
                    }
                }
                game.switchTurn();
            }
        }

        cout << "Game Over!" << endl;
        return 0;
    }
