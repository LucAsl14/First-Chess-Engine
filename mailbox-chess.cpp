/**************************\
                         
      0x88 mailbox chess   
                         
         By Lucas         
\**************************/

// check out @attention places for extra coding opportunities

/**
 * TODO: LVA MVV, 3-fold prevention, transposition tables, "contempt factor", opening repertoire, remove fen history when capturing
 *       put checks first
*/

// headers
#include <stdio.h> 
#include <map>
#include <iostream>
#include <string.h>
#include <vector>

using namespace std;

// FEN debugging
char startingPosition[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
char otherPosition[] = "r3k2r/p1pp1pb1/bn2Qnp1/2qPN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQkq - 3 2";

// FEN storing
vector<string> fenHistory;

// piece encoding (piece -> int)
enum pieces { e, P, N, B, R, Q, K, p, n, b, r, q, k, o};

// square encoding (square -> int)
enum squares {
    a8 = 0,   b8, c8, d8, e8, f8, g8, h8, 
    a7 = 16,  b7, c7, d7, e7, f7, g7, h7, 
    a6 = 32,  b6, c6, d6, e6, f6, g6, h6, 
    a5 = 48,  b5, c5, d5, e5, f5, g5, h5, 
    a4 = 64,  b4, c4, d4, e4, f4, g4, h4, 
    a3 = 80,  b3, c3, d3, e3, f3, g3, h3, 
    a2 = 96,  b2, c2, d2, e2, f2, g2, h2, 
    a1 = 112, b1, c1, d1, e1, f1, g1, h1, noSq
};

// -----------------  CUSTOMIZATION -------------------------
/*
    Material score
    P - 100
    N - 300
    B - 350
    R - 500
    Q - 900
    K - 10000
*/
int materialScore[13] = {
    0, 
     100,  300,  350,  500,  900,  50000,
    -100, -300, -350, -500, -900, -50000
};
// pawn positional score
const int pawnScore[128] = {
     0,  0,  0,  0,  0,  0,  0,  0,  o, o, o, o, o, o, o, o,
    50, 50, 50, 50, 50, 50, 50, 50,  o, o, o, o, o, o, o, o,
    40, 40, 40, 40, 40, 40, 40, 40,  o, o, o, o, o, o, o, o,
    10, 20, 20, 30, 30, 20, 20, 10,  o, o, o, o, o, o, o, o,
     0,  5, 10, 20, 20, 10,  5,  0,  o, o, o, o, o, o, o, o,
     0,  0,  0,  5,  5,  0,  0,  0,  o, o, o, o, o, o, o, o,
     0,  0,  0,-10,-10,  0,  0,  0,  o, o, o, o, o, o, o, o,
     0,  0,  0,  0,  0,  0,  0,  0,  o, o, o, o, o, o, o, o, 
};
// knight positional score
const int knightScore[128] = {
    -5, -5, -5, -5, -5, -5, -5, -5,  o, o, o, o, o, o, o, o,
    -5,  0,  0,  5,  5,  0,  0, -5,  o, o, o, o, o, o, o, o,
    -5,  0,  5, 15, 15,  5,  0, -5,  o, o, o, o, o, o, o, o,
    -5,  5, 15, 30, 30, 15,  5, -5,  o, o, o, o, o, o, o, o,
    -5,  5, 15, 30, 30, 15,  5, -5,  o, o, o, o, o, o, o, o,
    -5,  0, 10, 10, 10, 10,  0, -5,  o, o, o, o, o, o, o, o,
    -5,  0,  0,  5,  5,  0,  0, -5,  o, o, o, o, o, o, o, o,
    -5,-15, -5, -5, -5, -5,-15, -5,  o, o, o, o, o, o, o, o,
};
// bishop positional score
const int bishopScore[128] = {
     0,  0,  0,  0,  0,  0,  0,  0,  o, o, o, o, o, o, o, o,
     0,  0,  0,  0,  0,  0,  0,  0,  o, o, o, o, o, o, o, o,
     0,  0,  0, 10, 10,  0,  0,  0,  o, o, o, o, o, o, o, o,
     0,  0, 10, 20, 20, 10,  0,  0,  o, o, o, o, o, o, o, o,
     0,  0, 10, 20, 20, 10,  0,  0,  o, o, o, o, o, o, o, o,
     0, 10,  0,  0,  0,  0, 10,  0,  o, o, o, o, o, o, o, o,
     0, 30,  0,  0,  0,  0, 30,  0,  o, o, o, o, o, o, o, o,
     0,  0,-20,  0,  0,-20,  0,  0,  o, o, o, o, o, o, o, o,
};
// rook positional score
const int rookScore[128] = {
    50, 50, 50, 50, 50, 50, 50, 50,  o, o, o, o, o, o, o, o,
    50, 50, 50, 50, 50, 50, 50, 50,  o, o, o, o, o, o, o, o,
     0,  0, 10, 20, 20, 10,  0,  0,  o, o, o, o, o, o, o, o,
     0,  0, 10, 20, 20, 10,  0,  0,  o, o, o, o, o, o, o, o,
     0,  0, 10, 20, 20, 10,  0,  0,  o, o, o, o, o, o, o, o,
     0,  0, 10, 20, 20, 10,  0,  0,  o, o, o, o, o, o, o, o,
     0,  0, 10, 20, 20, 10,  0,  0,  o, o, o, o, o, o, o, o,
     0,  0,  0, 20, 20,  0,  0,  0,  o, o, o, o, o, o, o, o,
};
// king positional score
const int kingScore[128] = {
     0,  0,  0,  0,  0,  0,  0,  0,  o, o, o, o, o, o, o, o,
     0,  0,  5,  5,  5,  5,  0,  0,  o, o, o, o, o, o, o, o,
     0,  5,  5, 10, 10,  5,  5,  0,  o, o, o, o, o, o, o, o,
     0,  5, 10, 20, 20, 10,  5,  0,  o, o, o, o, o, o, o, o,
     0,  5, 10, 20, 20, 10,  5,  0,  o, o, o, o, o, o, o, o,
     0,  0,  5, 10, 10,  5,  0,  0,  o, o, o, o, o, o, o, o,
     0,  5,  5,-10,-10,  0,  5,  0,  o, o, o, o, o, o, o, o,
     0,  0,  5,  0,-15,  0, 10,  0,  o, o, o, o, o, o, o, o,
};
// mirror score for other side
int mirrorScore[128] = {
    a1, b1, c1, d1, e1, f1, g1, h1,  o, o, o, o, o, o, o, o, 
    a2, b2, c2, d2, e2, f2, g2, h2,  o, o, o, o, o, o, o, o,
    a3, b3, c3, d3, e3, f3, g3, h3,  o, o, o, o, o, o, o, o,
    a4, b4, c4, d4, e4, f4, g4, h4,  o, o, o, o, o, o, o, o,
    a5, b5, c5, d5, e5, f5, g5, h5,  o, o, o, o, o, o, o, o,
    a6, b6, c6, d6, e6, f6, g6, h6,  o, o, o, o, o, o, o, o,
    a7, b7, c7, d7, e7, f7, g7, h7,  o, o, o, o, o, o, o, o,
    a8, b8, c8, d8, e8, f8, g8, h8,  o, o, o, o, o, o, o, o,
};
// ----------------------------------------------------------

// capture flag
enum captureFlags {allMoves, onlyCaptures};

// castling binary representations enum
// example: 1001 --> white can queenside, black can kingside
// example: 1111 --> both can castle both ways
enum castling { Kc=0b0001, Qc=0b0010, kc=0b0100, qc=0b1000 };

// sides to move
enum sides { white, black };

// ascii pieces
char asciiPieces[] = ".PNBRQKpnbrqko";

// encode ascii pieces (char -> int) (use charPieces.at())
map<char, int> charPieces {
    {'P', P},
    {'N', N},
    {'B', B},
    {'R', R},
    {'Q', Q},
    {'K', K},
    {'p', p},
    {'n', n},
    {'b', b},
    {'r', r},
    {'q', q},
    {'k', k},
};

// decode promoted pieces (int -> char) (use promotedPieces.at())
map<int, char> promotedPieces {
    {N, 'N'},
    {B, 'B'},
    {R, 'R'},
    {Q, 'Q'},
    {n, 'n'},
    {b, 'b'},
    {r, 'r'},
    {q, 'q'},
};

// unicode pieces
const char *unicodePieces[] = {".","♟","♞","♝","♜","♛","♚","♙","♘","♗","♖","♕","♔","o"};

// castling rights
/*
                         castle   move     in      in
                          right    map     binary  decimal
    white king moved ->    1111 & 1100  =  1100    12
    K's rook moved   ->    1111 & 1110  =  1110    14
    Q's rook moved   ->    1111 & 1101  =  1101    13

    black king moved ->    1111 & 0011  =  0011    3
    k's rook moved   ->    1111 & 1011  =  1011    11
    q's rook moved   ->    1111 & 0111  =  0111    7

*/
int castlingRights[128] = {
     7, 15, 15, 15,  3, 15, 15, 11,  o, o, o, o, o, o, o, o,
    15, 15, 15, 15, 15, 15, 15, 15,  o, o, o, o, o, o, o, o,
    15, 15, 15, 15, 15, 15, 15, 15,  o, o, o, o, o, o, o, o,
    15, 15, 15, 15, 15, 15, 15, 15,  o, o, o, o, o, o, o, o,
    15, 15, 15, 15, 15, 15, 15, 15,  o, o, o, o, o, o, o, o,
    15, 15, 15, 15, 15, 15, 15, 15,  o, o, o, o, o, o, o, o,
    15, 15, 15, 15, 15, 15, 15, 15,  o, o, o, o, o, o, o, o,
    13, 15, 15, 15, 12, 15, 15, 14,  o, o, o, o, o, o, o, o
};

// chess board representation
int board[128] = {
    r, n, b, q, k, b, n, r,  o, o, o, o, o, o, o, o,
    p, p, p, p, p, p, p, p,  o, o, o, o, o, o, o, o,
    e, e, e, e, e, e, e, e,  o, o, o, o, o, o, o, o,
    e, e, e, e, e, e, e, e,  o, o, o, o, o, o, o, o,
    e, e, e, e, e, e, e, e,  o, o, o, o, o, o, o, o,
    e, e, e, e, e, e, e, e,  o, o, o, o, o, o, o, o,
    P, P, P, P, P, P, P, P,  o, o, o, o, o, o, o, o,
    R, N, B, Q, K, B, N, R,  o, o, o, o, o, o, o, o
};

// side to move
int side = 0;

// enpassant squares
int enpassant = noSq;

// castling rights (both sides castle both ways)
int castle = 0b1111;

// kings' squares
int kingSq[2] = {e1, e8};

/*
    Move formatting

    0000 0000 0000 0000 0111 1111       source square      
    0000 0000 0011 1111 1000 0000       target square      
    0000 0011 1100 0000 0000 0000       promoted piece
    0000 0100 0000 0000 0000 0000       capture flag
    0000 1000 0000 0000 0000 0000       double pawn flag
    0001 0000 0000 0000 0000 0000       enpassant flag
    0010 0000 0000 0000 0000 0000       castling flag
*/

// encode move
#define encodeMove(source, target, promoted, capture, pawn, enpassant, castling) \
    ((source)|(target<<7)|(promoted<<14)|(capture<<18)|(pawn<<19)|(enpassant<<20)|(castling<<21))

// decode move
#define getMoveSource(move) (move & 0x7f)
#define getMoveTarget(move) ((move>>7) & 0x7f)
#define getMovePromotion(move) ((move>>14) & 0xf)
#define getMoveCapture(move) ((move>>18) & 1)
#define getMoveDoublePawn(move) ((move>>19) & 1)
#define getMoveEnpassant(move) ((move>>20) & 1)
#define getMoveCastling(move) ((move>>21) & 1)

// convert squares to coordinates
const char *squareToCoords[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", "i8", "j8", "k8", "l8", "m8", "n8", "o8", "p8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7", "i7", "j7", "k7", "l7", "m7", "n7", "o7", "p7", 
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", "i6", "j6", "k6", "l6", "m6", "n6", "o6", "p6", 
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", "i5", "j5", "k5", "l5", "m5", "n5", "o5", "p5", 
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4", "i4", "j4", "k4", "l4", "m4", "n4", "o4", "p4", 
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", "i3", "j3", "k3", "l3", "m3", "n3", "o3", "p3", 
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2", "i2", "j2", "k2", "l2", "m2", "n2", "o2", "p2", 
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "i1", "j1", "k1", "l1", "m1", "n1", "o1", "p1"
};

// piece move offsets
int knightOffsets[] = {33, 31, 18, 14, -33, -31, -18, -14};
int bishopOffsets[] = {15, 17, -15, -17};
int rookOffsets[] = {16, -16, 1, -1};
int kingOffsets[] = {15, 16, 17, -1, 1, -15, -16, -17};

// move list structure @param moves[] the list @param count number of moves stored
struct moves {
    // move list
    int moves[256];

    // move count
    int count = 0;

};

// print board
void printBoard(){
    for(int rank=0; rank<8; rank++){
        for(int file=0; file<16; file++){
            // print ranks
            if(file==0){
                printf("%d   ", 8-rank);
            }

            // initialize the square
            int square = rank*16+file;
            if((square & 0x88) == 0)
                printf("%s ", unicodePieces[board[square]]);
        }
        printf("\n");
    }

    // print files
    printf("\n    A B C D E F G H\n");

    // print board stats
    printf("\nSide to move: %s\n", side==white? "white":"black");
    printf("Castling rights: %s%s%s%s\n", castle&Kc?"K":"-", castle&Qc?"Q":"-", castle&kc?"k":"-", castle&qc?"q":"-");
    printf("Enpassant: %s\n", enpassant==noSq?"-":squareToCoords[enpassant]);
    printf("King square: %s\n\n", squareToCoords[kingSq[side==white?white:black]]);
}

static inline bool isSquareAttacked(int sqr, int side){
    // pawn attacks
    if(side==white){
        // on board and white pawn
        if(!((sqr + 17) & 0x88) && board[sqr+17]==P) return true;
        if(!((sqr + 15) & 0x88) && board[sqr+15]==P) return true;
    }
    else {
        // on board and black pawn
        if(!((sqr - 17) & 0x88) && board[sqr-17]==p) return true;
        if(!((sqr - 15) & 0x88) && board[sqr-15]==p) return true;
    }

    // knight attacks
    for(int index = 0; index<8; index++){
        // calculate target square & piece
        int targetSqr = sqr + knightOffsets[index];
        int targetPiece = board[targetSqr];

        // on board and knight matches color
        if(!(targetSqr & 0x88) && targetPiece==(side==white?N:n)) return true;
    }

    // king attacks
    for(int index = 0; index<8; index++){
        // calculate target square & piece
        int targetSqr = sqr + kingOffsets[index];
        int targetPiece = board[targetSqr];

        // on board and king matches color
        if(!(targetSqr & 0x88) && targetPiece==(side==white?K:k)) return true;
    }

    // bishop attacks (also partly queen)
    for(int index = 0; index<4; index++){
        // calculate target square
        int targetSqr = sqr + bishopOffsets[index];

        // loop over ray
        while(!(targetSqr & 0x88)){
            // calculate target piece
            int targetPiece = board[targetSqr];

            // if found bishop or queen matching color
            if(targetPiece==(side==white?B:b)) return true;
            if(targetPiece==(side==white?Q:q)) return true;

            // break if hit a piece
            if(targetPiece!=e) break;

            // increment target square
            targetSqr += bishopOffsets[index];
        }

    }

    // rook attacks (also partly queen)
    for(int index = 0; index<4; index++){
        // calculate target square
        int targetSqr = sqr + rookOffsets[index];

        // loop over ray
        while(!(targetSqr & 0x88)){
            // calculate target piece
            int targetPiece = board[targetSqr];

            // if found bishop or queen matching color
            if(targetPiece==(side==white?R:r)) return true;
            if(targetPiece==(side==white?Q:q)) return true;

            // break if hit a piece
            if(targetPiece!=e) break;

            // increment target square
            targetSqr += rookOffsets[index];
        }

    }
    return false;
}

// print attacked squares
void printAttackedSquares(int side){
    printf("\nAttacking side: %s\n\n", side==white? "white":"black");
    for(int rank=0; rank<8; rank++){
        for(int file=0; file<16; file++){
            // print ranks
            if(file==0){
                printf("%d   ", 8-rank);
            }

            // initialize the square
            int square = rank*16+file;
            if((square & 0x88) == 0)
                printf("%s ", isSquareAttacked(square, side)? "x":".");
        }
        printf("\n");
    }

    // print files
    printf("\n    A B C D E F G H\n");
}

// print move list
void printMoveList(moves *moveList){
    for(int i=0; i<moveList->count; i++){
        int move = moveList->moves[i];
        cout << squareToCoords[getMoveSource(move)] << (getMoveCapture(move)?"X":" ") << squareToCoords[getMoveTarget(move)];
        cout << (getMovePromotion(move)!=0?("="+string(1,promotedPieces.at(getMovePromotion(move)))):"");
        cout << (getMoveDoublePawn(move)?" Double pawn move":"");
        cout << (getMoveEnpassant(move)?" Enpassant capture":"");
        cout << (getMoveCastling(move)?" Castling":"");
        cout << endl;
    }
}

// reset board
void resetBoard(){
    for(int rank=0; rank<8; rank++){
        for(int file=0; file<16; file++){
            // initialize the square
            int square = rank*16+file;
            if((square & 0x88) == 0)
                board[square] = e;
        }
    }
    // reset stats
    side = -1;
    castle = 0;
    enpassant = noSq;
    // reset fen history
    fenHistory.clear();
}

// parse FEN
void parseFEN(const char *fen){
    resetBoard();
    for(int rank=0; rank<8; rank++){
        for(int file=0; file<16; file++){
            // initialize the square
            int square = rank*16+file;

            // if square is on board
            if((square & 0x88) == 0){
                // match pieces
                if ((*fen>='a'&&*fen<='z')||(*fen>='A'&&*fen<='Z')){
                    // king's square
                    if(*fen=='K')
                        kingSq[white] = square;
                    else if(*fen=='k')
                        kingSq[black] = square;

                    // put piece
                    board[square] = charPieces.at(*fen);

                    // increment pointer
                    *fen++;
                }

                // match empty squares
                if(*fen>='0'&&*fen<='9'){
                    // calculate offset
                    int offset = *fen-'0';

                    // edge case: square empty
                    if(board[square]==0) file--;

                    // skip this many tiles
                    file += offset;

                    // increment pointer
                    *fen++;
                }

                // match end of rank
                if(*fen=='/'){
                    // increment pointer
                    *fen++;
                }
            }
        }
    }
    
    // go to side parsing
    *fen++;
    // parse side to move
    side = (*fen=='w') ? white : black;

    // go to castling parsing
    *fen++; *fen++;
    // parse castling rights
    while(*fen!=' '){
        switch(*fen){
            case 'K': castle|=Kc; break;
            case 'Q': castle|=Qc; break;
            case 'k': castle|=kc; break;
            case 'q': castle|=qc; break;
            case '-': break;
        }
        // increment pointer
        *fen++;
    }

    // go to enpassant parsing
    *fen++;
    // enpassant parsing
    if(*fen!='-'){
        // parse square file and rank
        int file = *fen - 'a';
        *fen++;
        int rank = '8' - *fen;
        *fen++;
        // set square info
        enpassant = rank*16+file;
    } else {
        enpassant = noSq;
        *fen++;
    }
    *fen++;
    // printf("%s\n", fen); @attention this is meant to do something with the turns (fullturn, halfturn)
}

// create an FEN
void createFEN(vector<string> &dest){
    // initialize fen string and number of empty squares
    string fen  = "";
    int empty;

    // loop over all on-board squares for board state
    for(int rank=0; rank<8; rank++){
        empty = 0;
        for(int file=0; file<8; file++){
            int square = rank*16+file;
            if(board[square]==e) empty++;
            else{
                if(empty!=0) fen += to_string(empty);
                empty = 0;
                fen += asciiPieces[board[square]];
            } 
        }
        if(empty!=0) fen += to_string(empty);
        fen += "/";
    }
    fen.pop_back();

    // side to move
    fen += " ";
    fen += side? "b":"w";

    // castling rights
    fen += " ";
    fen += (castle & 0b0001)? "K":"";
    fen += (castle & 0b0010)? "Q":"";
    fen += (castle & 0b0100)? "k":"";
    fen += (castle & 0b1000)? "q":"";

    // enpassant
    fen += " ";
    if(board[enpassant]==o) fen += "-";
    else fen += squareToCoords[enpassant];

    dest.push_back(fen);
}

// populate movelist (note, has been modified from moves* (ptr) to moves (struct))
static inline void addMove(moves *moveList, int move){
    // push move into movelist
    moveList->moves[moveList->count] = move;

    // increment move count
    moveList->count++;
}

// move generator
static inline void generateMoves(moves *moveList, bool onlyCaptures){
    // loop over all squares
    for(int sq = 0; sq<128; sq++){
        // if square is on board
        if(!(sq & 0x88)){
            // rook & queen moves
            if(board[sq]==(side==white?R:r) || board[sq]==(side==white?Q:q)){
                // loop over rook offsets
                for(int index=0; index<4; index++){
                    // init target square
                    int targetSq = sq+rookOffsets[index];
                    // loop over attack ray
                    while(!(targetSq & 0x88)){
                        // init target piece
                        int piece = board[targetSq];
                        // if hits own piece
                        if(piece>=(side==white?P:p) && piece<=(side==white?K:k)) break;
                        // if hits enemy piece
                        if(piece>=(side==white?p:P) && piece<=(side==white?k:K)){
                            addMove(moveList,encodeMove(sq, targetSq, 0, 1, 0, 0, 0));
                            break;
                        }
                        if(!onlyCaptures)
                            // if empty
                            addMove(moveList,encodeMove(sq, targetSq, 0, 0, 0, 0, 0));
                        targetSq+=rookOffsets[index];
                    }
                }
            }

            // bishop & queen moves
            if(board[sq]==(side==white?B:b) || board[sq]==(side==white?Q:q)){
                // loop over bishop offsets
                for(int index=0; index<4; index++){
                    // init target square
                    int targetSq = sq+bishopOffsets[index];
                    // loop over attack ray
                    while(!(targetSq & 0x88)){
                        // init target piece
                        int piece = board[targetSq];
                        // if hits own piece
                        if(piece>=(side==white?P:p) && piece<=(side==white?K:k)) break;
                        // if hits enemy piece
                        if(piece>=(side==white?p:P) && piece<=(side==white?k:K)){
                            addMove(moveList,encodeMove(sq, targetSq, 0, 1, 0, 0, 0));
                            break;
                        }
                        if(!onlyCaptures)
                            // if empty
                            addMove(moveList,encodeMove(sq, targetSq, 0, 0, 0, 0, 0));
                        targetSq+=bishopOffsets[index];
                    }
                }
            }

            // knight moves
            if(board[sq]==(side==white?N:n)){
                // loop over offsets
                for(int index=0; index<8; index++){
                    // init target square and target piece
                    int targetSq = sq+knightOffsets[index];
                    int piece = board[targetSq];
                    // if target on board
                    if(!(targetSq&0x88)){
                        // if not ally
                        if(piece==e || (piece>=(side==white?p:P) && piece<=(side==white?k:K))){
                            if(!onlyCaptures)
                                // on empty
                                if(piece==e) addMove(moveList,encodeMove(sq, targetSq, 0, 0, 0, 0, 0));
                            // on capture
                            if(piece!=e) addMove(moveList,encodeMove(sq, targetSq, 0, 1, 0, 0, 0));
                        }
                    }
                }
            }

            // white pawn and castling moves
            if(side==white){
                // pawn moves
                if(board[sq]==P){
                    // quiet moves
                    if(!onlyCaptures){
                        // init target square
                        int targetSq = sq - 16;
                        // check if target square is on board and empty
                        if(!(targetSq & 0x88) && board[targetSq]==e){
                            // pawn promotion
                            if(sq>=a7 && sq<=h7){
                                addMove(moveList,encodeMove(sq, targetSq, Q, 0, 0, 0, 0));
                                addMove(moveList,encodeMove(sq, targetSq, R, 0, 0, 0, 0));
                                addMove(moveList,encodeMove(sq, targetSq, B, 0, 0, 0, 0));
                                addMove(moveList,encodeMove(sq, targetSq, N, 0, 0, 0, 0));
                            } else {
                                // single pawn move
                                addMove(moveList,encodeMove(sq, targetSq, 0, 0, 0, 0, 0));

                                // double pawn move
                                if((sq>=a2 && sq<=h2) && board[targetSq-16]==e){
                                    addMove(moveList,encodeMove(sq, targetSq-16, 0, 0, 1, 0, 0));
                                }
                            }
                        }
                    }

                    // captures
                    for(int index=2; index<=3; index++){
                        // pawn offset and target square
                        int pawnOffset = bishopOffsets[index];
                        int targetSq = sq + pawnOffset;
                        // if square on board and capturable
                        if(!(targetSq&0x88) && board[targetSq]>=p && board[targetSq]<=k){
                            // capture pawn promotion
                            if(sq>=a7 && sq<=h7){
                            addMove(moveList,encodeMove(sq, targetSq, Q, 1, 0, 0, 0));
                            addMove(moveList,encodeMove(sq, targetSq, R, 1, 0, 0, 0));
                            addMove(moveList,encodeMove(sq, targetSq, B, 1, 0, 0, 0));
                            addMove(moveList,encodeMove(sq, targetSq, N, 1, 0, 0, 0));
                            } else {
                                // casual capture
                                addMove(moveList,encodeMove(sq, targetSq, 0, 1, 0, 0, 0));
                            }
                        }
                        // enpassant capture
                        if(targetSq==enpassant){
                            addMove(moveList,encodeMove(sq, targetSq, 0, 1, 0, 1, 0));
                        }
                    }
                }
                if(!onlyCaptures){
                    // white king castling
                    if(board[sq]==K){
                        // if kingside available
                        if(castle&Kc){
                            // make sure empty squares between king and rook, king not under attack while castling
                            if(board[f1]==e && board[g1]==e && !isSquareAttacked(e1, black) && !isSquareAttacked(f1, black)){
                                addMove(moveList,encodeMove(e1, g1, 0, 0, 0, 0, 1));
                            }
                        }
                        // if queenside available
                        if(castle&Qc){
                            // make sure empty squares between king and rook, king not under attack while castling
                            if(board[d1]==e && board[c1]==e && board[b1]==e && !isSquareAttacked(e1, black) && !isSquareAttacked(d1, black)){
                                addMove(moveList,encodeMove(e1, c1, 0, 0, 0, 0, 1));
                            }
                        }
                    }
                }
            }

            // black pawn and castling moves
            if(side==black){
                // pawn moves
                if(board[sq]==p){
                    // quiet moves
                    if(!onlyCaptures){
                        // init target square
                        int targetSq = sq + 16;
                        // check if target square is on board and empty
                        if(!(targetSq & 0x88) && board[targetSq]==e){
                            // pawn promotion
                            if(sq>=a2 && sq<=h2){
                                addMove(moveList,encodeMove(sq, targetSq, q, 0, 0, 0, 0));
                                addMove(moveList,encodeMove(sq, targetSq, r, 0, 0, 0, 0));
                                addMove(moveList,encodeMove(sq, targetSq, b, 0, 0, 0, 0));
                                addMove(moveList,encodeMove(sq, targetSq, n, 0, 0, 0, 0));
                            } else {
                                // single pawn move
                                addMove(moveList,encodeMove(sq, targetSq, 0, 0, 0, 0, 0));

                                // double pawn move
                                if((sq>=a7 && sq<=h7) && board[targetSq+16]==e){
                                    addMove(moveList,encodeMove(sq, targetSq+16, 0, 0, 1, 0, 0));
                                }
                            }
                        }
                    }

                    // captures
                    for(int index=0; index<=1; index++){
                        // pawn offset and target square
                        int pawnOffset = bishopOffsets[index];
                        int targetSq = sq + pawnOffset;
                        // if square on board and capturable
                        if(!(targetSq&0x88) && board[targetSq]>=P && board[targetSq]<=K){
                            // capture pawn promotion
                            if(sq>=a2 && sq<=h2){
                            addMove(moveList,encodeMove(sq, targetSq, q, 1, 0, 0, 0));
                            addMove(moveList,encodeMove(sq, targetSq, r, 1, 0, 0, 0));
                            addMove(moveList,encodeMove(sq, targetSq, b, 1, 0, 0, 0));
                            addMove(moveList,encodeMove(sq, targetSq, n, 1, 0, 0, 0));
                            } else {
                                // single pawn capture
                                addMove(moveList,encodeMove(sq, targetSq, 0, 1, 0, 0, 0));
                            }
                        }
                        // enpassant capture
                        if(targetSq==enpassant){
                            addMove(moveList,encodeMove(sq, targetSq, 0, 1, 0, 1, 0));
                        }
                    }
                }
                if(!onlyCaptures){
                    // black king castling
                    if(board[sq]==k){
                        // if kingside available
                        if(castle&kc){
                            // make sure empty squares between king and rook, king not under attack while castling
                            if(board[f8]==e && board[g8]==e && !isSquareAttacked(e8, white) && !isSquareAttacked(f8, white)){
                                addMove(moveList,encodeMove(e8, g8, 0, 0, 0, 0, 1));
                            }
                        }
                        // if queenside available
                        if(castle&qc){
                            // make sure empty squares between king and rook, king not under attack while castling
                            if(board[d8]==e && board[c8]==e && board[b8]==e && !isSquareAttacked(e8, white) && !isSquareAttacked(d8, white)){
                                addMove(moveList,encodeMove(e8, c8, 0, 0, 0, 0, 1));
                            }
                        }
                    }
                }
            }

            // king moves
            if(board[sq]==(side==white?K:k)){
                // loop over offsets
                for(int index=0; index<8; index++){
                    // init target square and target piece
                    int targetSq = sq+kingOffsets[index];
                    int piece = board[targetSq];
                    // if target on board
                    if(!(targetSq&0x88)){
                        // if not ally
                        if(piece==e || (piece>=(side==white?p:P) && piece<=(side==white?k:K))){
                            if(!onlyCaptures)
                                // on empty
                                if(piece==e) addMove(moveList,encodeMove(sq, targetSq, 0, 0, 0, 0, 0));
                            // on capture
                            if(piece!=e) addMove(moveList,encodeMove(sq, targetSq, 0, 1, 0, 0, 0));
                        }
                    }
                }
            }
        }
    }
}

/***************************************************\

                   PERFT MACROS

\***************************************************/
#define copyBoard()                                 \
        int boardCopy[128], kingSqCopy[2];          \
        int sideCopy, enpassantCopy, castleCopy;    \
        memcpy(boardCopy, board, 512);              \
        sideCopy = side;                            \
        enpassantCopy = enpassant;                  \
        castleCopy = castle;                        \
        memcpy(kingSqCopy, kingSq, 8);              \

#define takeBack()                                  \
        memcpy(board, boardCopy, 512);              \
        side = sideCopy;                            \
        enpassant = enpassantCopy;                  \
        castle = castleCopy;                        \
        memcpy(kingSq, kingSqCopy, 8);              \
        fenHistory.pop_back();                      \

// make move
static inline int makeMove(int move, int captureFlag){
    if(!move) return 0;

    // make quiet move
    if(captureFlag==allMoves){
        copyBoard();

        // parse move
        int source = getMoveSource(move);
        int target = getMoveTarget(move);
        int promotedPiece = getMovePromotion(move);
        int enpassantFlag = getMoveEnpassant(move);
        int doublePush = getMoveDoublePawn(move);
        int castling = getMoveCastling(move);

        // move piece
        board[target] = board[source];
        board[source] = e;

        // pawn promotion
        if(promotedPiece){
            board[target] = promotedPiece;
        }

        // enpassant move
        if(enpassantFlag){
            board[target+(side==white?16:-16)] = e;
        }

        // reset enpassant
        enpassant = noSq;

        // double push
        if(doublePush){
            enpassant = target+(side==white?16:-16);
        }

        // castling
        if(castling){
            // switch target square
            switch(target){
                case g1:
                    // white kingside
                    board[f1] = board[h1];
                    board[h1] = e;
                    break;
                case c1:
                    // white queenside
                    board[d1] = board[a1];
                    board[a1] = e;
                    break;
                case g8:
                    // black kingside
                    board[f8] = board[h8];
                    board[h8] = e;
                    break;
                case c8:
                    // black queenside
                    board[d8] = board[a8];
                    board[a8] = e;
                    break;
            }       
        }


        // update king square
        if(board[target]==K||board[target]==k){
            kingSq[side] = target;
        }

        // update castling rights
        castle &= castlingRights[source];
        castle &= castlingRights[target];

        // change sides
        side ^= 1;

        createFEN(fenHistory);

        // take move back if king under check after change
        if(isSquareAttacked(kingSq[side^1], side)){
            // restore board state
            takeBack();
            
            // illegal move
            return 0;
        }
        return 1;
    } 
    // if is capture
    else {
        if(getMoveCapture(move))
            // make capture move
            return makeMove(move, allMoves);
        else
            // not a capture
            return 0;
    }
}

// count nodes
long long nodes = 0;

// get time in milliseconds
int getTimeMs(){
    struct timeval t;
    mingw_gettimeofday(&t, NULL);
    return t.tv_sec *  1000 + t.tv_usec / 1000;
}

// perft driver
static inline void perftDriver(int depth){
    // escape sequence
    if(depth == 0){
        nodes++;
        return;
    } 
    // create move list
    moves moveList[1];

    // generate moves
    generateMoves(moveList, true);
    
    // loop over generated moves
    for(int moveCount=0; moveCount<moveList->count; moveCount++){
       
       // copy board state
        copyBoard();

        if(!makeMove(moveList->moves[moveCount], allMoves))
            // skip invalid moves
            continue;

        // recursive call for depth
        perftDriver(depth-1);

        // restore board state
        takeBack();

    }
}

// perft test
static inline void perftTest(int depth){
    printf("\nPerformance test\n\n");

    // init start time
    int startTime = getTimeMs();

    // create move list
    moves moveList[1];

    // generate moves
    generateMoves(moveList, true);
    
    // loop over generated moves
    for(int moveCount=0; moveCount<moveList->count; moveCount++){

        // copy board
        copyBoard();

        if(!makeMove(moveList->moves[moveCount], allMoves))
            // skip invalid moves
            continue;

        // cumulative nodes
        long long cumNodes = nodes;

        // recursive call for depth
        perftDriver(depth-1);

        // old nodes
        long long oldNodes = nodes-cumNodes;

        // restore board state
        takeBack();

        // print current move
        printf("move %d: %s%s%c     %ld\n", moveCount+1,
                                    squareToCoords[getMoveSource(moveList->moves[moveCount])], 
                                    squareToCoords[getMoveTarget(moveList->moves[moveCount])],
                                    promotedPieces[getMovePromotion(moveList->moves[moveCount])],
                                    oldNodes);
    }

    // print results
    printf("\nDepth: %d", depth);
    printf("\nNodes: %ld", nodes);
    printf("\nTime: %dms\n\n", getTimeMs()-startTime);
}

// evaluation of position
static inline int evaluatePosition(){
    int score = 0;
    // loop over board squares
    for(int sq=0; sq<128; sq++){
        if(!(sq&0x88)){
            // get piece
            int piece = board[sq];
            // add its score
            score += materialScore[piece];

            // position scores
            switch(piece){
                case P:
                    score += pawnScore[sq];
                    break;
                case p:
                    score -= pawnScore[mirrorScore[sq]];
                    break;
                case N:
                    score += knightScore[sq];
                    break;
                case n:
                    score -= knightScore[mirrorScore[sq]];
                    break;
                case B:
                    score += bishopScore[sq];
                    break;
                case b:
                    score -= bishopScore[mirrorScore[sq]];
                    break;
                case R:
                    score += rookScore[sq];
                    break;
                case r:
                    score -= rookScore[mirrorScore[sq]];
                    break;
                case K:
                    score += kingScore[sq];
                    break;
                case k:
                    score -= kingScore[mirrorScore[sq]];                    
            }

        }
    }
    // return position score
    return (side==white? score:-score);
}

/****************************************\
 
                  Search

\****************************************/

// best move
int bestMove = 0;

// half move counter
int ply = 0;

// quiescence nodes
long long qnodes = 0;

// quiescence search
int quiescence(int alpha, int beta){
    // increment nodes
    nodes++;
    
    int eval = evaluatePosition();

    // fail hard beta cutoff (score too high, opponent won't allow)
    if(eval>=beta){
        return beta;
    }

    // increase alpha
    if (alpha < eval) alpha = eval;

    // create move list and fill it
    moves movelist[1];
    generateMoves(movelist, true);
    // printf("\tgenerated %d capturing moves at ply %d\n", movelist->count, ply);

    // loop over moves generated
    for(int count = 0; count<movelist->count; count++){
        // snapshot current board state
        copyBoard();
        
        // initialize move
        int move = movelist->moves[count];
        // printf("\t\t analyzing move %s%s\n", squareToCoords[getMoveSource(move)], squareToCoords[getMoveTarget(move)]);

        // increment half move
        ply++;

        // make only legal captures, otherwise...
        if(!makeMove(move, onlyCaptures)){
            // decrement half move
            ply--;
            
            continue;
        }

        // recursive negamax quiescence call
        int score = -quiescence(-beta, -alpha);
        
        // decrement half move
        ply--;

        // take move back
        takeBack();
        
        // fail hard beta cutoff (no way you're doing better than beta, stop hallucinating)
        if(score >= beta){
            return beta;
        }

        // found a better worst outcome (increase alpha)
        if(score > alpha){
            alpha = score;
        }
    }

    qnodes++;

    // return best score
    return alpha;
}

#define MAXPLY 9 // should be fixed around depth
// killer moves
int killerMoves[MAXPLY][2];

// principal variation
string pvline[MAXPLY+1];

// negamax search function (fail-hard alpha beta version)
int negamax(int alpha, int beta, int depth){
    // increase nodes
    nodes++;
    
    // escape condition
    if(!depth){
        // return position score
        return quiescence(alpha, beta);
    }

    // 3-fold repetition prevention
    // @attention this really needs to be checked
    int repeats = 0;
    for(int i=fenHistory.size()-3; i>=0; i-=2){
        if(fenHistory[i]==fenHistory.back()) repeats++;
        if(repeats==2){
            return 0;
        }
    } 

    // legal moves
    int legalMoves = 0;

    // initialize old alpha
    int oldAlpha = alpha;

    // initialize best move so far
    int bestMoveSoFar = 0;

    // create move list and fill it
    moves movelist[1];
    // generate captures
    generateMoves(movelist, true);
    // generate normal moves
    moves legalMoveList[1];
    generateMoves(legalMoveList, false);
    // add killer moves
    for(int i=0; i<size(killerMoves[ply]); i++){
        if(killerMoves[ply][i]==0) continue;
        for(int j=0; j<legalMoveList->count; j++){
            if(legalMoveList->moves[i]==killerMoves[ply][i]){
                addMove(movelist, killerMoves[ply][i]);
                break;
            }
        }
    }
    // add normal moves
    for(int i=0; i<legalMoveList->count; i++){
        addMove(movelist, legalMoveList->moves[i]);
    }

    // loop over moves generated
    for(int count = 0; count<movelist->count; count++){
        // snapshot current board state
        copyBoard();
        
        // initialize move
        int move = movelist->moves[count];

        // increment half move
        ply++;
        
        // make only legal moves
        if(!makeMove(move, allMoves)){ // if illegal
            // decrement half move
            ply--;
            continue;
        }

        if(ply==1)
        printf("evaluating move %s%s, which is %d out of %d\n", squareToCoords[getMoveSource(move)],
                                                                squareToCoords[getMoveTarget(move)],
                                                                count+1, movelist->count);
                                        

        // increment legal moves
        legalMoves++;
        
        // recursive negamax call
        int score = -negamax(-beta, -alpha, depth-1);
        
        // decrement half move
        ply--;

        // take move back
        takeBack();

        // fail hard beta cutoff (move was too good for the other player to allow)
        if(score>=beta){
            // loop over killer moves to move old ones back
            for(int i=size(killerMoves[ply])-2; i>=0; i--){
                // early return if repeating
                if(move==killerMoves[ply][i+1]) return beta;
                killerMoves[ply][i+1] = killerMoves[ply][i];
            }
            killerMoves[ply][0] = move;
            return beta;
        }

        // found a better move
        if(score>alpha){
            // increase lower bound
            alpha = score;

            pvline[ply] = (string) squareToCoords[getMoveSource(move)] 
                                  + squareToCoords[getMoveTarget(move)] + " " +
                                    pvline[ply+1];

            if(!ply) // this is a root node
                // associate best move so far with best score so far
                bestMoveSoFar = move;
        }

    }

    // associate best move with best score
    if(oldAlpha != alpha){
        bestMove = bestMoveSoFar;
    }

    // if no legal moves... winning or draw?
    if(!legalMoves){
        // checkmate
        if(isSquareAttacked(kingSq[side], side ^ 1)){
            pvline[ply] = "";
            return -100000 + ply;
        }
        // stalemate
         else 
            return 0;
    }

    // return best score
    return alpha;
}

// run search from UCI
int search(int depth){
    nodes = 0;
    qnodes = 0;
    memset(killerMoves, 0, sizeof(killerMoves));
    fill(pvline, pvline+size(pvline), "");

    // run negamax alpha beta search on a given depth
    int score = negamax(-100000, 100000, depth);
    printf("quiescence evaluated %ld nodes\n", qnodes);
    return score;
}

/****************************************\
 
                    UCI
           User Controlled Input

\****************************************/

/** parse move (from UCI)
 *  @return 0 if illegal
 */
int parseMove(char* moveStr){
    // init move list
    moves moveList[1];

    // generate moves
    generateMoves(moveList, false);

    // parse move string
    int parseFrom = (moveStr[0]-'a') + (8-(moveStr[1] - '0')) * 16;
    int parseTo = (moveStr[2]-'a') + (8-(moveStr[3] - '0')) * 16;
    int promPiece = 0;

    // init move to encode
    int move;

    // loop over generated moves
    for(int count = 0; count<moveList->count; count++){
        // pick up move
        move = moveList->moves[count];

        // if input present on move list
        if(getMoveSource(move)==parseFrom && getMoveTarget(move)==parseTo){
            // init promoted piece
            promPiece = getMovePromotion(move);

            // if promoted piece is present, compare it with user input
            if(promPiece!=e){
                if((promPiece==N||promPiece==n)&&moveStr[4]=='n') return move;
                else if((promPiece==B||promPiece==b)&&moveStr[4]=='b') return move;
                else if((promPiece==R||promPiece==r)&&moveStr[4]=='r') return move;
                else if((promPiece==Q||promPiece==q)&&moveStr[4]=='q') return move;
                continue;
            }
            return move;
        }
    }
    // illegal move
    return 0;
}

// function to evaluate for uci
void goDepth(int depth){
    int time = getTimeMs();
    // search position with current depth
    int score = search(depth);

    // output best move
    if(abs(score)<99900){
        printf("info score cp %d ", score);
    } else {
        int mult = score<0? -1:1;
        int mate = (100000-abs(score))/2;
        printf("info score mate %d ", mate*mult);
    }
    printf("depth %d nodes %ld time %dms ", depth, nodes, getTimeMs()-time);
    printf("pv %s \n", pvline[0].c_str());

    printf("bestmove %s%s%c\n", squareToCoords[getMoveSource(bestMove)],
                                squareToCoords[getMoveTarget(bestMove)],
                                promotedPieces[getMovePromotion(bestMove)]);    
}

// input buffer size
#define inputBuffer (400*6)
// UCI driver
void uci(){
    // init input buffer
    char line[inputBuffer];

    // print engine
    printf("id name 0x88 mailbox chess\n");
    printf("id author me :)\n");
    printf("uciok\n");

    // main loop
    while(true){
        // reset input
        memset(line, 0, sizeof(line));

        // flush output
        fflush(stdout);

        // skip if no user input or empty input
        if(!fgets(line, inputBuffer, stdin)) continue;
        if(line[0]=='\n') continue;

        // parse "uci" command
        if(!strncmp(line, "uci", 3)){
            // print engine info again
            printf("id name 0x88 mailbox chess\n");
            printf("id author Lucas :)\n");
            printf("uciok\n");
        }
        // parse "isready" commmand
        else if(!strncmp(line, "isready", 7)){
            // return engine is ready
            printf("readyok\n");
            continue;
        }
        // parse "ucinewgame" command
        else if(!strncmp(line, "ucinewgame", 10)){
            parseFEN(startingPosition);
        }
        // parse GUI input for all other positions
        else if(!strncmp(line, "position startpos moves", 23)){
            // init board with initial position
            parseFEN(startingPosition);

            // init move string
            char *moves = line;

            // increment until moves start
            moves += 23;

            // init character counter
            // int countChar = -1;

            // loop over move strings
            while(*moves){
                // pick next move in GUI input
                if(*moves==' '){
                    // go to next move
                    *moves++;

                    // parse move, make it on board
                    makeMove(parseMove(moves), allMoves);
                }

                // go to next move
                *moves++;
            }

            printBoard();
        }
        // parse initial position
        else if(!strncmp(line, "position startpos", 17)){
            // init starting position
            parseFEN(startingPosition);
            printBoard();
        }

        // init board from FEN
        else if(!strncmp(line, "position fen", 12)){
            // get FEN
            char* fen = line;
            fen += 13;

            // init board with current fen
            parseFEN(fen);

            // parse moves after fen
            char* moves = line;

            // find "moves" command
            while(strncmp(moves, "moves", 5)){
                *moves++;
                // break if no moves available
                if(*moves=='\0') break;
            }

            // go to moves
            moves += 4;

            // parse moves
            if(*moves == 's'){
                // init character counter
                int countChar = -1;

                // loop over move strings
                while(*moves){
                    // pick next move in GUI input
                    if(*moves==' '){
                        // go to next move
                        *moves++;

                        // parse move, make it on board
                        makeMove(parseMove(moves), allMoves);
                    }

                    // go to next move
                    *moves++;
                }
            }

            // print position
            printBoard();
        }

        // parse fixed depth "go" command
        else if (!strncmp(line, "go depth", 8)){
            // parse "go" command
            char* go = line;
            go += 9;

            // parse depth
            int depth = *go - '0';

            goDepth(depth);
        }

        // use fixed depth 5 for all the other modes but fixed depth, e.g. in blitz mode
        else if (!strncmp(line, "go", 2)){
            goDepth(5);
        }

        // search a specific move (not part of uci)
        else if (!strncmp(line, "consider", 8)){
            bool valid = true;
            char* consider = line;
            consider += 9;

            // get the depth (1 digit) @attention if this ever gets to more than 1 digit...
            int depth = consider[0]-'0';
            consider += 1;

            copyBoard();
            // loop over move strings
            while(*consider){
                // pick next move in GUI input
                if(*consider==' '){
                    // go to next move
                    *consider++;

                    // parse move, make it on board
                    if(!makeMove(parseMove(consider), allMoves)){
                        valid = false;
                    }
                }

                // go to next move
                *consider++;
            }
            if(valid){
                printBoard();
                goDepth(depth);
            }
            takeBack();

        }

        // parse "quit" command
        else if(!strncmp(line, "quit", 4)) break;
    }
}

// driver
int main()
{

    uci();
    for(string s: fenHistory){
        cout << s << '\n';
    }

    return 0;
}