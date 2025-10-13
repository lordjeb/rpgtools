# RPG Tools

A C++ library and command-line tool for rolling dice and evaluating mathematical expressions commonly used in tabletop role-playing games.

## Features

### Dice Rolling
- **Standard dice notation**: `1d6`, `2d8`, `3d20`, etc.
- **Exploding dice**: `d6!` - dice that keep rolling when they hit maximum value
- **Advantage/Disadvantage**: `2d20b1` (keep best), `2d20w1` (keep worst)
- **Keep best/worst**: `4d6b3` (roll 4d6, keep best 3)
- **Special dice**: `d66` (Year Zero style percentile), `d666` (triple digit rolls)

### Mathematical Expressions
- **Basic arithmetic**: Addition (`+`), subtraction (`-`), multiplication (`*`)
- **Order of operations**: Proper PEMDAS evaluation
- **Parentheses**: Group operations with `(` and `)`
- **Mixed expressions**: Combine dice rolls with math, e.g., `1d8+3` or `2d6*2`

### Detailed Output
- Shows individual dice results: `2d6` → `(4, 3) = 7`
- Displays exploding dice chains: `d6!` → `([6+3]) = 9`
- Indicates dropped dice in advantage/disadvantage rolls

## Usage

### Command Line Tool

```bash
# Simple dice rolls
roll.exe 1d6
roll.exe 2d8+3

# Advantage/disadvantage (D&D 5e style)
roll.exe 2d20b1+5    # Advantage: roll 2d20, keep best, add 5
roll.exe 2d20w1+5    # Disadvantage: roll 2d20, keep worst, add 5

# Character generation (roll 4d6, keep best 3)
roll.exe 4d6b3

# Exploding dice
roll.exe 3d6!        # Each die that rolls max value explodes

# Year Zero Engine style rolls
roll.exe d66         # Roll 2d6, combine as tens and ones (11-66)
roll.exe d666        # Roll 3d6, combine as hundreds, tens, ones

# Complex expressions
roll.exe "(2d6+3)*2"
roll.exe "1d20+1d4+2"

# Multiple rolls at once
roll.exe 1d20+5 2d6+1 4d6b3
```

### Example Output

```
> roll.exe 2d20b1+5
2d20b1+5: (17, 3) = 22

> roll.exe 3d6!
3d6!: (4, [6+2], 5) = 17

> roll.exe 4d6b3
4d6b3: (5, 4, 6, 2) = 15
```

## Library Usage

```cpp
#include "rpgtools/expression_evaluator.h"
#include "rpgtools/random_number_generator.h"

// Create a random number generator and expression evaluator
random_number_generator rng;
expression_evaluator evaluator(&rng);

// Evaluate expressions
std::string description;
int result = evaluator.evaluate("2d6+3", &description);

std::cout << "Result: " << result << std::endl;
std::cout << "Rolls: " << description << std::endl;
```

## Building

This project uses Visual Studio 2022 and vcpkg for dependency management.

### Prerequisites
- Visual Studio 2022 with C++ development tools
- vcpkg package manager
- CMake (if building with CMake)

### Dependencies
- Google Test (for unit tests)

### Build Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/lordjeb/rpgtools.git
   cd rpgtools
   ```

2. Install dependencies via vcpkg:
   ```bash
   vcpkg install
   ```

3. Build with Visual Studio:
   - Open `rpgtools.sln` in Visual Studio
   - Build the solution (Ctrl+Shift+B)

4. Or build with MSBuild:
   ```bash
   msbuild rpgtools.sln /p:Configuration=Release /p:Platform=x64
   ```

## Project Structure

```
rpgtools/
├── src/
│   ├── rpgtools/           # Core library
│   │   ├── expression_evaluator.cpp/h    # Expression parsing and evaluation
│   │   ├── random_number_generator.cpp/h # RNG abstraction
│   │   └── rpgtools.cpp                  # Library main
│   └── roll/               # Command-line tool
│       └── roll.cpp        # CLI application
├── tst/
│   └── rpgtools_tests/     # Unit tests
└── vcpkg.json             # Package dependencies
```

## Running Tests

Tests are built automatically with the solution. Run them in Visual Studio Test Explorer or via command line:

```bash
# Run tests from the build output directory
.\x64\Debug\rpgtools_tests.exe
```

## Dice Notation Reference

| Notation | Description | Example |
|----------|-------------|---------|
| `XdY` | Roll X dice with Y sides | `3d6` = roll 3 six-sided dice |
| `dY` | Roll 1 die with Y sides | `d20` = roll 1 twenty-sided die |
| `XdY!` | Exploding dice | `2d6!` = exploding six-sided dice |
| `XdYbZ` | Keep best Z of X dice | `4d6b3` = roll 4d6, keep best 3 |
| `XdYwZ` | Keep worst Z of X dice | `4d6w1` = roll 4d6, keep worst 1 |
| `d66` | Special 2d6 roll (11-66) | Year Zero Engine d66 |
| `d666` | Special 3d6 roll (111-666) | Extended Year Zero d666 |

## Supported Operations

- `+` Addition
- `-` Subtraction  
- `*` Multiplication
- `()` Parentheses for grouping

## Future Enhancements

- Division with rounding (Savage Worlds raises)
- Success counting (count dice above target number)
- Different dice pools (attribute + skill + stress dice)
- Alias system for common rolls
- JSON configuration for custom roll aliases
- Target number evaluation

## Contributing

This is a personal project, but contributions are welcome! Please feel free to submit issues and pull requests.

## License

This project is open source. See the repository for license details.

## Version History

- **0.2.0** - Current version with exploding dice and advantage/disadvantage support
- **0.1.0** - Initial release with basic dice rolling and expression evaluation