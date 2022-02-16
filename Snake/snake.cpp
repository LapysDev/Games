/* Import > ... */
#include <csignal>  // --> std::signal(…)
#include <cstddef>  // --> std::size_t
#include <cstdio>   // --> std::fclose(…), std::fgetc(…), std::fopen(…), std::fprintf(…), std::fputc(…), std::fputs(…), std::setbuf(…)
#include <cstdlib>  // --> std::calloc(…), std::free(…)
#include <ctime>    // --> std::time(…), std::time_t
#include <new>      // --> std::launder(…), new (…) T{…}
#include <stdint.h> // --> uint_least32_t

#if defined(__APPLE__) || defined(__gnu_linux__) || defined(linux) || defined(__linux) || defined(__linux__) || defined(__MACH__) || defined(__unix) || defined(__unix__)
# include <conio.h>
#elif defined(__NT__) || defined(__TOS_WIN__) || defined(_WIN16) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN32_WCE) || defined(_WIN64) || defined(__WINDOWS__)
# ifndef _WIN32_WINNT
#   define _WIN32_WINNT 0x0500
# endif
# include <windows.h>
#endif

/* Definition */
#define TITLE "Snake"

/* Namespace */
namespace Game {
  class Fruit;
  class Snake;
  class Object;
  class Wall;

  static void draw(void (*)(char[], char, unsigned));
  static void input(char const);
  template <typename type> inline static type* launder(type* const);
  static void quit();
  inline static void quit(int const);
  static std::size_t randint(std::size_t const);
  static std::size_t const (&randseed(std::size_t const = 0u))[4];
}

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

    // ...
    inline void draw(void (*const)(char[], char, unsigned), char const[], std::size_t const, std::size_t const, std::size_t const, std::size_t, unsigned const);
    inline std::size_t getHeight(char const[]);
    inline std::size_t getWidth(char const[]);
  }

  static char const *const ASSETS[]         = {Assets::BONUS_FRUIT, Assets::FRUIT, Assets::INVINCIBLE_FRUIT, Assets::SNAKE_EAST_TO_NORTH, Assets::SNAKE_EAST_TO_SOUTH, Assets::SNAKE_HEAD, Assets::SNAKE_NORTH_TO_EAST, Assets::SNAKE_NORTH_TO_WEST, Assets::SNAKE_SOUTH_TO_EAST, Assets::SNAKE_SOUTH_TO_WEST, Assets::SNAKE_TAIL, Assets::SNAKE_WEST_TO_NORTH, Assets::SNAKE_WEST_TO_SOUTH, Assets::WALL};
  static std::size_t       COLUMN_COUNT     = 48u;
  static std::size_t       CURRENT_SCORE    = 0u;
  static char             *DISPLAY          = NULL;
  static std::size_t       DISPLAY_CAPACITY = 0u;
  static std::size_t       FRUIT_CAPACITY   = 0u;
  static std::size_t       FRUIT_COUNT      = 0u;
  static Fruit            *FRUITS           = NULL;
  static std::size_t       HIGH_SCORE       = 0u;
  static std::size_t       ROW_COUNT        = 12u;
  static FILE             *SCORE_FILE       = NULL;
  static Snake  const     *SNAKE            = NULL;
  static std::size_t       WALL_COUNT       = 0u;
  static Wall             *WALLS            = NULL;

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

    "==================== ..."   "\r\n"
    "||" " SCORE: %u" "%a +%u%a" "\r\n"
    "||" " HIGH SCORE: %u"
    "%a" "\r\n" "||" " INVINCIBLE" "%a"
         "\r\n" "||" " [%u, %u] in [%u, %u]"
    "%a" "\r\n" "GAMEOVER !!" "\r\n\n" "  %S" "\r\n" "  Save your score? [Y]es / [N]o" "\r\n" "%a"
  };
}

/* Alias > ... */
using Game::Fruit;
using Game::Object;
using Game::Snake;
using Game::Wall;

