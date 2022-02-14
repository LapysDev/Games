/* Import > ... */
#include <cctype>
#include <climits>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <new>
#include <stdint.h>

#if defined(__APPLE__) || defined(__gnu_linux__) || defined(linux) || defined(__linux) || defined(__linux__) || defined(__MACH__) || defined(__unix) || defined(__unix__)
# include <conio.h>
#elif defined(__NT__) || defined(__TOS_WIN__) || defined(_WIN16) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN32_WCE) || defined(_WIN64) || defined(__WINDOWS__)
# include <windows.h>
#endif

/* Definition > ... */
#define TITLE "Snake"

class Fruit;
class Snake;
class Object;
class Wall;

/* Namespace > ... */
namespace Game {
  namespace Assets {
    static char const BORDER_BOTTOM = '_';
    static char const BORDER_LEFT   = '|';
    static char const BORDER_RIGHT  = '|';
    static char const BORDER_TOP    = '-';
    static char const CORNERS    [] = {'/', '\\', '\\', '/'};
    static char const DECORATIONS[] = {'.', '^', '`'};
    static char const FLOOR         = ' ';

    static char const BONUS_FRUIT        [] = "BB" "\0" "BB" "\0";
    static char const FRUIT              [] = "FF" "\0" "FF" "\0";
    static char const INVINCIBLE_FRUIT   [] = "II" "\0" "II" "\0";
    static char const SNAKE_EAST_TO_NORTH[] = "S " "\0" "ss" "\0";
    static char const SNAKE_EAST_TO_SOUTH[] = "ss" "\0" "S " "\0";
    static char const SNAKE_HEAD         [] = "SS" "\0" "SS" "\0";
    static char const SNAKE_NORTH_TO_EAST[] = "s " "\0" "sS" "\0";
    static char const SNAKE_NORTH_TO_WEST[] = " s" "\0" "Ss" "\0";
    static char const SNAKE_SOUTH_TO_EAST[] = "sS" "\0" "s " "\0";
    static char const SNAKE_SOUTH_TO_WEST[] = "Ss" "\0" " s" "\0";
    static char const SNAKE_TAIL         [] = "ss" "\0" "ss" "\0";
    static char const SNAKE_WEST_TO_NORTH[] = " S" "\0" "ss" "\0";
    static char const SNAKE_WEST_TO_SOUTH[] = "ss" "\0" " S" "\0";
    static char const WALL               [] = "##" "\0" "##" "\0";

    // ... --- MINIFY (Lapys)
    inline std::size_t getHeight(char const asset[]) { static std::size_t height = 0u; if (0u != height) return height; while (true) if ('\0' == *(asset++)) { ++height; if ('\0' == *asset) return 0u == height ? 1u : height; } }
    inline std::size_t getWidth (char const asset[]) { std::size_t count = 0u; static std::size_t width = 0u; if (0u != width) return width; while (true) { if ('\0' == *(asset++)) { width = count > width ? count : width; count = 0u; if ('\0' == *asset) { return 0u == width ? 1u : width; } continue; } ++count; } }
  }

  static char const *const ASSETS[]   = {Assets::BONUS_FRUIT, Assets::FRUIT, Assets::INVINCIBLE_FRUIT, Assets::SNAKE_EAST_TO_NORTH, Assets::SNAKE_EAST_TO_SOUTH, Assets::SNAKE_HEAD, Assets::SNAKE_NORTH_TO_EAST, Assets::SNAKE_NORTH_TO_WEST, Assets::SNAKE_SOUTH_TO_EAST, Assets::SNAKE_SOUTH_TO_WEST, Assets::SNAKE_TAIL, Assets::SNAKE_WEST_TO_NORTH, Assets::SNAKE_WEST_TO_SOUTH, Assets::WALL};
  static std::size_t COLUMN_COUNT     = 20u/*64u*/;
  static std::size_t CURRENT_SCORE    = 0u;
  static char       *DISPLAY          = NULL;
  static std::size_t DISPLAY_CAPACITY = 0u;
  static std::size_t FRUIT_CAPACITY   = 0u;
  static std::size_t FRUIT_COUNT      = 0u;
  static Fruit      *FRUITS           = NULL;
  static std::size_t HIGH_SCORE       = 0u;
  static std::size_t ROW_COUNT        = 10u/*48u*/;
  static FILE       *SCORE_FILE       = NULL;
  static Snake      *SNAKE            = NULL;
  static std::size_t WALL_COUNT       = 0u;
  static Wall       *WALLS            = NULL;

