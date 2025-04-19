#include <algorithm>
#include <climits>
#include <vector>
#include <string>
#include <cmath>
#include "common_header.h"

// Piece values for evaluation
const int PAWN_VALUE = 100;
const int KNIGHT_VALUE = 320;
const int BISHOP_VALUE = 330;
const int ROOK_VALUE = 500;
const int QUEEN_VALUE = 900;
const int KING_VALUE = 20000;

const int PAWN_PST[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5, 5, 10, 25, 25, 10, 5, 5,
    0, 0, 0, 20, 20, 0, 0, 0,
    5, -5, -10, 0, 0, -10, -5, 5,
    5, 10, 10, -20, -20, 10, 10, 5,
    0, 0, 0, 0, 0, 0, 0, 0};

const int KNIGHT_PST[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20, 0, 0, 0, 0, -20, -40,
    -30, 0, 10, 15, 15, 10, 0, -30,
    -30, 5, 15, 20, 20, 15, 5, -30,
    -30, 0, 15, 20, 20, 15, 0, -30,
    -30, 5, 10, 15, 15, 10, 5, -30,
    -40, -20, 0, 5, 5, 0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50};

const int BISHOP_PST[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 5, 10, 10, 5, 0, -10,
    -10, 5, 5, 10, 10, 5, 5, -10,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -10, 10, 10, 10, 10, 10, 10, -10,
    -10, 5, 0, 0, 0, 0, 5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20};

const int ROOK_PST[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    5, 10, 10, 10, 10, 10, 10, 5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    0, 0, 0, 5, 5, 0, 0, 0};

const int QUEEN_PST[64] = {
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 5, 5, 5, 5, 0, -10,
    -5, 0, 5, 5, 5, 5, 0, -5,
    0, 0, 5, 5, 5, 5, 0, -5,
    -10, 5, 5, 5, 5, 5, 0, -10,
    -10, 0, 5, 0, 0, 0, 0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20};

const int KING_PST[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    20, 20, 0, 0, 0, 0, 20, 20,
    20, 30, 10, 0, 0, 10, 30, 20};

// Function to evaluate the board position
int evaluateBoard(const Board &board)
{
    int whiteScore = 0;
    int blackScore = 0;

    // Count material and apply piece-square tables
    for (int i = 0; i < 64; i++)
    {
        Bitboard mask = 1ULL << i;
        if (board.pices[white][pawn] & mask)
            whiteScore += PAWN_VALUE + PAWN_PST[i];
        if (board.pices[white][knight] & mask)
            whiteScore += KNIGHT_VALUE + KNIGHT_PST[i];
        if (board.pices[white][bishop] & mask)
            whiteScore += BISHOP_VALUE + BISHOP_PST[i];
        if (board.pices[white][rook] & mask)
            whiteScore += ROOK_VALUE + ROOK_PST[i];
        if (board.pices[white][queen] & mask)
            whiteScore += QUEEN_VALUE + QUEEN_PST[i];
        if (board.pices[white][king] & mask)
            whiteScore += KING_VALUE + KING_PST[i];

        if (board.pices[black][pawn] & mask)
            blackScore += PAWN_VALUE + PAWN_PST[63 - i];
        if (board.pices[black][knight] & mask)
            blackScore += KNIGHT_VALUE + KNIGHT_PST[63 - i];
        if (board.pices[black][bishop] & mask)
            blackScore += BISHOP_VALUE + BISHOP_PST[63 - i];
        if (board.pices[black][rook] & mask)
            blackScore += ROOK_VALUE + ROOK_PST[63 - i];
        if (board.pices[black][queen] & mask)
            blackScore += QUEEN_VALUE + QUEEN_PST[63 - i];
        if (board.pices[black][king] & mask)
            blackScore += KING_VALUE + KING_PST[63 - i];
    }

    // Evaluate pawn structure, mobility, king safety, etc.
    // ... (implement these evaluations)

    return whiteScore - blackScore;
}

// Function to generate all legal moves
bool isLegalMove( Board &board, const string &move, int color)
{
    // Implement move legality check, including:
    // 1. Basic move rules for each piece
    // 2. Check if the move leaves the king in check
    // 3. Special rules (castling, en passant, etc.)
    if (moveGeneration(move, board))
        return true; // Placeholder
    else
        return false;
}
vector<std::string> generateAllLegalMoves( Board &board, int color)
{
    std::vector<std::string> allMoves;

    for (char col = 'a'; col <= 'h'; col++)
    {
        for (int row = 1; row <= 8; row++)
        {
            // Check each piece type
            std::vector<std::vector<std::string>> pieceMoves;

            if (board.pices[color][pawn] & (1ULL << ((row * 8) - (col - 97) - 1)))
                pieceMoves = get_pawn_moves(const_cast<Board &>(board), col, row);
            else if (board.pices[color][rook] & (1ULL << ((row * 8) - (col - 97) - 1)))
                pieceMoves = get_rook_moves(const_cast<Board &>(board), col, row);
            else if (board.pices[color][knight] & (1ULL << ((row * 8) - (col - 97) - 1)))
                pieceMoves = get_knight_moves(const_cast<Board &>(board), col, row);
            else if (board.pices[color][bishop] & (1ULL << ((row * 8) - (col - 97) - 1)))
                pieceMoves = get_bishop_moves(const_cast<Board &>(board), col, row);
            else if (board.pices[color][queen] & (1ULL << ((row * 8) - (col - 97) - 1)))
                pieceMoves = get_queen_moves(const_cast<Board &>(board), col, row);
            else if (board.pices[color][king] & (1ULL << ((row * 8) - (col - 97) - 1)))
                pieceMoves = get_king_moves(const_cast<Board &>(board), col, row);

            for (const auto &moveList : pieceMoves)
            {
                for (const auto &move : moveList)
                {
                    std::string fullMove = std::string(1, col) + std::to_string(row) + move;
                    // if (isLegalMove(board, fullMove, color))
                    // {
                        allMoves.push_back(fullMove);
                    // }
                }
            }
        }
    }

    return allMoves;
}

