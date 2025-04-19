#include "common_header.h"

bool is_king_in_check(Board &board, int kingColor)
{
    char kingCol;
    int kingRow;
    // Find the king's position
    Bitboard kingPosition = board.pices[kingColor][king];
    for (int i = 0; i < 64; ++i)
    {
        if (kingPosition & (1ULL << i))
        {

            kingCol = 'h' - (i % 8);
            kingRow = 1 + (i / 8);
            cout << "king row and col and i" << kingRow << " " << kingCol << " " << i << endl;
            break;
        }
    }
    int opponentColor = (kingColor == white) ? black : white;
    // if (is_square_under_attack(board, kingCol, kingRow, opponentColor))
    // {
    //     cout << "Allowed Empty move: " << endl;
    //     for (auto moves : board.emptySquares)
    //         cout << moves << " ";
    //     cout << endl;
    //     cout << "Size of emptySquares: " << board.emptySquares.size() << endl;
    //     cout<<"name : "<<board.name<<endl;
    // }
    return is_square_under_attack(board, kingCol, kingRow, opponentColor);
}
int findPieceName(Board &board, const string &move, int color)
{
    Bitboard bit = 1;
    Bitboard from_mask = bit << ((move[1] - '0') * 8 - (move[0] - 97) - 1);
    if (board.pices[color][pawn] & from_mask)
    {
        return 0;
    }
    else if (board.pices[color][rook] & from_mask)
    {
        return 1;
    }
    else if (board.pices[color][knight] & from_mask)
    {
        return 2;
    }
    else if (board.pices[color][bishop] & from_mask)
    {
        return 3;
    }
    else if (board.pices[color][queen] & from_mask)
    {
        return 5;
    }
    else if (board.pices[color][king] & from_mask)
    {
        return 4;
    }
    return -1;
}

bool is_checkmate(Board &board, int currentPlayer)
{
    // Get the king's position
    string kingPos = kingPosition(board, currentPlayer);
    char kingCol = kingPos[0];
    int kingRow = kingPos[1] - '0';
    int opponentPlayer = currentPlayer == white ? black : white;

    // Check if the king is in check
    if (!is_king_in_check(board, currentPlayer))
    {
        return false; // Not in check, so not checkmate
    }
    int pieceName = 10;

    // Try all possible moves for the current player
    for (char col = 'a'; col <= 'h'; ++col)
    {
        for (int row = 1; row <= 8; ++row)
        {
            Bitboard bit = 1;
            Bitboard from_mask = bit << ((row * 8) - (col - 97) - 1);
            if (board.ouccupancy[currentPlayer] & from_mask)
            {
                // Get the piece type at this position
                cout << "row and column : " << row << " " << col << endl;
                vector<vector<string>> pieceMoves;
                if (board.pices[currentPlayer][pawn] & from_mask)
                {
                    pieceMoves = get_pawn_moves(board, col, row);
                    pieceName = 0;
                    cout << pieceName << endl;
                }
                else if (board.pices[currentPlayer][rook] & from_mask)
                {
                    pieceMoves = get_rook_moves(board, col, row);
                    pieceName = 1;
                    cout << pieceName << endl;
                }
                else if (board.pices[currentPlayer][knight] & from_mask)
                {
                    pieceMoves = get_knight_moves(board, col, row);
                    pieceName = 2;
                    cout << pieceName << endl;
                }
                else if (board.pices[currentPlayer][bishop] & from_mask)
                {
                    pieceMoves = get_bishop_moves(board, col, row);
                    pieceName = 3;
                    cout << pieceName << endl;
                }
                else if (board.pices[currentPlayer][queen] & from_mask)
                {
                    pieceMoves = get_queen_moves(board, col, row);
                    pieceName = 5;
                    cout << pieceName << endl;
                }
                else if (board.pices[currentPlayer][king] & from_mask)
                {
                    pieceMoves = get_king_moves(board, col, row);
                    pieceName = 4;
                    cout << pieceName << endl;
                }

                // Check if any move can remove the check
                cout << "simulating the move of << : " << pieceName << endl;
                int i = 0;
                int opponentPieceName;
                for (const auto &moveList : pieceMoves)
                {
                    for (const auto &move : moveList)
                    {
                        cout << move << " ";
                        // Simulate the move
                        Bitboard to_mask = bit << ((move[1] - '0') * 8 - (move[0] - 97) - 1);
                        // if (board.ouccupancy[opponentPlayer] & to_mask)
                        // {
                        // }
                        if (i == 1)
                        {

                            opponentPieceName = findPieceName(board, move, opponentPlayer);
                            cout<<"opponent piece name : "<<opponentPieceName<<endl;
                            board.ouccupancy[opponentPlayer] &= ~to_mask;
                            board.pices[opponentPlayer][opponentPieceName] &= ~to_mask;
                            board.ouccupancy[both] &= ~to_mask;
                        }
                        board.ouccupancy[currentPlayer] &= ~from_mask;
                        board.ouccupancy[currentPlayer] |= to_mask;
                        board.pices[currentPlayer][pieceName] &= ~from_mask;
                        board.pices[currentPlayer][pieceName] |= to_mask;
                        board.ouccupancy[both] &= ~from_mask;
                        board.ouccupancy[both] |= to_mask;

                        bool stillInCheck = is_king_in_check(board, currentPlayer);

                        // Undo the move
                        board.ouccupancy[currentPlayer] &= ~to_mask;
                        board.ouccupancy[currentPlayer] |= from_mask;
                        board.pices[currentPlayer][pieceName] &= ~to_mask;
                        board.pices[currentPlayer][pieceName] |= from_mask;
                        board.ouccupancy[both] &= ~to_mask;
                        board.ouccupancy[both] |= from_mask;
                        if (i == 1)
                        {
                            // board.ouccupancy[opponentPlayer] &= ~;
                            board.ouccupancy[opponentPlayer] |= to_mask;
                            board.pices[opponentPlayer][opponentPieceName] |= to_mask;
                            board.ouccupancy[both] |= to_mask;
                        }
                        // printBoard(board);

                        if (!stillInCheck)
                        {
                            return false; // Found a move that removes the check
                        }
                    }
                    i++;
                }
            }
        }
    }

    return true; // No legal moves to remove the check, so it's checkmate
}