  static char const HEADER[] = {
    "Welcome to " TITLE                                                  "\r\n\n"
    "===================="                                                 "\r\n"
    "*" " Arrow keys direct the (S)nake"                                   "\r\n"
    "*" "   eat the (F)ruits to get points,"                               "\r\n"
    "*" "   don't eat yourself!"                                         "\r\n\n"
    "*" "   The (B)onus Fruit gives you extra points \\(^_^)/"             "\r\n"
    "*" "   The (I)nvincibility Fruit makes you unstoppable |_(*o * )__|"  "\r\n"
    "*" " Key `I` to toggle Information (during gameplay)"                 "\r\n"
    "*" " Key `Q` to Quit"

    "\r\n"
  };

  static char const INFORMATION[] = {
    "\r\n"

    "==================== ..." "\r\n"
    "||" " SCORE: %u" "%a +%u%a" "\r\n"
    "||" " HIGH SCORE: %u"
    "%a" "\r\n" "||" " INVINCIBLE" "%a"
    "\r\n" "||" " [%u, %u] in [%u, %u]"
    "%a" "\r\n" "GAMEOVER !!" "\r\n\n" "  %S" "\r\n" "  Save your score? [Y]es / [N]o" "\r\n" "%a"
  };
}

/* Class > ... */
class Object {
  private:
    inline static std::size_t getColumn(std::size_t const coordinates) { return coordinates % Game::COLUMN_COUNT; }
    inline static std::size_t getRow   (std::size_t const coordinates) { return coordinates / Game::COLUMN_COUNT; }

  public:
    // ... --- MINIFY (Lapys)
    static struct height_t { inline operator std::size_t() const { static std::size_t height = 0u; if (0u == height) { for (char const *const *asset = Game::ASSETS; asset != Game::ASSETS + (sizeof(Game::ASSETS) / sizeof(char const*)); ++asset) { std::size_t const assetHeight = Game::Assets::getHeight(*asset); if (assetHeight > height) height = assetHeight; } } return height; } } const HEIGHT;
    static struct size_t   { inline operator std::size_t() const { return Object::HEIGHT > Object::WIDTH ? Object::HEIGHT : Object::WIDTH; } }                                                                                                                                                                                                                                                const SIZE;
    static struct width_t  { inline operator std::size_t() const { static std::size_t width = 0u; if (0u == width) { for (char const *const *asset = Game::ASSETS; asset != Game::ASSETS + (sizeof(Game::ASSETS) / sizeof(char const*)); ++asset) { std::size_t const assetWidth = Game::Assets::getWidth(*asset); if (assetWidth > width) width = assetWidth; } } return width; } }          const WIDTH;

    // ... --- WARN (Lapys) -> Direct member function access is undefined behavior
    union {
      struct {
        private: std::size_t coordinates;
        public:
          inline std::size_t operator =(std::size_t const coordinate) { this -> coordinates = coordinate + (Game::COLUMN_COUNT * Object::getRow(this -> coordinates)); return this -> operator std::size_t(); }
          inline operator std::size_t() const { return Object::getColumn(this -> coordinates); }
      } column;