// Structure to store move information
struct MoveInfo
{
    std::string move;
    int movingColor;
    int capturedPiece; // -1 if no capture
    bool wasFirstMove; // For pawns and kings (castling rights)
};

// Function to make a move on the board
MoveInfo makeMove(Board &board, const std::string &move)
{
    char fromCol = move[0], toCol = move[2];
    int fromRow = move[1] - '0', toRow = move[3] - '0';

    Bitboard fromMask = 1ULL << ((fromRow * 8) - (fromCol - 'a') - 1);
    Bitboard toMask = 1ULL << ((toRow * 8) - (toCol - 'a') - 1);

    int movingColor = (board.ouccupancy[white] & fromMask) ? white : black;
    int capturedColor = 1 - movingColor;

    MoveInfo info;
    info.move = move;
    info.movingColor = movingColor;
    info.capturedPiece = -1;
    info.wasFirstMove = false;

    // Check for capture and store captured piece
    for (int pieceType = 0; pieceType < 6; pieceType++)
    {
        if (board.pices[capturedColor][pieceType] & toMask)
        {
            info.capturedPiece = pieceType;
            board.pices[capturedColor][pieceType] &= ~toMask;
            break;
        }
    }

    // Move the piece
    for (int pieceType = 0; pieceType < 6; pieceType++)
    {
        if (board.pices[movingColor][pieceType] & fromMask)
        {
            board.pices[movingColor][pieceType] &= ~fromMask;
            board.pices[movingColor][pieceType] |= toMask;

            // Check if it's a pawn or king's first move
            if ((pieceType == pawn && (fromRow == 2 || fromRow == 7)) ||
                (pieceType == king && fromCol == 'e' && (fromRow == 1 || fromRow == 8)))
            {
                info.wasFirstMove = true;
            }

            break;
        }
    }

    // Update occupancy bitboards
    board.ouccupancy[movingColor] = (board.ouccupancy[movingColor] & ~fromMask) | toMask;
    board.ouccupancy[capturedColor] &= ~toMask;
    board.ouccupancy[both] = board.ouccupancy[white] | board.ouccupancy[black];

    return info;
}

// Function to undo a move on the board
void undoMove(Board &board, const MoveInfo &info)
{
    char fromCol = info.move[0], toCol = info.move[2];
    int fromRow = info.move[1] - '0', toRow = info.move[3] - '0';

    Bitboard fromMask = 1ULL << ((fromRow * 8) - (fromCol - 'a') - 1);
    Bitboard toMask = 1ULL << ((toRow * 8) - (toCol - 'a') - 1);
    int movingColor = info.movingColor;
    int capturedColor = 1 - movingColor;

    // Move the piece back
    for (int pieceType = 0; pieceType < 6; pieceType++)
    {
        if (board.pices[movingColor][pieceType] & toMask)
        {
            board.pices[movingColor][pieceType] &= ~toMask;
            board.pices[movingColor][pieceType] |= fromMask;
            break;
        }
    }

    // Restore captured piece if any
    if (info.capturedPiece != -1)
    {
        board.pices[capturedColor][info.capturedPiece] |= toMask;
    }

    // Update occupancy bitboards
    board.ouccupancy[movingColor] = (board.ouccupancy[movingColor] & ~toMask) | fromMask;
    if (info.capturedPiece != -1)
    {
        board.ouccupancy[capturedColor] |= toMask;
    }
    board.ouccupancy[both] = board.ouccupancy[white] | board.ouccupancy[black];
}

// Updated minimax function
int minimax(Board &board, int depth, int alpha, int beta, bool maximizingPlayer)
{
    if (depth == 0)
    {
        return evaluateBoard(board);
    }

    std::vector<std::string> moves = generateAllLegalMoves(board, maximizingPlayer ? white : black);

    if (maximizingPlayer)
    {
        int maxEval = INT_MIN;
        for (const auto &move : moves)
        {
            MoveInfo info = makeMove(board, move);
            int eval = minimax(board, depth - 1, alpha, beta, false);
            undoMove(board, info);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha)
                break;
        }
        return maxEval;
    }
    else
    {
        int minEval = INT_MAX;
        for (const auto &move : moves)
        {
            MoveInfo info = makeMove(board, move);
            int eval = minimax(board, depth - 1, alpha, beta, true);
            undoMove(board, info);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha)
                break;
        }
        return minEval;
    }
}

// Updated getBestMove function
std::string getBestMove(Board &board, int color, int depth)
{
    std::vector<std::string> moves = generateAllLegalMoves(board, color);
    std::string bestMove;
    int bestValue = (color == white) ? INT_MIN : INT_MAX;

    for (const auto &move : moves)
    {
        cout << move << " ";
        MoveInfo info = makeMove(board, move);
        int value = minimax(board, depth - 1, INT_MIN, INT_MAX, color != white);
        undoMove(board, info);

        if ((color == white && value > bestValue) || (color == black && value < bestValue))
        {
            bestValue = value;
            bestMove = move;
        }
    }
    cout << "endl";

    return bestMove;
}