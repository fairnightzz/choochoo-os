typedef enum
{
  RPS_SIGNUP,
  RPS_PLAY,
  RPS_QUIT,
} RPSMessageType;

typedef enum
{
  MOVE_NONE = 0,
  MOVE_ROCK,
  MOVE_PAPER,
  MOVE_SCISSORS,
} RPSMove;

typedef enum
{
  RESULT_NONE = 0,   // Undefined behaviour
  RESULT_INCOMPLETE, // the other player quits
  RESULT_WIN,
  RESULT_LOSE,
  RESULT_TIE,
} RPSResult;

typedef struct
{
  RPSMessageType type;
  RPSMove move;
} RPSMessage;

typedef struct
{
  RPSMessageType type;
  RPSResult res;
} RPSResponse;

typedef struct
{
  bool gameComplete;
  int p1;
  int p2;
  RPSMove p1Move;
  RPSMove p2Move;
} RPSGameState;

int Signup(int server);
int Play(int server, RPSMove move);
int Quit(int server);