      struct {
        private: std::size_t coordinates;
        public:
          inline std::size_t operator =(std::size_t const coordinate) { this -> coordinates = Object::getColumn(this -> coordinates) + (Game::COLUMN_COUNT * coordinate); return this -> operator std::size_t(); }
          inline operator std::size_t() const { return Object::getRow(this -> coordinates); }
      } row;
    };
};
  Object::height_t const Object::HEIGHT = Object::height_t();
  Object::size_t   const Object::SIZE   = Object::size_t();
  Object::width_t  const Object::WIDTH  = Object::width_t();

  // ...
  class Fruit : public Object {
    public:
      enum { BONUS = 0x1u, INVINCIBLE } type;
  };

  // ...
  class Snake : public Object {
    public:
      class Tail : public Object {};

      class TailCollection {
        public:
          struct {
            friend class TailCollection;

            private: void *value;
            public: inline operator std::size_t() const { return NULL != this -> value ? *static_cast<std::size_t*>(this -> value) : 0u; }
          } length;

          inline operator Tail*() const {
            if (NULL == this -> length.value)
            return NULL;

            for (unsigned char *value = static_cast<unsigned char*>(this -> length.value) + sizeof(std::size_t); ; ++value) {
              if (0u == static_cast<std::size_t>(value - static_cast<unsigned char*>(this -> length.value)) % sizeof(Tail))
              return reinterpret_cast<Tail*>(value);
            }
          }
      } tail;
  };

  // ...
  class Wall : public Object {};

/* Function */
static void draw() {
  struct Graphics {
    inline static void drawAsset(char const asset[], std::size_t const xOrigin, std::size_t const yOrigin) {
      char *const DISPLAY = Game::DISPLAY + 5u + (Game::COLUMN_COUNT * Object::SIZE);
      std::size_t x = xOrigin;
      std::size_t y = yOrigin;

      for (char const *pixel = asset; ; ++pixel) {
        if ('\0' == *pixel) {
          x = xOrigin; ++y;

          if ('\0' == *(pixel + 1)) return; // --> break
          continue;
        }

        DISPLAY[x++ + (y * 4u) + (y * Game::COLUMN_COUNT * Object::SIZE)] = *pixel;
      }
    }
  };

  // ...
  for (Wall const *wall = Game::WALLS; NULL != Game::WALLS && wall != Game::WALLS + Game::WALL_COUNT; ++wall)
  Graphics::drawAsset(Game::Assets::WALL, Object::SIZE * launder(wall) -> column, Object::SIZE * launder(wall) -> row);
}

// ...
static void input(char const key) {
  std::printf("[" TITLE "] '%c'" "\r\n", key);
  switch (key) {
    case 'A': break;
    case 'D': break;
    case 'S': break;
    case 'W': break;
  }
}

// ...
#ifdef __cpp_lib_launder
  using std::launder;
#else
  template <typename type>
  inline static void launder(type* const) {}
#endif

// ...
static void quit() {
  std::free(Game::DISPLAY);
  Game::DISPLAY = NULL;

  std::free(Game::WALLS);
  Game::WALLS = NULL;

  if (NULL != Game::SCORE_FILE) {
    std::fclose(Game::SCORE_FILE);
    Game::SCORE_FILE = NULL;
  }

  // ...
  std::fputs("\r\n" "[" TITLE "] Program terminating", stderr);
  std::exit(EXIT_SUCCESS);
}

inline static void quit(int const signal) {
  switch (signal) {
    case SIGINT : std::fputs("\r\n" "[" TITLE "] Program interrupted...",                stderr); break;
    case SIGSEGV: std::fputs("\r\n" "[" TITLE "] Program requested unpermitted data...", stderr); break;
    default: break;
  }

  quit();
}

// ...
std::size_t const (&randseed(std::size_t const reseed = 0u))[4] {
  static std::size_t seeds[4] = {0u, 0x2A430C43u, 0u, 0xDE83A17u}; // ->> pre-seeded to `0u`

  // ...
  if (0u != reseed)
  for (std::size_t *seed = seeds + 4, state; seed != seeds; ) {
    state  = reseed + (0x9E3779B9u * ((seed - seeds) >> 1u));
    state ^= state >> 15u; state *= 0x85EBCA6Bu;
    state ^= state >> 13u; state *= 0xC2B2AE35u;
    state ^= state >> 16u;

    *--seed = state;
    *--seed = state >> sizeof(uint_least32_t);
  }

  return seeds;
}