/* Class > ... */
class Object {
  private:
    template <std::size_t (*accessor)(std::size_t), std::size_t (*mutator)(std::size_t, std::size_t)>
    struct coordinate_t {
      inline coordinate_t& operator =(std::size_t const coordinate) { this -> coordinates = (*mutator)(this -> coordinates, coordinate); return *this; }
      inline operator std::size_t() const { return (*accessor)(this -> coordinates); }

      private: std::size_t coordinates;
    };

    template <std::size_t (*assetSizeAccessor)(char const[])>
    struct dimension_t {
      inline operator std::size_t() const {
        static std::size_t size = 0u;

        if (0u == size)
        for (char const *const *asset = Game::ASSETS + (sizeof(Game::ASSETS) / sizeof(char const*)); Game::ASSETS != asset; ) {
          std::size_t const assetSize = (*assetSizeAccessor)(*--asset);
          size = assetSize > size ? assetSize : size;
        }

        return size;
      }
    };

    struct size_t {
      inline operator std::size_t() const {
        return Object::HEIGHT > Object::WIDTH ? Object::HEIGHT : Object::WIDTH;
      }
    };

    // ...
    inline static std::size_t getColumn(std::size_t const coordinates) { return coordinates % Game::COLUMN_COUNT; }
    inline static std::size_t getRow   (std::size_t const coordinates) { return coordinates / Game::COLUMN_COUNT; }
    inline static std::size_t setColumn(std::size_t const coordinates, std::size_t const column) { return column + (Game::COLUMN_COUNT * Object::getRow(coordinates)); }
    inline static std::size_t setRow   (std::size_t const coordinates, std::size_t const row)    { return Object::getColumn(coordinates) + (Game::COLUMN_COUNT * row); }

  public:
    enum Type { FRUIT = 0x1u, FRUIT_BONUS, FRUIT_INVINCIBLE, SNAKE, SNAKE_BODY, SNAKE_TAIL, WALL };

    static dimension_t<&Game::Assets::getHeight> const HEIGHT;
    static size_t                                const SIZE;
    static dimension_t<&Game::Assets::getWidth>  const WIDTH;

    // ... --- WARN (Lapys) -> Direct member function access is undefined behavior
    union {
      coordinate_t<&Object::getColumn, &Object::setColumn> column;
      coordinate_t<&Object::getRow,    &Object::setRow>    row;
    };
};
  Object::dimension_t<&Game::Assets::getHeight> const Object::HEIGHT = Object::dimension_t<&Game::Assets::getHeight>();
  Object::size_t                                const Object::SIZE   = Object::size_t();
  Object::dimension_t<&Game::Assets::getWidth>  const Object::WIDTH  = Object::dimension_t<&Game::Assets::getWidth>();

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

            private: mutable void *value;
            public: inline operator std::size_t() const { return NULL != this -> value ? *Game::launder(static_cast<std::size_t*>(this -> value)) : 0u; }
          } length;

          inline operator Tail*() const {
            if (NULL == this -> length.value)
            return NULL;

            for (unsigned char *value = static_cast<unsigned char*>(this -> length.value) + sizeof(std::size_t); ; ++value) {
              if (0u == static_cast<std::size_t>(value - static_cast<unsigned char*>(this -> length.value)) % sizeof(Tail))
              return Game::launder(reinterpret_cast<Tail*>(value));
            }
          }
      } tail;
  };

  // ...
  class Wall : public Object {};

/* Function */
void Game::Assets::draw(void (*const renderer)(char[], char, unsigned), char const asset[], std::size_t const x, std::size_t const y, std::size_t const width, std::size_t height, unsigned const type) {
  static std::size_t const boardWidth = Game::COLUMN_COUNT * Object::SIZE;

  // ...
  for (char *display = Game::DISPLAY + boardWidth + x + 5u + (y * boardWidth) + (y * 4u); height--; ++asset, display += (boardWidth - width) + 4u)
  for (std::size_t count = width; count; --count) (*renderer)(display++, *(asset++), type); // --> *(display++) = *(asset++);
}

// ... --- MINIFY (Lapys)
std::size_t Game::Assets::getHeight(char const asset[]) { for (std::size_t height = 0u; ; ) if ('\0' == *(asset++)) { ++height; if ('\0' == *asset) return height; } }
std::size_t Game::Assets::getWidth (char const asset[]) { for (std::size_t count = 0u, width = 0u; ; ) { if ('\0' == *(asset++)) { width = count > width ? count : width; count = 0u; if ('\0' == *asset) { return width; } continue; } ++count; } }

