# Lamina Example Tests

This directory contains official Lamina language examples that test various features of the interpreter.

## Test Files

1. **01_precise_math.lm** - Precise mathematical calculations with fractions and irrational numbers
2. **02_fraction.lm** - Conversion between fractions and decimals
3. **03_quadratic.lm** - Solving quadratic equations
4. **04_factorial.lm** - Big integer factorial calculations with single-line if statements
5. **05_vector.lm** - Vector and matrix operations
6. **06_singleline_syntax.lm** - Single-line if/else/while statements without braces
7. **07_semicolon_with_comments.lm** - Statements with inline comments after semicolons

## Running Tests

To run all tests:

```bash
for file in tests/lamina_examples/*.lm; do
    echo "=== Running $file ==="
    cargo run -- "$file"
    echo ""
done
```

Or run individual tests:

```bash
cargo run -- tests/lamina_examples/01_precise_math.lm
```

## Features Tested

### Precise Math
- Exact fraction arithmetic (16/9)
- Exact irrational number calculations (√2, √3)
- Mathematical constants (π, e)

### Fractions and Decimals
- Converting decimals to fractions
- Converting fractions to decimals
- Precise arithmetic vs floating point

### Advanced Features
- Quadratic equation solver
- Factorial with bigint
- Vector operations (addition, dot product, cross product, norm)
- Matrix operations (determinant)

### Language Syntax
- Single-line if statements: `if (condition) statement;`
- Single-line if-else: `if (condition) stmt; else stmt;`
- Single-line while loops: `while (condition) statement;`
- Inline comments after semicolons: `var x = 10; // comment`
