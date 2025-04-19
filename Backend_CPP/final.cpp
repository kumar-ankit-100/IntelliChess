#include <iostream>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <thread>
#include <chrono>
#include <asio.hpp>
#include <atomic>
#include <boost/asio/strand.hpp>
#include <boost/bind/bind.hpp>
#include <memory>
#include <vector>
#include <algorithm> // for set_intersection and sort
#include <iterator>  // for back_inserter
#include <unordered_map>

// #include "common_header.h"
#include "gameManager.hpp"

using boost::asio::ip::tcp;
using namespace std;
using namespace boost;

#define BUFFER_SIZE 1024
#define PORT 8080

// Global atomic flag for socket validity
std::atomic<bool> socket_valid(true);
class WebSocketSession;
std::unordered_map<int, std::shared_ptr<WebSocketSession>> sessions_;

string find_knight_checking_king(Board &board, char king_col, int king_row, int kingColor)
{
    // Define all possible knight moves (L-shaped)
    int knightMoves[8][2] = {
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1}, // Vertical L-shapes
        {1, 2},
        {1, -2},
        {-1, 2},
        {-1, -2} // Horizontal L-shapes
    };

    int opponentColor = (kingColor == white) ? black : white;

    for (auto &move : knightMoves)
    {
        char new_col = king_col + move[1];
        int new_row = king_row + move[0];

        // Check if the new position is within the board boundaries
        if (new_col >= 'a' && new_col <= 'h' && new_row >= 1 && new_row <= 8)
        {
            Bitboard to_mask = 1ULL << ((new_row * 8) - (new_col - 97) - 1);
            // cout<<"knight checked position : "<<new_col<<new_row<<endl;

            // Check if the square is occupied by an opponent's knight
            if (board.pices[opponentColor][knight] & to_mask)
            {
                return positionToString(new_col, new_row);
            }
        }
    }

    return ""; // Return an empty string if no knight is checking the king
}

string kingPosition(Board &board, int pieceColor)
{
    // Find the opponent king's position
    char kingCol;
    int kingRow;
    Bitboard kingPositionInBit = board.pices[pieceColor][king];
    for (int i = 0; i < 64; ++i)
    {
        if (kingPositionInBit & (1ULL << i))
        {

            kingCol = 'h' - (i % 8);
            kingRow = 1 + (i / 8);
            break;
        }
    }
    string kingPositionInString = positionToString(kingCol, kingRow);
    return kingPositionInString;
}

string base64_encode(unsigned char *input, int length)
{
    BIO *bmem = BIO_new(BIO_s_mem());
    BIO *b64 = BIO_new(BIO_f_base64());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, input, length);
    BIO_flush(b64);

    BUF_MEM *bptr;
    BIO_get_mem_ptr(b64, &bptr);

    string encoded(bptr->data, bptr->length - 1);
    BIO_free_all(b64);

    return encoded;
}

string sha1_hash(const string &input)
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char *>(input.c_str()), input.length(), hash);
    return string(reinterpret_cast<char *>(hash), SHA_DIGEST_LENGTH);
}

string websocket_handshake_response(const string &key)
{
    string magic_string = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    string hashed_key = sha1_hash(magic_string);
    return base64_encode(reinterpret_cast<unsigned char *>(&hashed_key[0]), hashed_key.size());
}

string decode_websocket_frame(const char *buffer, int length)
{
    unsigned char payload_length = buffer[1] & 0x7F;
    int mask_index = 2;
    if (payload_length == 126)
    {
        mask_index = 4;
    }
    else if (payload_length == 127)
    {
        mask_index = 10;
    }

    char masking_key[4];
    memcpy(masking_key, &buffer[mask_index], 4);

    string decoded_message;
    for (int i = mask_index + 4; i < length; ++i)
    {
        decoded_message += buffer[i] ^ masking_key[(i - (mask_index + 4)) % 4];
    }

    return decoded_message;
}