// ...
void Game::draw(void (*const renderer)(char[], char, unsigned)) {
  static bool wallsRendered = false;

  // ...
  if (false == wallsRendered) {
    std::size_t const height = Game::Assets::getHeight(Game::Assets::WALL);
    std::size_t const width  = Game::Assets::getWidth(Game::Assets::WALL);

    // ...
    wallsRendered = true;

    for (Wall const *wall = Game::WALLS; wall != Game::WALLS + Game::WALL_COUNT; ++wall)
    Game::Assets::draw(renderer, Game::Assets::WALL, Object::SIZE * Game::launder(wall) -> column, Object::SIZE * Game::launder(wall) -> row, width, height, Object::WALL);
  }
}

// ...
void Game::input(char const key) {
  // std::printf("[" TITLE "] '%c'" "\r\n", key);
  switch (key) {
    case 'A': break;
    case 'D': break;
    case 'S': break;
    case 'W': break;

    case 'Q': break;
  }
}

// ...
template <typename type>
type* Game::launder(type* const pointer) {
  #ifdef __cpp_lib_launder
    return std::launder(pointer);
  #else
    return pointer;
  #endif
}

// ...
void Game::quit() {
  std::free(Game::DISPLAY);
  std::free(Game::WALLS);
  if (NULL != Game::SCORE_FILE) std::fclose(Game::SCORE_FILE);

  // ...
  std::fputs("\r\n" "[" TITLE "] Program terminating", stderr);
  std::exit(EXIT_SUCCESS);
}

void Game::quit(int const signal) {
  switch (signal) {
    case SIGINT : std::fputs("\r\n" "[" TITLE "] Program interrupted",                stderr); break;
    case SIGSEGV: std::fputs("\r\n" "[" TITLE "] Program requested unpermitted data", stderr); break;
    default: break;
  }

  Game::quit();
}

