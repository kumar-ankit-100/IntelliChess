#include "common_header.h"

int is_bishop_move_valid(Board &board, string &moves, int flag)
{
    int source_row = moves[1] - 48;
    int source_col = moves[0] - 97;
    int destination_col = moves[2] - 97;
    int destination_row = moves[3] - 48;

    if (!(abs(source_col - destination_col) == abs(source_row - destination_row)))
    {
        cout << "piece can only move in diagonally" << endl;
        return 0;
    }
    // Direction 1: Moving diagonally right-up

    if (destination_col > source_col && destination_row > source_row)
    {
        source_col += 1;
        source_row += 1;
        destination_col -= 1;
        destination_row -= 1;
        while (source_col <= destination_col && source_row <= destination_row)
        {
            Bitboard bit = 1;
            Bitboard from_mask = bit << ((source_row * 8) - source_col - 1);
            if (board.ouccupancy[both] & from_mask)
            {
                cout << "No pieces should be present between source and destination" << endl;
                return 0;
            }
            source_col++;
            source_row++;
        }
    }
    // Direction 2: Moving diagonally left-up
    else if (destination_col < source_col && destination_row > source_row)
    {
        source_col -= 1;
        source_row += 1;
        while (source_col > destination_col && source_row < destination_row)
        {
            Bitboard bit = 1;
            Bitboard from_mask = bit << ((source_row * 8) - source_col - 1);
            if (board.ouccupancy[both] & from_mask)
            {
                cout << "No pieces should be present between source and destination" << endl;
                return 0;
            }
            source_col--;
            source_row++;
        }
    }
    // Direction 3: Moving diagonally right-down
    else if (destination_col > source_col && destination_row < source_row)
    {
        source_col += 1;
        source_row -= 1;
        while (source_col < destination_col && source_row > destination_row)
        {
            Bitboard bit = 1;
            Bitboard from_mask = bit << ((source_row * 8) - source_col - 1);
            if (board.ouccupancy[both] & from_mask)
            {
                cout << "No pieces should be present between source and destination" << endl;
                return 0;
            }
            source_col++;
            source_row--;
        }
    }
    // Direction 4: Moving diagonally left-down
    else if (destination_col < source_col && destination_row < source_row)
    {
        source_col -= 1;
        source_row -= 1;
        while (source_col > destination_col && source_row > destination_row)
        {
            Bitboard bit = 1;
            Bitboard from_mask = bit << ((source_row * 8) - source_col - 1);
            if (board.ouccupancy[both] & from_mask)
            {
                cout << "No pieces should be present between source and destination" << endl;
                return 0;
            }
            source_col--;
            source_row--;
        }
    }

    return 1;
}


vector<vector<string>> get_bishop_moves(Board &board, char col, int row) {
    vector<vector<string>> possible_moves;
    vector<string> emptyMove;
    vector<string> captureMove;
    Bitboard bit = 1;
    Bitboard from_mask = bit << ((row * 8) - (col - 97) - 1);
    int flag = board.ouccupancy[white] & from_mask ? white : black;  // Determine if the bishop is white or black
    int opponentColor = (flag == white) ? black : white;

    // Move diagonally in all four directions
    int directions[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}}; // top-right, top-left, bottom-right, bottom-left

    for (auto dir : directions) {
        char cur_col = col;
        int cur_row = row;

        while (true) {
            cur_col += dir[1];  // Move left or right
            cur_row += dir[0];  // Move up or down

            // Check if the current position is out of bounds
            if (cur_col < 'a' || cur_col > 'h' || cur_row < 1 || cur_row > 8)
                break;

            // Check if there's a piece in the current square
            if (is_piece_present_in_square(board, cur_col, cur_row)) {
                Bitboard to_mask = bit << ((cur_row * 8) - (cur_col - 97) - 1);

                // If it's an opponent's piece, mark it as a capture move
                if (board.ouccupancy[opponentColor] & to_mask) {
                    captureMove.push_back(string(1, cur_col) + to_string(cur_row));
                }

                // Stop moving in this direction after capturing or finding a piece
                break;
            } else {
                // If the square is empty, it's a valid move
                emptyMove.push_back(string(1, cur_col) + to_string(cur_row));
            }
        }
    }

    // Add possible moves and capture moves to the result
    possible_moves.push_back(emptyMove);
    possible_moves.push_back(captureMove);

    return possible_moves;
}
