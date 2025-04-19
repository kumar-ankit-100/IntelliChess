#include "common_header.h"

int is_knight_move_valid(Board &board, string &moves, int flag)
{
    int source_row = moves[1] - 48;
    int source_col = moves[0] - 97;
    int destination_col = moves[2] - 97;
    int destination_row = moves[3] - 48;

    if (abs(source_row - destination_row) == 1)
    {
        if (abs(source_col - destination_col) == 2)
        {
            return 1; // valid knight move
        }
    }
    if (abs(source_row - destination_row) == 2)
    {
        if (abs(source_col - destination_col) == 1)
        {
            return 1; // valid knight move
        }
    }
    return 0; // invalid knight move
}

vector<vector<string>> get_knight_moves(Board &board, char col, int row)
{
    vector<vector<string>> possible_moves;
    vector<string> moveList;
    vector<string> captureList;
    Bitboard bit = 1;
    Bitboard from_mask = bit << ((row * 8) - (col - 97) - 1);
    int flag = board.ouccupancy[white] & from_mask ? white : black;
    int opponentColor = flag == white ? black : white;

    // Define all possible knight moves (L-shaped)
    int knightMoves[8][2] = {
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};

    for (auto &move : knightMoves)
    {
        char new_col = col + move[1];
        int new_row = row + move[0];

        // Check if the new position is within the board boundaries
        if (new_col >= 'a' && new_col <= 'h' && new_row >= 1 && new_row <= 8)
        {
            Bitboard to_mask = bit << ((new_row * 8) - (new_col - 97) - 1);

            // Check if the square is occupied by an opponent's piece
            if (board.ouccupancy[opponentColor] & to_mask)
            {
                captureList.push_back(string(1, new_col) + to_string(new_row));
            }
            // Check if the square is empty
            else if (!(board.ouccupancy[both] & to_mask))
            {
                moveList.push_back(string(1, new_col) + to_string(new_row));
            }
        }
    }

    possible_moves.push_back(moveList);     // Normal moves
    possible_moves.push_back(captureList);  // Capture moves
    return possible_moves;
}
