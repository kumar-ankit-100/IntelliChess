#include "common_header.h"

int is_queen_move_valid(Board &board, string &moves, int flag)
{
    // Queen moves like both a bishop (diagonal) and a rook (straight).
    if (is_bishop_move_valid(board, moves, flag) || is_rook_move_valid(board, moves, flag))
    {
        return 1; // Move is valid either as a bishop or a rook move.
    }

    // If neither of bishop or rook moves are valid, return invalid.
    return 0;
}

vector<vector<string>> get_queen_moves(Board &board, char col, int row)
{
    vector<vector<string>> possible_moves;
    vector<string> emptyMove;
    vector<string> captureMove;
    Bitboard bit = 1;
    Bitboard from_mask = bit << ((row * 8) - (col - 97) - 1);
    int flag = board.ouccupancy[white] & from_mask ? 1 : 0;
    int opponentColor = flag ? black : white;

    // Define all 8 possible directions for the queen: rook + bishop combined
    int directions[8][2] = {
        {1, 0},  // Up (rook)
        {-1, 0}, // Down (rook)
        {0, 1},  // Right (rook)
        {0, -1}, // Left (rook)
        {1, 1},  // Up-right (bishop)
        {1, -1}, // Up-left (bishop)
        {-1, 1}, // Down-right (bishop)
        {-1, -1} // Down-left (bishop)
    };

    // Loop through each direction
    for (int i = 0; i < 8; ++i)
    {
        int dirX = directions[i][0];
        int dirY = directions[i][1];
        char curr_col = col + dirY;
        int curr_row = row + dirX;

        // Keep moving in the current direction until you hit another piece or go out of bounds
        while (curr_col >= 'a' && curr_col <= 'h' && curr_row >= 1 && curr_row <= 8)
        {
            Bitboard to_mask = bit << ((curr_row * 8) - (curr_col - 97) - 1);

            // If there's a piece on the current square
            if (is_piece_present_in_square(board, curr_col, curr_row))
            {
                // If it's an opponent's piece, it's a capture move
                if (board.ouccupancy[opponentColor] & to_mask)
                {
                    captureMove.push_back(string(1, curr_col) + to_string(curr_row));
                }
                // Stop exploring further in this direction (since a piece blocks the path)
                break;
            }
            else
            {
                // No piece on this square, it's a valid move
                emptyMove.push_back(string(1, curr_col) + to_string(curr_row));
            }

            // Move further in this direction
            curr_col += dirY;
            curr_row += dirX;
        }
    }

    possible_moves.push_back(emptyMove);
    possible_moves.push_back(captureMove);
    return possible_moves;
}