std::size_t randint(std::size_t const range) {
  std::size_t (&seeds)[4] = const_cast<std::size_t (&)[4]>(randseed());
  std::size_t const control = seeds[1] << 17u;
  std::size_t const random  = seeds[0] + seeds[3];

  // ...
  seeds[2] ^= seeds[0];
  seeds[3] ^= seeds[1];
  seeds[1] ^= seeds[2];
  seeds[0] ^= seeds[3];

  seeds[2] ^= control;
  seeds[3]  = (seeds[3] << 45u) | (seeds[3] >> 19u);

  return random % range;
}

/* Main */
int main(int, char* arguments[]) {
  Snake             snake = Snake();
  std::time_t const time  = std::time(static_cast<std::time_t*>(NULL));

  // ...
  std::atexit(static_cast<void (*)()>(&quit));
  std::setbuf(stderr, NULL);
  std::signal(SIGINT,  static_cast<void (*)(int)>(&quit));
  std::signal(SIGSEGV, static_cast<void (*)(int)>(&quit));

  randseed(time == static_cast<std::time_t>(-1) ? 0u : static_cast<std::size_t>(time));

  // ...
  if (NULL != arguments[1]) {
    if (NULL == arguments[2]) {
      std::fputs("[" TITLE "] Expected both width then height to be specified", stderr);
      return EXIT_SUCCESS;
    }

    else for (std::size_t *const counts[] = {&Game::COLUMN_COUNT, &Game::ROW_COUNT}, *const *count = counts; count != counts + (sizeof(counts) / sizeof(std::size_t*)); ++count) {
      bool        invalid  = false;
      bool        overflow = false;
      std::size_t value    = 0u;

      // ...
      for (char *argument = *++arguments; '\0' != *argument && (false == invalid && false == overflow); ++argument) {
        unsigned char digit;

        // ...
        switch (*argument) {
          case '0': digit = 0u; break;
          case '1': digit = 1u; break;
          case '2': digit = 2u; break;
          case '3': digit = 3u; break;
          case '4': digit = 4u; break;
          case '5': digit = 5u; break;
          case '6': digit = 6u; break;
          case '7': digit = 7u; break;
          case '8': digit = 8u; break;
          case '9': digit = 9u; break;
          default: invalid = true; break;
        }

        if (invalid) continue;
        overflow = value > SIZE_MAX / 10u;
        value    = digit + (value * 10u);
      }

      if (invalid || overflow) {
        std::fprintf(stderr, invalid ? "[" TITLE "] Invalid %5.6s specified: `" : "[" TITLE "] The %5.6s specified is too large: `", &Game::COLUMN_COUNT == *count ? "width" : "height");

        for (char *argument = *arguments; '\0' != *argument; std::fputc(*argument++, stderr))
        switch (*argument) {
          case '`'  : std::fputc('\\',   stderr); break;
          case '\0' : std::fputs("\\0",  stderr); break;
          case '\1' : std::fputs("\\1",  stderr); break;
          case '\2' : std::fputs("\\2",  stderr); break;
          case '\3' : std::fputs("\\3",  stderr); break;
          case '\4' : std::fputs("\\4",  stderr); break;
          case '\5' : std::fputs("\\5",  stderr); break;
          case '\6' : std::fputs("\\6",  stderr); break;
          case '\7' : std::fputs("\\7",  stderr); break;
          case '\16': std::fputs("\\16", stderr); break;
          case '\17': std::fputs("\\17", stderr); break;
          case '\20': std::fputs("\\20", stderr); break;
          case '\21': std::fputs("\\21", stderr); break;
          case '\22': std::fputs("\\22", stderr); break;
          case '\23': std::fputs("\\23", stderr); break;
          case '\24': std::fputs("\\24", stderr); break;
          case '\25': std::fputs("\\25", stderr); break;
          case '\26': std::fputs("\\26", stderr); break;
          case '\27': std::fputs("\\27", stderr); break;
          case '\30': std::fputs("\\30", stderr); break;
          case '\31': std::fputs("\\31", stderr); break;
          case '\b' : std::fputs("\\b",  stderr); break;
          case '\f' : std::fputs("\\f",  stderr); break;
          case '\n' : std::fputs("\\n",  stderr); break;
          case '\r' : std::fputs("\\r",  stderr); break;
          case '\t' : std::fputs("\\t",  stderr); break;
          case '\v' : std::fputs("\\v",  stderr); break;
        }

        std::fprintf(stderr, invalid ? "` (expected a contiguous collection of digits between 0-9)" : "` (value should be smaller than `%lu%2s`)", static_cast<unsigned long>(SIZE_MAX), "zu");
        return EXIT_SUCCESS;
      }

      **count = value;
    }
  }

  Game::DISPLAY_CAPACITY = (
    // (1u           * 2u * 2u) +                                        // ->> CRLF newlines (for bottom & top borders); no idea why removing this works...
    (Object::SIZE * 2u * Game::ROW_COUNT) +                              // ->> CRLF newlines (for board)
    (((Game::COLUMN_COUNT * Object::SIZE) + 2u) * 2u) +                  // ->> Bottom & top borders
    (((Game::ROW_COUNT    * Object::SIZE) + 2u) * 2u) +                  // ->> Left & right borders
    (Game::COLUMN_COUNT * Game::ROW_COUNT * Object::SIZE * Object::SIZE) // ->> Board
  );

  Game::DISPLAY          = static_cast<char*>(std::calloc(Game::DISPLAY_CAPACITY + 1u, sizeof(char)));
  Game::SCORE_FILE       = std::fopen("scores.txt", "rb");
  Game::SNAKE            = &snake;
  Game::WALL_COUNT       = randint(((Game::COLUMN_COUNT * Game::ROW_COUNT) * 1u) / 10u);
  Game::WALLS            = static_cast<Wall*>(0u == Game::WALL_COUNT ? NULL : std::malloc(Game::WALL_COUNT * sizeof(Wall)));

  static_cast<void>(Game::CURRENT_SCORE);
  static_cast<void>(Game::FRUIT_CAPACITY);
  static_cast<void>(Game::FRUIT_COUNT);
  static_cast<void>(Game::FRUITS);

  // [Display]
  if (NULL == Game::DISPLAY) {
    std::fputs("[" TITLE "] Unable to render game graphics", stderr);
    return EXIT_SUCCESS;
  }

  else {
    for (char *pixel = Game::DISPLAY + Game::DISPLAY_CAPACITY; Game::DISPLAY != pixel; )
    *--pixel = '@';

    char *pixel = Game::DISPLAY + Game::DISPLAY_CAPACITY;

    // ...
    *pixel = '\0';

    *--pixel = '\n';
    *--pixel = '\r';
    *--pixel = Game::Assets::CORNERS[3];
      for (std::size_t count = Game::COLUMN_COUNT * Object::SIZE; count; --count)
      *--pixel = Game::Assets::BORDER_BOTTOM;
    *--pixel = Game::Assets::CORNERS[2];

    for (std::size_t row = Game::ROW_COUNT * Object::SIZE; row; --row) {
      *--pixel = '\n';
      *--pixel = '\r';
      *--pixel = Game::Assets::BORDER_RIGHT;
        for (std::size_t column = Game::COLUMN_COUNT * Object::SIZE; column; --column)
        *--pixel = randint(Game::COLUMN_COUNT * Game::ROW_COUNT) < ((Game::COLUMN_COUNT * Game::ROW_COUNT) * 5u) / 100u ? Game::Assets::DECORATIONS[randint(sizeof(Game::Assets::DECORATIONS) / sizeof(char))] : Game::Assets::FLOOR;
      *--pixel = Game::Assets::BORDER_LEFT;
    }

    *--pixel = '\n';
    *--pixel = '\r';
    *--pixel = Game::Assets::CORNERS[1];
      for (std::size_t count = Game::COLUMN_COUNT * Object::SIZE; count; --count)
      *--pixel = Game::Assets::BORDER_TOP;
    *--pixel = Game::Assets::CORNERS[0];
  }

  // [Score]
  if (NULL != Game::SCORE_FILE) {
    bool        invalid = false;
    bool        parsing = false;
    std::size_t score   = 0u;

    do {
      unsigned char digit = 10u;
      int const character = std::fgetc(Game::SCORE_FILE);

      // ...
      if (EOF == character)
      break;

      switch (static_cast<char>(character)) {
        case '0': digit = 0u; break;
        case '1': digit = 1u; break;
        case '2': digit = 2u; break;
        case '3': digit = 3u; break;
        case '4': digit = 4u; break;
        case '5': digit = 5u; break;
        case '6': digit = 6u; break;
        case '7': digit = 7u; break;
        case '8': digit = 8u; break;
        case '9': digit = 9u; break;
        case ',': invalid = false == parsing; parsing = false; break;
        case ' ': invalid = parsing; break;
        default : invalid = true;    break;
      }

      if (digit != 10u) {
        if (false == parsing) {
          Game::HIGH_SCORE = Game::HIGH_SCORE > score ? Game::HIGH_SCORE : score;
          parsing = true;
          score   = 0u;
        }

        if (score > SIZE_MAX / 10u) invalid = true;
        else score = digit + (score * 10u);
      }
    } while (false == invalid);

    if (invalid) {
      Game::HIGH_SCORE = 0u;
      std::fputs("[" TITLE "] Unable to retrieve high score, time for a new record then!", stderr);
    }

    std::fclose(Game::SCORE_FILE);
    Game::SCORE_FILE = NULL;
  }

  // [Walls]
  if (NULL == Game::WALLS)
  Game::WALL_COUNT = 0u;

  for (Wall *wall = Game::WALLS; NULL != Game::WALLS && wall != Game::WALLS + Game::WALL_COUNT; ++wall) {
    Wall *const currentWall = new (wall) Wall();

    for (bool recoordinate = true; recoordinate; ) {
      currentWall -> column = randint(Game::COLUMN_COUNT);
      currentWall -> row    = randint(Game::ROW_COUNT);
      recoordinate          = false;

      for (Wall const *recentWall = wall; Game::WALLS != recentWall && (false == recoordinate); ) {
        --recentWall;
        recoordinate = (wall -> column == launder(recentWall) -> column) && (wall -> row == launder(recentWall) -> row);
      }
    }
  }

  // [Gameplay]
  #if defined(__APPLE__) || defined(__gnu_linux__) || defined(linux) || defined(__linux) || defined(__linux__) || defined(__MACH__) || defined(__unix) || defined(__unix__)
    while (true) ...;
  #elif defined(__NT__) || defined(__TOS_WIN__) || defined(_WIN16) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN32_WCE) || defined(_WIN64) || defined(__WINDOWS__)
    bool         bufferRendering                             = false;
    COORD        consoleCursorPosition                       = COORD();
    HANDLE       consoleInputHandle                          = INVALID_HANDLE_VALUE;
    HANDLE       consoleOutputHandle                         = INVALID_HANDLE_VALUE;
    char const (*consoleTitle)[sizeof(TITLE) / sizeof(char)] = NULL;
    HWND         consoleWindow                               = NULL;
    bool         rendered                                    = false;

    while (true) {
      if (INVALID_HANDLE_VALUE == consoleInputHandle)
      consoleInputHandle = ::GetStdHandle(STD_INPUT_HANDLE);

      if (INVALID_HANDLE_VALUE == consoleOutputHandle)
      consoleOutputHandle = ::GetStdHandle(STD_OUTPUT_HANDLE);

      // ...
      if (NULL == consoleTitle) {
        if (FALSE != ::SetConsoleTitle(TITLE))
        consoleTitle = &TITLE;
      }

      if (NULL == consoleTitle) {
        if (NULL == consoleWindow) consoleWindow = ::GetConsoleWindow();
        if (NULL != consoleWindow) { if (FALSE != ::SetWindowTextA(consoleWindow, TITLE)) consoleTitle = &TITLE; }
      }

      // ...
      if (INVALID_HANDLE_VALUE == consoleInputHandle)  continue;
      if (INVALID_HANDLE_VALUE == consoleOutputHandle) continue;

      if (bufferRendering)
      ::WaitForSingleObject(consoleInputHandle, INFINITE);

      if (false == bufferRendering || WAIT_OBJECT_0 == ::WaitForSingleObject(consoleInputHandle, 0u)) {
        static INPUT_RECORD  inputRecord;
        static std::size_t   inputRecordCapacity = 0u;
        DWORD                inputRecordCount    = 0u;
        static INPUT_RECORD *inputRecords        = NULL;

        // ...
        if (FALSE != ::GetNumberOfConsoleInputEvents(consoleInputHandle, &inputRecordCount))
        if (0u != inputRecordCount) {
          if (inputRecordCapacity < inputRecordCount) {
            void *const allocation = std::realloc(inputRecords == &inputRecord ? NULL : inputRecords, inputRecordCount * sizeof(INPUT_RECORD));

            if (NULL == allocation) { inputRecordCapacity = inputRecordCount = 1u; inputRecords = &inputRecord; }
            else { inputRecordCapacity = inputRecordCount; inputRecords = static_cast<INPUT_RECORD*>(allocation); }
          }

          if (FALSE != ::ReadConsoleInput(consoleInputHandle, inputRecords, inputRecordCount, &inputRecordCount)) {
            if (inputRecordCapacity < inputRecordCount)
            inputRecordCount = inputRecordCapacity;

            for (INPUT_RECORD const *inputRecord = inputRecords; inputRecord != inputRecords + inputRecordCount; ++inputRecord)
            if (KEY_EVENT == inputRecord -> EventType) {
              if (inputRecord -> Event.KeyEvent.bKeyDown)
              switch (inputRecord -> Event.KeyEvent.wVirtualKeyCode) {
                case VK_DOWN : input('S'); break;
                case VK_LEFT : input('A'); break;
                case VK_RIGHT: input('D'); break;
                case VK_UP   : input('W'); break;
                default      : input(static_cast<char>(std::toupper(static_cast<unsigned char>(inputRecord -> Event.KeyEvent.uChar.AsciiChar)))); break;
              }
            }
          }
        }

        // ...
        consoleCursorPosition.X = 0;
        consoleCursorPosition.Y = 0;

        if (FALSE != ::SetConsoleCursorPosition(consoleOutputHandle, consoleCursorPosition)) {
          DWORD &charactersWritten = inputRecordCount;

          // ...
          if (NULL == Game::DISPLAY) continue;
          draw();

          if (FALSE != ::WriteConsoleA(consoleOutputHandle, Game::DISPLAY, Game::DISPLAY_CAPACITY, &charactersWritten, static_cast<LPVOID>(NULL))) {
            rendered = true;
            continue;
          }
        }

        if (rendered) {
          rendered = false;
          std::fputs("[" TITLE "] Unable to render game graphics", stderr);
        }
      }

      // ...
      if (false == bufferRendering)
      bufferRendering = true;
    }
  #else
    std::fputs("[" TITLE "] Unable to render game graphics", stderr);
  #endif

  // ...
  return EXIT_SUCCESS;
}
