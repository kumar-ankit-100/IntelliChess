#include "common_header.h"

int is_rook_move_valid(Board &board, string &moves, int flag)
{
    // check that rook moves in same direction or not
    if (!((moves[0] == moves[2]) || (moves[1] == moves[3])))
    {
        cout << "Please ! Enter valid rook move since rook move only in one direction " << endl;
        return 0;
    }
    // this is the case for capturing pieces
    //  if(is_piece_present_in_square(board,moves[2],moves[3])){
    //      return 5;
    //  }
    if (moves[0] == moves[2])
    {
        // if file is same then check is there any pieces in between source and destination
        int source = (moves[1] - 48);
        int destination = moves[3] - 48;
        // cout << source << " " << destination << endl;

        if (source <= destination)
        {
            source = source + 1;
            destination = destination - 1;
            cout << "rook is moving upward " << endl;

            while (source <= destination)
            {
                Bitboard bit = 1;
                Bitboard from_mask = bit << ((source * 8) - (moves[0] - 97) - 1);
                // checking that is piece is present in particular box or not

                if (board.ouccupancy[both] & from_mask)
                {
                    cout << "No pieces should be present between source and destination" << endl;
                    return 0;
                }
                source++;
            }
        }
        else
        {
            source = source - 1;
            destination = destination + 1;
            cout << "rook is moving downward " << endl;
            while (source >= destination)
            {
                Bitboard bit = 1;
                Bitboard from_mask = bit << ((source * 8) - (moves[0] - 97) - 1);
                // checking that is piece is present in particular box or not

                if (board.ouccupancy[both] & from_mask)
                {
                    cout << "No pieces should be present between source and destination" << endl;
                    return 0;
                }
                source--;
            }
        }
    }
    if (moves[1] == moves[3])
    {
        // if rank  is same then check is there any pieces in between source and destination
        int source = (moves[0] - 97);
        int destination = moves[2] - 97;
        // cout << source << " " << destination << endl;
        if (source <= destination)
        {
            source = source + 1;
            destination = destination - 1;
            cout << "rook is moving right " << endl;
            while (source <= destination)
            {
                Bitboard bit = 1;
                Bitboard from_mask = bit << (((moves[1] - 48) * 8) - source - 1);
                // checking that is piece is present in particular box or not
                if (board.ouccupancy[both] & from_mask)
                {
                    cout << "No pieces should be present between source and destination" << endl;
                    return 0;
                }
                source++;
            }
        }
        else
        {
            source -= 1;
            destination = destination + 1;
            cout << "rook is moving left" << endl;
            while (source >= destination)
            {

                Bitboard bit = 1;
                Bitboard from_mask = bit << (((moves[1] - 48) * 8) - source - 1);
                // checking that is piece is present in particular box or not
                if (board.ouccupancy[both] & from_mask)
                {
                    cout << "No pieces should be present between source and destination" << endl;
                    return 0;
                }
                source--;
            }
        }
    }
    return 1;
}

vector<vector<string>> get_rook_moves(Board &board, char col, int row)
{       
    vector<vector<string>> possible_moves;
    vector<string> straightMoves;
    Bitboard bit = 1;
    // cout << "bit is shift by " << ((row * 8) - (col - 97) - 1) << endl;
    Bitboard from_mask = bit << ((row * 8) - (col - 97) - 1);
    int flag = board.ouccupancy[white] & from_mask ? 1 : 0; // Check if it's a white rook

    // Check rook's horizontal and vertical moves (straight lines)
    // Horizontal (left-right)
    for (int i = col - 1; i >= 'a'; --i) // Move left
    {
        if (!is_piece_present_in_square(board, i, row))
        {
            straightMoves.push_back(string(1, i) + to_string(row));
        }
        else
        {
            Bitboard to_mask = bit << ((row * 8) - (i - 97) - 1);
            if (board.ouccupancy[flag ? black : white] & to_mask) // Check for opponent piece
            {
                straightMoves.push_back(string(1, i) + to_string(row)); // Capture opponent's piece
            }
            break; // Blocked by any piece
        }
    }

    for (int i = col + 1; i <= 'h'; ++i) // Move right
    {
        if (!is_piece_present_in_square(board, i, row))
        {
            straightMoves.push_back(string(1, i) + to_string(row));
        }
        else
        {
            Bitboard to_mask = bit << ((row * 8) - (i - 97) - 1);
            if (board.ouccupancy[flag ? black : white] & to_mask)
            {
                straightMoves.push_back(string(1, i) + to_string(row));
            }
            break;
        }
    }

    // Vertical (up-down)
    for (int j = row + 1; j <= 8; ++j) // Move upwards
    {
        if (!is_piece_present_in_square(board, col, j))
        {
            straightMoves.push_back(string(1, col) + to_string(j));
        }
        else
        {
            Bitboard to_mask = bit << ((j * 8) - (col - 97) - 1);
            if (board.ouccupancy[flag ? black : white] & to_mask)
            {
                straightMoves.push_back(string(1, col) + to_string(j));
            }
            break;
        }
    }

    for (int j = row - 1; j >= 1; --j) // Move downwards
    {
        if (!is_piece_present_in_square(board, col, j))
        {
            straightMoves.push_back(string(1, col) + to_string(j));
        }
        else
        {
            Bitboard to_mask = bit << ((j * 8) - (col - 97) - 1);
            if (board.ouccupancy[flag ? black : white] & to_mask)
            {
                straightMoves.push_back(string(1, col) + to_string(j));
            }
            break;
        }
    }

    // Add the straight moves to possible moves
    possible_moves.push_back(straightMoves);

    // Handle rook's capturing moves
    vector<string> captureMoves;
    for (const string &move : straightMoves)
    {
        int move_col = move[0];
        int move_row = stoi(move.substr(1));

        Bitboard to_mask = bit << ((move_row * 8) - (move_col - 97) - 1);
        if (board.ouccupancy[flag ? black : white] & to_mask) // If it's an opponent piece
        {
            captureMoves.push_back(move);
        }
    }

    possible_moves.push_back(captureMoves); // Capture moves

    return possible_moves;
}