// ...
std::size_t Game::randint(std::size_t const range) {
  std::size_t (&seeds)[4] = const_cast<std::size_t (&)[4]>(Game::randseed());
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

std::size_t const (&Game::randseed(std::size_t const reseed))[4] {
  static std::size_t seeds[4] = {0u, 0x2A430C43u, 0u, 0xDE83A17u}; // ->> pre-seeded to `0u`

  // ...
  for (std::size_t *seed = seeds + 4, state; 0u != reseed && seed != seeds; ) {
    state  = reseed + (0x9E3779B9u * ((seed - seeds) >> 1u));
    state ^= state >> 15u; state *= 0x85EBCA6Bu;
    state ^= state >> 13u; state *= 0xC2B2AE35u;
    state ^= state >> 16u;

    *--seed = state;
    *--seed = state >> sizeof(uint_least32_t);
  }

  return seeds;
}

/* Main */
int main(int, char* arguments[]) {
  std::time_t const time = std::time(static_cast<std::time_t*>(NULL));

  // ...
  std::atexit(static_cast<void (*)()>(&Game::quit));
  std::setbuf(stderr, NULL);
  std::signal(SIGINT,  static_cast<void (*)(int)>(&Game::quit));
  std::signal(SIGSEGV, static_cast<void (*)(int)>(&Game::quit));

  Game::randseed(time == static_cast<std::time_t>(-1) ? 0u : static_cast<std::size_t>(time));

  // ...
  if (NULL != arguments[1]) {
    if (NULL == arguments[2]) {
      std::fputs("[" TITLE "] Expected both width then height to be specified", stderr);
      return EXIT_SUCCESS;
    }

    for (std::size_t *const counts[] = {&Game::COLUMN_COUNT, &Game::ROW_COUNT}, *const *count = counts; count != counts + (sizeof(counts) / sizeof(std::size_t*)); ++count) {
      bool        invalid = false;
      std::size_t value   = 0u;

      // ...
      for (char *argument = *++arguments; '\0' != *argument; ++argument) {
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

        if (invalid || value > SIZE_MAX / 10u) {
          std::fprintf(stderr, invalid ? "[" TITLE "] Invalid %5.6s specified: `" : "[" TITLE "] The %5.6s specified is too large: `", &Game::COLUMN_COUNT == *count ? "width" : "height");

          for (argument = *arguments; '\0' != *argument; std::fputc(*argument++, stderr))
          switch (*argument) {
            case '\\': std::fputc('\\',  stderr); break;
            case '`' : std::fputs("\\`", stderr); break;
            case '\b': std::fputs("\\b", stderr); break;
            case '\f': std::fputs("\\f", stderr); break;
            case '\n': std::fputs("\\n", stderr); break;
            case '\r': std::fputs("\\r", stderr); break;
            case '\t': std::fputs("\\t", stderr); break;
            case '\v': std::fputs("\\v", stderr); break;
            case '\0':  case '\1':  case '\2':  case '\3':  case '\4':  case '\5':  case '\6':  case '\7':
            case '\16': case '\17': case '\20': case '\21': case '\22': case '\23': case '\24': case '\25': case '\26': case '\27': case '\30': case '\31': {
              value = 0u;

              for (unsigned char index = static_cast<unsigned char>(*argument); index; index /= 10u) value = (index % 10u) + (value * 10u);
              for (; value; value /= 10u) std::fputc("0123456789"[value % 10u], stderr);
            } break;
          }

          std::fprintf(stderr, invalid ? "` (expected a contiguous collection of digits between 0-9)" : "` (value should be smaller than `%lu%2s`)", static_cast<unsigned long>(SIZE_MAX), "zu");
          return EXIT_SUCCESS;
        }

        value = digit + (value * 10u);
      }

      **count = value;
    }
  }

  if (0u == Game::COLUMN_COUNT && 0u == Game::ROW_COUNT) {
    #if defined(__APPLE__) || defined(__gnu_linux__) || defined(linux) || defined(__linux) || defined(__linux__) || defined(__MACH__) || defined(__unix) || defined(__unix__)
      /* ... */
    #elif defined(__NT__) || defined(__TOS_WIN__) || defined(_WIN16) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN32_WCE) || defined(_WIN64) || defined(__WINDOWS__)
      HANDLE consoleHandle = INVALID_HANDLE_VALUE;

      // ...
      if (INVALID_HANDLE_VALUE == consoleHandle) consoleHandle = ::CreateFileW(L"CONOUT$", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, static_cast<LPSECURITY_ATTRIBUTES>(NULL), CREATE_NEW | OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, static_cast<HANDLE>(NULL));
      if (INVALID_HANDLE_VALUE == consoleHandle) consoleHandle = ::GetStdHandle(STD_OUTPUT_HANDLE);

      if (INVALID_HANDLE_VALUE != consoleHandle) {
        CONSOLE_FONT_INFO          consoleFontInformation;
        CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInformation;
        int                        systemHeight;
        int                        systemWidth;

        // ...
        if (FALSE != ::GetCurrentConsoleFont(consoleHandle, FALSE, &consoleFontInformation) && FALSE != ::GetConsoleScreenBufferInfo(consoleHandle, &consoleScreenBufferInformation)) {
          ::SetProcessDPIAware();
          systemHeight = ::GetSystemMetrics(SM_CYSCREEN);
          systemWidth  = ::GetSystemMetrics(SM_CXSCREEN);

          if (0 != systemHeight && 0 != systemWidth) {
            std::size_t const consoleHeight = ((consoleFontInformation.dwFontSize.Y * consoleScreenBufferInformation.dwSize.Y) * 7u) / 10u;
            std::size_t const consoleWidth  = ((consoleFontInformation.dwFontSize.X * consoleScreenBufferInformation.dwSize.X) * 7u) / 10u;

            // ...
            systemWidth  = (systemWidth  * 7u) / 10u;
            systemHeight = (systemHeight * 7u) / 10u;
            Game::ROW_COUNT    = (consoleHeight < static_cast<unsigned>(systemHeight) ? consoleHeight : systemHeight) / (Object::SIZE * consoleFontInformation.dwFontSize.Y);
            Game::COLUMN_COUNT = (consoleWidth  < static_cast<unsigned>(systemWidth)  ? consoleWidth  : systemWidth)  / (Object::SIZE * consoleFontInformation.dwFontSize.X);
          }
        }

        ::CloseHandle(consoleHandle);
      }
    #endif
  }

  // ...
  Game::DISPLAY_CAPACITY = (
    (Game::ROW_COUNT * Object::SIZE * 2u) +                              // ->> CRLF newlines
    (((Game::COLUMN_COUNT * Object::SIZE) + 2u) * 2u) +                  // ->> Bottom & top borders
    (((Game::ROW_COUNT    * Object::SIZE) + 2u) * 2u) +                  // ->> Left & right borders
    (Game::COLUMN_COUNT * Game::ROW_COUNT * Object::SIZE * Object::SIZE) // ->> Board
  );

  Game::DISPLAY    = static_cast<char*>(std::calloc(Game::DISPLAY_CAPACITY + 1u, sizeof(char)));
  Game::SCORE_FILE = std::fopen("scores.txt", "rb");
  Game::SNAKE      = &static_cast<Snake const&>(Snake());
  Game::WALL_COUNT = Game::randint(((Game::COLUMN_COUNT * Game::ROW_COUNT) * 1u) / 10u);
  Game::WALLS      = static_cast<Wall*>(0u == Game::WALL_COUNT ? NULL : std::malloc(Game::WALL_COUNT * sizeof(Wall)));

  static_cast<void>(Game::CURRENT_SCORE);
  static_cast<void>(Game::FRUIT_CAPACITY);
  static_cast<void>(Game::FRUIT_COUNT);
  static_cast<void>(Game::FRUITS);

  // [Display]
  if (NULL == Game::DISPLAY) {
    std::fputs("[" TITLE "] Unable to render game graphics", stderr);
    return EXIT_SUCCESS;
  }

  *(Game::DISPLAY += Game::DISPLAY_CAPACITY) = '\0';

  *--Game::DISPLAY = '\n';
  *--Game::DISPLAY = '\r';
  *--Game::DISPLAY = Game::Assets::CORNERS[3];
    for (std::size_t count = Game::COLUMN_COUNT * Object::SIZE; count; --count)
    *--Game::DISPLAY = Game::Assets::BORDER_BOTTOM;
  *--Game::DISPLAY = Game::Assets::CORNERS[2];

  for (std::size_t row = Game::ROW_COUNT * Object::SIZE; row; --row) {
    *--Game::DISPLAY = '\n';
    *--Game::DISPLAY = '\r';
    *--Game::DISPLAY = Game::Assets::BORDER_RIGHT;
      for (std::size_t column = Game::COLUMN_COUNT * Object::SIZE; column; --column)
      *--Game::DISPLAY = Game::randint(Game::COLUMN_COUNT * Game::ROW_COUNT) < ((Game::COLUMN_COUNT * Game::ROW_COUNT) * 5u) / 100u ? Game::Assets::DECORATIONS[Game::randint(sizeof(Game::Assets::DECORATIONS) / sizeof(char))] : Game::Assets::FLOOR;
    *--Game::DISPLAY = Game::Assets::BORDER_LEFT;
  }

  *--Game::DISPLAY = '\n';
  *--Game::DISPLAY = '\r';
  *--Game::DISPLAY = Game::Assets::CORNERS[1];
    for (std::size_t count = Game::COLUMN_COUNT * Object::SIZE; count; --count)
    *--Game::DISPLAY = Game::Assets::BORDER_TOP;
  *--Game::DISPLAY = Game::Assets::CORNERS[0];

  // [Score]
  if (NULL != Game::SCORE_FILE) {
    bool        invalid = false;
    bool        parsing = false;
    std::size_t score   = 0u;

    while (true) {
      int const character = std::fgetc(Game::SCORE_FILE);
      unsigned char digit;

      // ...
      if (EOF == character)
      break;

      switch (static_cast<char>(character)) {
        case '0': digit = 0u; parsing = true; break;
        case '1': digit = 1u; parsing = true; break;
        case '2': digit = 2u; parsing = true; break;
        case '3': digit = 3u; parsing = true; break;
        case '4': digit = 4u; parsing = true; break;
        case '5': digit = 5u; parsing = true; break;
        case '6': digit = 6u; parsing = true; break;
        case '7': digit = 7u; parsing = true; break;
        case '8': digit = 8u; parsing = true; break;
        case '9': digit = 9u; parsing = true; break;
        case ',': invalid = false == parsing; parsing = false; break;
        case ' ': invalid = parsing; break;
        default:  invalid = true;    break;
      }

      if (invalid || score > SIZE_MAX / 10u) {
        Game::HIGH_SCORE = 0u;
        std::fputs("[" TITLE "] Unable to retrieve high score, time for a new record then!", stderr);
        if (EOF != std::fclose(Game::SCORE_FILE)) Game::SCORE_FILE = std::fopen("scores.txt", "wb"); // ->> Empty file contents

        break;
      }

      if (parsing) {
        score = digit + (score * 10u);
        continue;
      }

      Game::HIGH_SCORE = Game::HIGH_SCORE > score ? Game::HIGH_SCORE : score;
      score = 0u;
    }

    std::fclose(Game::SCORE_FILE);
    Game::SCORE_FILE = NULL;
  }

  // [Walls]
  if (NULL == Game::WALLS)
  Game::WALL_COUNT = 0u;

  for (Wall *wall = Game::WALLS + Game::WALL_COUNT; Game::WALLS != wall; ) {
    Wall *const currentWall = new (--wall) Wall();

    for (bool recoordinate = true; recoordinate; ) {
      currentWall -> column = Game::randint(Game::COLUMN_COUNT);
      currentWall -> row    = Game::randint(Game::ROW_COUNT);
      recoordinate          = false;

      for (Wall const *recentWall = wall + 1; recentWall != Game::WALLS + Game::WALL_COUNT && false == recoordinate; ++recentWall)
      recoordinate = (wall -> column == Game::launder(recentWall) -> column) && (wall -> row == Game::launder(recentWall) -> row);
    }
  }

  // [Gameplay]
  #if defined(__APPLE__) || defined(__gnu_linux__) || defined(linux) || defined(__linux) || defined(__linux__) || defined(__MACH__) || defined(__unix) || defined(__unix__)
    /* ... */
  #elif defined(__NT__) || defined(__TOS_WIN__) || defined(_WIN16) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN32_WCE) || defined(_WIN64) || defined(__WINDOWS__)
    static COORD               consoleCursorPosition  = COORD();
    HANDLE                     consoleInputHandle     = NULL;
    static HANDLE              consoleOutputHandle    = NULL;
    CONSOLE_SCREEN_BUFFER_INFO consoleScreenBufferInformation;
    char const         *const *consoleTitle           = NULL;
    HWND                       consoleWindow          = NULL;
    bool                       pendingAttributes      = true;
    bool                       pendingBufferedRender  = false;
    static bool                pendingCompletedRender = false;
    static bool                pendingColoredRender   = false;
    bool                       pendingInput           = false;
    bool                       pendingOutput          = false;

    struct Graphics {
      inline static void render(char pixelDisplay[], char const pixel, unsigned const information) {
        if (pendingCompletedRender) {
          static std::size_t const boardWidth = Game::COLUMN_COUNT * Object::SIZE;

          // ...
          consoleCursorPosition.X = 0;
          consoleCursorPosition.Y = 0;

          for (char const *display = Game::DISPLAY + 5u + boardWidth; ; ++consoleCursorPosition.Y, display += 4u + boardWidth)
          if (display > pixelDisplay) {
            consoleCursorPosition.X  = (4u + boardWidth) - (display - pixelDisplay);
            consoleCursorPosition.Y -= 1;

            break;
          }

          if (FALSE != ::SetConsoleCursorPosition(consoleOutputHandle, consoleCursorPosition)) {
            DWORD charactersWritten;

            // ...
            switch (information) {
              case Object::FRUIT:            pendingColoredRender = FALSE == ::SetConsoleTextAttribute(consoleOutputHandle, FOREGROUND_INTENSITY | FOREGROUND_RED); break;
              case Object::FRUIT_BONUS:      pendingColoredRender = FALSE == ::SetConsoleTextAttribute(consoleOutputHandle, FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);  break;
              case Object::FRUIT_INVINCIBLE: pendingColoredRender = FALSE == ::SetConsoleTextAttribute(consoleOutputHandle, FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);  break;
              case Object::SNAKE:            pendingColoredRender = FALSE == ::SetConsoleTextAttribute(consoleOutputHandle, FOREGROUND_INTENSITY | FOREGROUND_GREEN);                                     break;
              case Object::SNAKE_BODY:       pendingColoredRender = FALSE == ::SetConsoleTextAttribute(consoleOutputHandle, FOREGROUND_GREEN);                                                            break;
              case Object::SNAKE_TAIL:       pendingColoredRender = FALSE == ::SetConsoleTextAttribute(consoleOutputHandle, FOREGROUND_BLUE | FOREGROUND_GREEN);                                          break;
              case Object::WALL:             pendingColoredRender = FALSE == ::SetConsoleTextAttribute(consoleOutputHandle, COMMON_LVB_UNDERSCORE | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED); break;
            }

            if (false == pendingColoredRender && FALSE != ::FillConsoleOutputCharacterA(consoleOutputHandle, pixel, 1u, consoleCursorPosition, &charactersWritten))
            return;
          }

          pendingColoredRender = true;
        }

        *pixelDisplay = pixel;
      }
    };

    // ...
    while (true) {
      if (INVALID_HANDLE_VALUE == consoleInputHandle  || NULL == consoleInputHandle)  consoleInputHandle  = ::CreateFileW(L"CONIN$", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, static_cast<LPSECURITY_ATTRIBUTES>(NULL), CREATE_NEW | OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, static_cast<HANDLE>(NULL));
      if (INVALID_HANDLE_VALUE == consoleOutputHandle || NULL == consoleOutputHandle) consoleOutputHandle = ::CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, static_cast<LPSECURITY_ATTRIBUTES>(NULL), CREATE_NEW | OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, static_cast<HANDLE>(NULL));

      if (INVALID_HANDLE_VALUE == consoleInputHandle)  consoleInputHandle  = ::GetStdHandle(STD_INPUT_HANDLE);
      if (INVALID_HANDLE_VALUE == consoleOutputHandle) consoleOutputHandle = ::GetStdHandle(STD_OUTPUT_HANDLE);

      // ...
      if (NULL == consoleTitle) {
        if (FALSE != ::SetConsoleTitle(TITLE))
        consoleTitle = &static_cast<char const* const&>(TITLE);
      }

      if (NULL == consoleTitle) {
        if (NULL == consoleWindow) consoleWindow = ::GetConsoleWindow();
        if (NULL != consoleWindow) { if (FALSE != ::SetWindowTextA(consoleWindow, TITLE)) consoleTitle = &static_cast<char const* const&>(TITLE); }
      }

      // ...
      if (pendingAttributes)
      pendingAttributes = FALSE == ::GetConsoleScreenBufferInfo(consoleOutputHandle, &consoleScreenBufferInformation);

      // ...
      if (INVALID_HANDLE_VALUE == consoleInputHandle  || NULL == consoleInputHandle)  { if (false == pendingInput)  { pendingInput  = true; std::fputs("[" TITLE "] Unable to parse player input",   stderr); } continue; }
      if (INVALID_HANDLE_VALUE == consoleOutputHandle || NULL == consoleOutputHandle) { if (false == pendingOutput) { pendingOutput = true; std::fputs("[" TITLE "] Unable to render game graphics", stderr); } continue; }
      if (pendingBufferedRender) ::WaitForSingleObject(consoleInputHandle, INFINITE);

      if (false == pendingCompletedRender || WAIT_OBJECT_0 == ::WaitForSingleObject(consoleInputHandle, 0u)) {
        static unsigned char inputRecord[sizeof(INPUT_RECORD)];
        static std::size_t   inputRecordCapacity = 0u;
        DWORD                inputRecordCount    = 0u;
        static INPUT_RECORD *inputRecords        = NULL;

        // ...
        if (FALSE != ::GetNumberOfConsoleInputEvents(consoleInputHandle, &inputRecordCount))
        if (0u != inputRecordCount) {
          if (inputRecordCapacity < inputRecordCount) {
            void *const allocation = std::realloc(inputRecords == reinterpret_cast<INPUT_RECORD*>(&inputRecord) ? NULL : inputRecords, inputRecordCount * sizeof(INPUT_RECORD));

            if (NULL == allocation) { inputRecordCapacity = inputRecordCount = 1u; inputRecords = reinterpret_cast<INPUT_RECORD*>(&inputRecord); }
            else { inputRecordCapacity = inputRecordCount; inputRecords = static_cast<INPUT_RECORD*>(allocation); }
          }

          if (FALSE != ::ReadConsoleInput(consoleInputHandle, inputRecords, inputRecordCount, &inputRecordCount)) {
            if (inputRecordCapacity < inputRecordCount)
            inputRecordCount = inputRecordCapacity;

            for (INPUT_RECORD const *inputRecord = inputRecords; inputRecord != inputRecords + inputRecordCount; ++inputRecord)
            if (KEY_EVENT == inputRecord -> EventType) {
              if (inputRecord -> Event.KeyEvent.bKeyDown)
              switch (inputRecord -> Event.KeyEvent.wVirtualKeyCode) {
                case VK_DOWN : Game::input('S'); break;
                case VK_LEFT : Game::input('A'); break;
                case VK_RIGHT: Game::input('D'); break;
                case VK_UP   : Game::input('W'); break;
                default: switch (inputRecord -> Event.KeyEvent.uChar.AsciiChar) {
                  case 'a': Game::input('A'); break;
                  case 'b': Game::input('B'); break;
                  case 'c': Game::input('C'); break;
                  case 'd': Game::input('D'); break;
                  case 'e': Game::input('E'); break;
                  case 'f': Game::input('F'); break;
                  case 'g': Game::input('G'); break;
                  case 'h': Game::input('H'); break;
                  case 'i': Game::input('I'); break;
                  case 'j': Game::input('J'); break;
                  case 'k': Game::input('K'); break;
                  case 'l': Game::input('L'); break;
                  case 'm': Game::input('M'); break;
                  case 'n': Game::input('N'); break;
                  case 'o': Game::input('O'); break;
                  case 'p': Game::input('P'); break;
                  case 'q': Game::input('Q'); break;
                  case 'r': Game::input('R'); break;
                  case 's': Game::input('S'); break;
                  case 't': Game::input('T'); break;
                  case 'u': Game::input('U'); break;
                  case 'v': Game::input('V'); break;
                  case 'w': Game::input('W'); break;
                  case 'x': Game::input('X'); break;
                  case 'y': Game::input('Y'); break;
                  case 'z': Game::input('Z'); break;
                  default : Game::input(inputRecord -> Event.KeyEvent.uChar.AsciiChar); break;
                }
              }
            }
          }
        }

        if (NULL == Game::DISPLAY)
        continue;

        pendingColoredRender = false;
        Game::draw(&Graphics::render);

        if (pendingColoredRender || false == pendingCompletedRender) {
          consoleCursorPosition.X = 0;
          consoleCursorPosition.Y = 0;

          if (FALSE != ::SetConsoleCursorPosition(consoleOutputHandle, consoleCursorPosition)) {
            DWORD &charactersWritten = inputRecordCount;

            // ...
            ::SetConsoleTextAttribute(consoleOutputHandle, pendingAttributes ? FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED : consoleScreenBufferInformation.wAttributes);
            (FALSE == ::WriteConsoleA(consoleOutputHandle, Game::DISPLAY, Game::DISPLAY_CAPACITY, &charactersWritten, static_cast<LPVOID>(NULL)) ? pendingBufferedRender : pendingCompletedRender) = true;
          }
        }

        if (pendingBufferedRender) {
          pendingBufferedRender = false;
          std::fputs("[" TITLE "] Unable to render game graphics", stderr);
        }
      }
    }
  #else
    std::fputs("[" TITLE "] Unable to render game graphics", stderr);
  #endif

  // ...
  return EXIT_SUCCESS;
}