string encode_websocket_frame(const string &message)
{
    string frame;
    frame += 0x81; // First byte: FIN bit set, opcode for text frame (0x1)

    // Payload length encoding
    if (message.size() <= 125)
    {
        frame += static_cast<char>(message.size()); // No masking and length <= 125
    }
    else if (message.size() <= 65535)
    {
        frame += 126;
        frame += static_cast<char>((message.size() >> 8) & 0xFF);
        frame += static_cast<char>(message.size() & 0xFF);
    }
    else
    {
        frame += 127;
        for (int i = 7; i >= 0; --i)
        {
            frame += static_cast<char>((message.size() >> (8 * i)) & 0xFF);
        }
    }

    // Append the actual message
    frame += message;
    return frame;
}

std::atomic<int> pendingPlayer{-1};
// Global map to store player relationships
std::map<int, int> player_relationships;
std::mutex relationships_mutex;
std::mutex sockets_mutex;

std::unordered_map<int, std::shared_ptr<tcp::socket>> idToSocket;

// Mutex to protect access to the map (optional for thread safety)
std::mutex idToSocketMutex;
std::mutex read_mutex;
std::mutex isCheckmateMutex;
std::mutex isPlayWithAi;
int prev_socket_id=-1;

void notify_match_started(tcp::socket &socket)
{
    int socket_id = socket.native_handle();
    auto game_it = game_manager.findGameBySocket(socket_id);
    if (game_it == game_manager.games.end())
    {
        std::cerr << "Error: Game not found for socket " << socket_id << std::endl;
        return;
    }

    std::shared_ptr<Game> game = game_it->second;
    int opponent_socket_id = game->getOpponentSocket(socket_id);

    std::string color = (socket_id == game->white_socket) ? "white" : "black";
    std::string opponent_color = (color == "white") ? "black" : "white";

    std::cout << "Match started: " << color << " (socket " << socket_id
              << ") vs " << opponent_color << " (socket " << opponent_socket_id << ")" << std::endl;

    // JSON message for the player
    json message = {
        {"message", "Match started"},
        {"your_socket_id", socket_id},
        {"opponent_socket_id", opponent_socket_id},
        {"color", color}};

    // Convert JSON to WebSocket frame
    std::string encoded_message = encode_websocket_frame(message.dump());

    // Mutex lock to prevent race conditions while sending messages
    {
        std::lock_guard<std::mutex> lock(sockets_mutex);

        // Send the message to the player
        asio::async_write(socket, asio::buffer(encoded_message),
                          [socket_id, color](boost::system::error_code ec, std::size_t /*length*/)
                          {
                              if (!ec)
                              {
                                  std::cout << "Notified " << color << " player: " << socket_id << std::endl;
                              }
                              else
                              {
                                  std::cerr << "Error notifying " << color << " player: " << ec.message() << std::endl;
                              }
                          });
    }
}

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession>
{
public:
    WebSocketSession(tcp::socket socket, asio::io_context &context)
        : socket_(std::move(socket)),
          strand_(asio::make_strand(context)),
          timer_(context),
          matched_(false),
          opponent_socket_id_(-1) {}

    void start()
    {
        do_handshake();
    }
    tcp::socket socket_;
    asio::strand<asio::io_context::executor_type> strand_;

private:
    void do_handshake()
    {
        auto self = shared_from_this();
        asio::async_read_until(socket_, asio::dynamic_buffer(buffer_), "\r\n\r\n",
                               asio::bind_executor(strand_, [this, self](boost::system::error_code ec, std::size_t length)
                                                   {
                if (!ec) {
                    std::string request(buffer_.substr(0, length));
                    buffer_.erase(0, length);

                    // Extract WebSocket key and send handshake response
                    size_t start_key = request.find("Sec-WebSocket-Key: ") + 19;
                    size_t end_key = request.find("\r\n", start_key);
                    std::string key = request.substr(start_key, end_key - start_key);
                    std::string accept_key = websocket_handshake_response(key);
                    std::string response =
                        "HTTP/1.1 101 Switching Protocols\r\n"
                        "Upgrade: websocket\r\n"
                        "Connection: Upgrade\r\n"
                        "Sec-WebSocket-Accept: " + accept_key + "\r\n\r\n";

                    do_write(response);
                } }));
    }

    void do_write(const std::string &message)
    {
        auto self = shared_from_this();
        asio::async_write(socket_, asio::buffer(message),
                          asio::bind_executor(strand_, [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                              {
                if (!ec) {
                    cout << "handshake completed and socket id:" << socket_.native_handle() << endl;
                    sessions_[socket_.native_handle()]=shared_from_this();
                    do_read();
                    // if (!matched_) {
                    //     check_for_match();
                    // } else {
                    //     do_read();
                    //     // do_send_periodic_messages();
                    // }
                } }));
    }
    // void storeSocket(int socket_id, tcp::socket socket)
    // {
    //     std::lock_guard<std::mutex> lock(idToSocketMutex); // Lock the mutex for thread safety
    //     idToSocket[socket_id] = std::make_shared<tcp::socket>(std::move(socket));
    // }

    void check_for_match()
    {
        auto self = shared_from_this();
        asio::post(strand_, [this, self]()
                   {
            if (matched_) return;  // Skip if already matched
            int currentSocket = socket_.native_handle();
            int expectedPending = -1;
            if (pendingPlayer.compare_exchange_strong(expectedPending, currentSocket)) {
                // This player becomes the pending player
                std::cout << "Waiting for opponent. Socket: " << currentSocket << std::endl;
                wait_for_opponent();
            } else {
                // Match found
                int opponent = pendingPlayer.exchange(-1);
                matched_ = true;
                // Update the player relationships map
                
                game_manager.makeGame(currentSocket,opponent);
                std::cout << "Match started between sockets: " << opponent << " and " << currentSocket << std::endl;
                
                // Notify both players
                // notify_match_started(opponent, currentSocket);   
                notify_match_started(socket_);
                
                do_read();
                // do_send_periodic_messages();
            } });
    }

    void do_read()
    {
        auto self = shared_from_this();
        socket_.async_read_some(asio::buffer(read_buffer_),
                                asio::bind_executor(strand_, [this, self](boost::system::error_code ec, std::size_t length)
                                                    {
                if (!ec) {
                    std::string message ;
                    {
                       // Lock the mutex before accessing the shared buffer
                       std::lock_guard<std::mutex> lock(read_mutex);
                       message = decode_websocket_frame(read_buffer_, length);
                    }
                    std::cout << "Received: " << message << std::endl;
                             string response ;
                           try {
                                json request = json::parse(message);
                                string purpose = request["purpose"];
                    
                        json temp;
                       

                    //    std::lock_guard<std::mutex> lock(isPlayWithAi);
                    if (!matched_) {
                        if(purpose=="playWithAI")
                        {
                            matched_=true;
                            // cout<<"error is here 2"<<endl;
                            game_manager.makeGame(socket_.native_handle(),-1);
                              std::cout << "Match started between Ai and you sockets: " << -1 << " and " << socket_.native_handle() << std::endl;
                 
                               notify_match_started(socket_);

                        }
                        else if(purpose=="givemeopponentupdate"){

                        }
                        else
                        check_for_match();
                    }
                    else{
                        auto it = game_manager.findGameBySocket(socket_.native_handle());
                      std::shared_ptr<Game> game = it->second;
                      int opponent_socket_id = game->getOpponentSocket(socket_.native_handle());

                      std::string color = (socket_.native_handle() == game->white_socket) ? "white" : "black";
                      std::string opponent_color = (color == "white") ? "black" : "white";

                    if(purpose=="messageSent"){
                          if (it == game_manager.games.end()){
                              temp["erroringame"]="player not found invalid socket id ";
                              response = temp.dump();
                          }
                          else{
                                 if(color=="white"){

                                 game->whiteMessage=request["message"];
                                 }
                                 else{
                                 game->blackMessage = request["message"];
                                 }
                                 temp["messageSent"]="successfull";
                                 response=temp.dump();
                          }
                    }
                    else if(purpose == "givemeopponentupdate"){
                          if (it == game_manager.games.end()){
                              temp["erroringame"]="player not found invalid socket id ";
                          }
                          else{
                            {

                            std::lock_guard<std::mutex> lock(isCheckmateMutex);
                                  
                            if (is_checkmate(game->nboard, white))
                              {
                                cout << "Checkmate! " <<  "Black Wins !" << endl;
                                temp["status"] = "Checkmate";
                                temp["winColor"] = "Black" ;
                              }
                            else if(is_checkmate(game->nboard, black)){
                                cout << "Checkmate! " << "White Wins !" << endl;
                                temp["status"] = "Checkmate";
                                temp["winColor"] = "White";
                            }
                            //   else{
                             
                            }
                             if(color=="white"){
                                if(game->whitePlayer==""){
                                    game->whitePlayer=request["playerName"];
                                }
                             }
                             else{
                                if(game->blackPlayer==""){
                                    game->blackPlayer=request["playerName"];
                                }

                             }
                            temp["purpose"]="updateBoard";
                            temp["previousMove"]=game->previousMove;
                            temp["colorToUpdate"]=opponent_color;
                            temp["opponentName"]=(color == "white") ? game->blackPlayer : game->whitePlayer;
                            temp["message"]=(color=="white")?game->blackMessage:game->whiteMessage;
                            if(color=="white") game->blackMessage="";
                            else game->whiteMessage="";
                            //   }
                          

                          }
                        response= temp.dump();
                        
                    }
                    else{
                    pair<int,string> response_pair = game_manager.handleMove(socket_.native_handle(),message);
                    if(response_pair.first){
                        cout<<"valid move"<<endl;
                        response =  response_pair.second;
                    }
                    else{
                        cout << "Invalid move" << endl;
                        response = "Invalid move!";
                    }
                    } 
                    }
                              } catch (nlohmann::json::parse_error& e) {
                                  std::cerr << "Error parsing JSON: " << e.what() << std::endl;
                                  json temp;
                                  temp["ErrorIN"]="Error in parsing";
                                  response = temp.dump();  // Handle this gracefully by ignoring or logging the invalid message.
                            }

                    

                    
                    
                    std::string response_frame = encode_websocket_frame(response);
                    // cout<<"error is here 1"<<endl;
                    do_write(response_frame);

                    // Continue reading without checking for match again
                    do_read();
                } else {
                    std::cout << "Client disconnected or error occurred: " << ec.message() << std::endl;
                    socket_.close();
                } }));
    }
    void wait_for_opponent()
    {
        auto self = shared_from_this();
        timer_.expires_after(std::chrono::milliseconds(500));
        timer_.async_wait(asio::bind_executor(strand_, [this, self](boost::system::error_code ec)
                                              {
            if (!ec) {
                if (!matched_) {
                    // Check if we're still the pending player
                    if (pendingPlayer.load() == socket_.native_handle()) {
                        // Still waiting, check again
                        cout<<"Waitign again"<<endl;
                        wait_for_opponent();
                    } else {
                        // We've been matched
                        cout<<"Waiting completed! match have been started also for "<<socket_.native_handle()<<endl;
                        notify_match_started(socket_);
                        matched_ = true;
                        do_read();
                        // do_send_periodic_messages();
                    }
                }
            } else if (ec != asio::error::operation_aborted) {
                std::cerr << "Timer error: " << ec.message() << std::endl;
            } }));
    }

    void do_send_periodic_messages()
    {
        auto self = shared_from_this();
        timer_.expires_after(std::chrono::seconds(10));
        timer_.async_wait(asio::bind_executor(strand_, [this, self](boost::system::error_code ec)
                                              {
            if (!ec) {
                // std::string message = "Periodic update: Your socket ID is " + 
                //                       std::to_string(socket_.native_handle()) + 
                //                       ". Your opponent's socket ID is " + 
                //                       std::to_string(opponent_socket_id_);
                int currentSocket = socket_.native_handle();
                std::string message;
                 {
                    std::lock_guard<std::mutex> lock(relationships_mutex);
                    auto it = player_relationships.find(currentSocket);
                    if (it != player_relationships.end()) {
                        message = "Periodic update: Your socket ID is " + std::to_string(currentSocket) + 
                                  ". Your opponent's socket ID is " + std::to_string(it->second);
                    } else {
                        message = "Periodic update: Your socket ID is " + std::to_string(currentSocket) + 
                                  ". Waiting for an opponent...";
                    }
                }
                
                std::string encoded_message = encode_websocket_frame(message);
                asio::async_write(socket_, asio::buffer(encoded_message),
                    asio::bind_executor(strand_, [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                        if (!ec) {
                            std::cout << "Sent periodic message to client: " << socket_.native_handle() << std::endl;
                            // Continue sending periodic messages
                            do_send_periodic_messages();
                        } else {
                            std::cout << "Error sending periodic message: " << ec.message() << std::endl;
                            socket_.close();
                        }
                    })
                );
            } else {
                std::cout << "Timer error in do_send_periodic_messages: " << ec.message() << std::endl;
            } }));
    }

  
    asio::steady_timer timer_;
    std::string buffer_;
    char read_buffer_[BUFFER_SIZE];
    // Board board;
    bool matched_;
    int opponent_socket_id_; // New member to store opponent's socket ID
};
string handle_request(const string &json_request, Board &board,int socketId)
{
    std::cout << "Raw JSON request: " << json_request << std::endl;

    json request = json::parse(json_request);
    json response;

    string purpose = request["purpose"];

    if (purpose == "generateMove")
    {
        string position = request["position"];
        char col = position[0];
        int row = position[1] - '0';
        string pieceName = request["pieceName"];
        vector<vector<string>> possible_moves;

        Bitboard bit = 1;
        Bitboard from_mask = bit << ((row * 8) - (col - 97) - 1);
        int currentPlayer = board.ouccupancy[white] & from_mask ? white : black;

        string kingPositionInString = kingPosition(board, currentPlayer);
        cout << "king position : " << kingPositionInString << endl;
        char kingCol = kingPositionInString[0];
        int kingRow = kingPositionInString[1] - 48;

        // First we check for checkmate , we check that either king have safe place to move or not

        if (is_checkmate(board, currentPlayer))
        {
            cout << "Checkmate! " << (currentPlayer == white ? "Black" : "White") << " Wins !" << endl;
            response["status"] = "Checkmate";
            response["winColor"] = (currentPlayer == white ? "Black" : "White");
            return response.dump();
        }

        // Indirect check : moving of other piece leads to check of its king

        if (pieceName != "king")
        {
            board.ouccupancy[currentPlayer] &= ~from_mask;
            board.ouccupancy[both] &= ~from_mask;

            if (is_king_in_check(board, currentPlayer))
            {
                cout << "Allowed Empty move: " << endl;
                cout << "name: " << board.name << endl;
                for (auto moves : board.emptySquares)
                    cout << moves << " ";
                cout << endl;
                int is_pice_between_cheker_and_king = 0;
                for (auto i : board.emptySquares)
                {
                    if (i == position)
                        is_pice_between_cheker_and_king = 1;
                }
                board.ouccupancy[currentPlayer] = board.ouccupancy[currentPlayer] | from_mask;
                board.ouccupancy[both] = board.ouccupancy[both] | from_mask;
                if (is_pice_between_cheker_and_king)
                {
                    cout << "indirect check , you cannot move" << endl;

                    if (pieceName == "queen")
                    {
                        possible_moves = get_queen_moves(board, col, row);
                    }
                    else if (pieceName == "rook")
                    {
                        possible_moves = get_rook_moves(board, col, row);
                    }
                    else if (pieceName == "bishop")
                    {
                        possible_moves = get_bishop_moves(board, col, row);
                    }
                    else if (pieceName == "knight")
                    {
                        response["possibleMoves"] = "";
                        response["possibleCaptures"] = "";
                        return response.dump();
                    }
                    else if (pieceName == "pawn")
                    {
                        possible_moves = get_pawn_moves(board, col, row);
                    }

                    for (auto moves : board.emptySquares)
                        cout << moves << " ";
                    cout << endl;

                    for (int i = 0; i <= 1; i++)
                    {
                        vector<string> vec1 = possible_moves[i];
                        vector<string> vec2 = board.emptySquares;
                        sort(vec1.begin(), vec1.end());
                        sort(vec2.begin(), vec2.end());

                        vector<string> intersection;

                        // Find the intersection of vec1 and vec2
                        set_intersection(
                            vec1.begin(), vec1.end(),
                            vec2.begin(), vec2.end(),
                            back_inserter(intersection));

                        // Output the intersection
                        cout << "Intersection: ";
                        for (const auto &item : intersection)
                        {
                            cout << item << " ";
                        }
                        cout << endl;
                        if (i == 0)
                        {
                            response["possibleMoves"] = intersection;
                        }
                        if (i == 1)
                        {
                            response["possibleCaptures"] = intersection;
                        }
                    }

                    return response.dump();
                }
            }
            else
            {
                board.ouccupancy[currentPlayer] = board.ouccupancy[currentPlayer] | from_mask;
                board.ouccupancy[both] = board.ouccupancy[both] | from_mask;
            }
        }

        // direct check
        //  Check if the current player is in check
        if (is_king_in_check(board, currentPlayer))
        {
            cout << "Allowed Empty move: " << endl;
            cout << "name: " << board.name << endl;
            for (auto moves : board.emptySquares)
                cout << moves << " ";
            cout << endl;
            // cout<<"finding knight "<<endl;
            string isCheckByKnight = find_knight_checking_king(board, kingCol, kingRow, currentPlayer);
            // for(auto i : isCheckByKnight)
            // cout<<i<<" ";
            // cout<<endl;
            // cout<<"finding knight completed "<<endl;

            if (!isCheckByKnight.empty())
            {
                cout << "check is given by knight position : " << isCheckByKnight << endl;
                if (pieceName == "knight")
                {
                    possible_moves = get_knight_moves(board, col, row);
                }
                else if (pieceName == "king")
                {
                    possible_moves = get_king_moves(board, col, row);
                    response["possibleMoves"] = possible_moves[0];
                    response["possibleCaptures"] = possible_moves[1];
                    return response.dump();
                }
                else if (pieceName == "queen")
                {
                    possible_moves = get_queen_moves(board, col, row);
                }
                else if (pieceName == "pawn")
                {
                    possible_moves = get_pawn_moves(board, col, row);
                }
                else if (pieceName == "rook")
                {
                    possible_moves = get_rook_moves(board, col, row);
                }
                else if (pieceName == "bishop")
                {
                    possible_moves = get_bishop_moves(board, col, row);
                }

                vector<string> intersection;

                cout << "Intersection: ";
                for (auto i : possible_moves[1])
                    if (i == isCheckByKnight)
                        intersection.push_back(i);
                for (const auto &item : intersection)
                {
                    cout << item << " ";
                }
                cout << endl;
                response["possibleMoves"] = "";
                response["possibleCaptures"] = intersection;

                return response.dump();
            }

            cout << (currentPlayer == white ? "White" : "Black") << " is in check!" << endl;
            // if (is_checkmate(board, currentPlayer))
            // {
            //     cout << (currentPlayer == white ? "Black" : "White") << " wins by checkmate!" << endl;
            //     response["status"] = "checkmate";
            //     response["message"] = "king is in checkmate";
            //     return response.dump();
            // }
            // response["status"] = "check";
            // response["message"] = "king is not in checkmate";

            if (pieceName == "knight")
            {
                possible_moves = get_knight_moves(board, col, row);
            }
            else if (pieceName == "king")
            {
                possible_moves = get_king_moves(board, col, row);
                response["possibleMoves"] = possible_moves[0];
                response["possibleCaptures"] = possible_moves[1];
                return response.dump();
            }
            else if (pieceName == "queen")
            {
                possible_moves = get_queen_moves(board, col, row);
            }
            else if (pieceName == "pawn")
            {
                possible_moves = get_pawn_moves(board, col, row);
            }
            else if (pieceName == "rook")
            {
                possible_moves = get_rook_moves(board, col, row);
            }
            else if (pieceName == "bishop")
            {
                possible_moves = get_bishop_moves(board, col, row);
            }

            // Sort both vectors before applying set_intersection
            for (int i = 0; i <= 1; i++)
            {
                vector<string> vec1 = possible_moves[i];
                vector<string> vec2 = board.emptySquares;
                sort(vec1.begin(), vec1.end());
                sort(vec2.begin(), vec2.end());

                vector<string> intersection;

                // Find the intersection of vec1 and vec2
                set_intersection(
                    vec1.begin(), vec1.end(),
                    vec2.begin(), vec2.end(),
                    back_inserter(intersection));

                // Output the intersection
                cout << "Intersection: ";
                for (const auto &item : intersection)
                {
                    cout << item << " ";
                }
                cout << endl;
                if (i == 0)
                {
                    response["possibleMoves"] = intersection;
                }
                if (i == 1)
                {
                    response["possibleCaptures"] = intersection;
                }
            }

            return response.dump();
        }
        // else if (is_stalemate(board, currentPlayer))
        // {
        //     cout << "The game is a draw by stalemate!" << endl;
        //     response["status"] = "stalemate";
        //     response["message"] = "king is in stalemate";
        //     return response.dump();
        // }
        else
        {
            cout << (currentPlayer == white ? "White" : "Black") << " is not in check!" << endl;
        }

        if (pieceName == "knight")
        {
            possible_moves = get_knight_moves(board, col, row);
        }
        else if (pieceName == "king")
        {
            possible_moves = get_king_moves(board, col, row);
        }
        else if (pieceName == "queen")
        {
            possible_moves = get_queen_moves(board, col, row);
        }
        else if (pieceName == "pawn")
        {
            possible_moves = get_pawn_moves(board, col, row);
        }
        else if (pieceName == "rook")
        {
            possible_moves = get_rook_moves(board, col, row);
        }
        else if (pieceName == "bishop")
        {
            possible_moves = get_bishop_moves(board, col, row);
        }

        response["possibleMoves"] = possible_moves[0];
        vector<string> temp;
        string opponentKingPosition = kingPosition(board, !currentPlayer);
        for (auto i : possible_moves[1])
        {
            if (i != opponentKingPosition)
                temp.push_back(i);
        }
        response["possibleCaptures"] = temp;
    }
    else if (purpose == "updateBoard")
    {
        string move = request["position"];
        // Process the move on the board
        // bool success = update_board(board, move);
        cout << "updated move : ";
        for (auto i : move)
            cout << i << " ";
        cout << endl;
        bool success = moveGeneration(move, board);
        printBoard(board);
        cout<<"socket id by mapping is : "<<sessions_[socketId]->socket_.native_handle()<<endl;
        auto it = game_manager.findGameBySocket(socketId);
        auto self = sessions_[socketId];
        if (it != game_manager.games.end() && it->second->isAIGame)
        {
            auto game = it->second;
            boost::asio::post(self->strand_, [self,  game, &board]() {
            std::string aiMove = getBestMove(board, black, 5);
            
            boost::asio::post(self->strand_, [self, aiMove, game,&board]() {
                moveGeneration(aiMove, board);
                std::cout << "AI move is: " << aiMove << std::endl;
                game->previousMove = aiMove;
                game->switchTurn();
                
                json aiResponse;
                aiResponse["Aiposition"] = aiMove;
                aiResponse["isAIMove"] = true;
                
                // do_write(aiResponse.dump());
            });
        });
    } else {
        std::cout << "Error! Game not found with AI" << std::endl;
    }
        

        response["status"] = success ? "success" : "error";
        response["position"] = move;
    }

    return response.dump(); // Convert JSON response to string
}

class WebSocketServer
{
public:
    WebSocketServer(asio::io_context &context, short port)
        : acceptor_(context, tcp::endpoint(tcp::v4(), port)),
          context_(context)
    {
        cout << "Server is started and Listening on port " << port << endl;
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    std::make_shared<WebSocketSession>(std::move(socket), context_)->start();
                }
                do_accept(); // Continue accepting new connections
            });
    }

    tcp::acceptor acceptor_;
    asio::io_context &context_;
    // Board board;
};

int main()
{
    try
    {
        // Board board;
        // string moves;
        // initializeBoard(board);
        // printBoard(board);
        asio::io_context context;
        WebSocketServer server(context, PORT);

        asio::thread_pool pool(8); // Thread pool with 4 threads
        for (int i = 0; i < 8; ++i)
        {
            asio::post(pool, [&context]
                       { context.run(); });
        }

        pool.join();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}