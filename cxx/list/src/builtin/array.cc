#include <builtin/array.h>

#include <cmath>
#include <numeric>
#include <algorithm>

namespace rumina {
namespace builtin {
namespace array {

Value foreach(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("foreach expects 2 arguments (array, function)");
    }
    
    throw std::runtime_error("foreach not yet fully implemented - use in interpreter");
}

Value map(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("map expects 2 arguments (array, function)");
    }
    
    throw std::runtime_error("map not yet implemented");
}

Value filter(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("filter expects 2 arguments (array, function)");
    }
    
    throw std::runtime_error("filter implemented in interpreter");
}

Value reduce(const std::vector<Value>& args) {
    if (args.size() < 2 || args.size() > 3) {
        throw std::runtime_error("reduce expects 2 or 3 arguments (array, function, [initial])");
    }
    
    if (args[0].getType() != Value::Type::Array) {
        throw std::runtime_error("reduce expects array, got " + args[0].typeName());
    }
    
    return Value();
}

Value push(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("push expects 2 arguments (array, value)");
    }
    
    if (args[0].getType() != Value::Type::Array) {
        throw std::runtime_error("push expects array, got " + args[0].typeName());
    }
    
    args[0].getArray()->push_back(args[1]);
    return Value();
}

Value pop(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("pop expects 1 argument (array)");
    }
    
    if (args[0].getType() != Value::Type::Array) {
        throw std::runtime_error("pop expects array, got " + args[0].typeName());
    }
    
    auto arr = args[0].getArray();
    if (arr->empty()) {
        throw std::runtime_error("Cannot pop from empty array");
    }
    
    Value result = arr->back();
    arr->pop_back();
    return result;
}

Value range(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("range expects 1 argument (length)");
    }
    
    if (args[0].getType() != Value::Type::Int) {
        throw std::runtime_error("range expects integer, got " + args[0].typeName());
    }
    
    int64_t n = args[0].getInt();
    if (n < 0) {
        throw std::runtime_error("range expects non-negative integer");
    }
    
    std::vector<Value> result;
    result.reserve(static_cast<size_t>(n));
    for (int64_t i = 0; i < n; ++i) {
        result.push_back(Value(i));
    }
    
    return Value::makeArray(std::make_shared<std::vector<Value>>(std::move(result)));
}

Value concat(const std::vector<Value>& args) {
    if (args.empty()) {
        return Value::makeArray(std::make_shared<std::vector<Value>>());
    }
    
    std::vector<Value> result;
    
    for (const auto& arg : args) {
        if (arg.getType() != Value::Type::Array) {
            throw std::runtime_error("concat expects only arrays, got " + arg.typeName());
        }
        
        const auto& arr = *arg.getArray();
        result.insert(result.end(), arr.begin(), arr.end());
    }
    
    return Value::makeArray(std::make_shared<std::vector<Value>>(std::move(result)));
}

Value dot(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("dot expects 2 arguments (vector1, vector2)");
    }
    
    if (args[0].getType() != Value::Type::Array || args[1].getType() != Value::Type::Array) {
        throw std::runtime_error("dot expects two arrays");
    }
    
    const auto& v1 = *args[0].getArray();
    const auto& v2 = *args[1].getArray();
    
    if (v1.size() != v2.size()) {
        throw std::runtime_error("Vectors must have same length");
    }
    
    double result = 0.0;
    for (size_t i = 0; i < v1.size(); ++i) {
        double a = v1[i].toFloat();
        double b = v2[i].toFloat();
        result += a * b;
    }
    
    return Value(result);
}

Value norm(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("norm expects 1 argument (vector)");
    }
    
    if (args[0].getType() != Value::Type::Array) {
        throw std::runtime_error("norm expects array");
    }
    
    const auto& v = *args[0].getArray();
    double sum = 0.0;
    
    for (const auto& val : v) {
        double f = val.toFloat();
        sum += f * f;
    }
    
    return Value(std::sqrt(sum));
}

Value cross(const std::vector<Value>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("cross expects 2 arguments (vector1, vector2)");
    }
    
    if (args[0].getType() != Value::Type::Array || args[1].getType() != Value::Type::Array) {
        throw std::runtime_error("cross expects two arrays");
    }
    
    const auto& v1 = *args[0].getArray();
    const auto& v2 = *args[1].getArray();
    
    if (v1.size() != 3 || v2.size() != 3) {
        throw std::runtime_error("cross expects 3D vectors");
    }
    
    double x1 = v1[0].toFloat();
    double y1 = v1[1].toFloat();
    double z1 = v1[2].toFloat();
    
    double x2 = v2[0].toFloat();
    double y2 = v2[1].toFloat();
    double z2 = v2[2].toFloat();
    
    std::vector<Value> result;
    result.reserve(3);
    result.push_back(Value(y1 * z2 - z1 * y2));
    result.push_back(Value(z1 * x2 - x1 * z2));
    result.push_back(Value(x1 * y2 - y1 * x2));
    
    return Value::makeArray(std::make_shared<std::vector<Value>>(std::move(result)));
}

double calculateDeterminant(const std::vector<std::vector<double>>& matrix) {
    size_t n = matrix.size();
    
    if (n == 1) {
        return matrix[0][0];
    }
    
    if (n == 2) {
        return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
    }
    
    double det = 0.0;
    for (size_t col = 0; col < n; ++col) {
        std::vector<std::vector<double>> submatrix;
        submatrix.reserve(n - 1);
        
        for (size_t i = 1; i < n; ++i) {
            std::vector<double> row;
            row.reserve(n - 1);
            for (size_t j = 0; j < n; ++j) {
                if (j != col) {
                    row.push_back(matrix[i][j]);
                }
            }
            submatrix.push_back(std::move(row));
        }
        
        double sign = (col % 2 == 0) ? 1.0 : -1.0;
        det += sign * matrix[0][col] * calculateDeterminant(submatrix);
    }
    
    return det;
}

Value det(const std::vector<Value>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("det expects 1 argument (matrix)");
    }
    
    if (args[0].getType() != Value::Type::Array) {
        throw std::runtime_error("det expects array, got " + args[0].typeName());
    }
    
    const auto& matrix_val = *args[0].getArray();
    size_t n = matrix_val.size();
    
    if (n == 0) {
        throw std::runtime_error("Cannot compute determinant of empty matrix");
    }
    
    std::vector<std::vector<double>> matrix;
    matrix.reserve(n);
    
    for (const auto& row_val : matrix_val) {
        if (row_val.getType() != Value::Type::Array) {
            throw std::runtime_error("det expects a matrix (2D array)");
        }
        
        const auto& row = *row_val.getArray();
        if (row.size() != n) {
            throw std::runtime_error("Matrix must be square");
        }
        
        std::vector<double> float_row;
        float_row.reserve(n);
        for (const auto& elem : row) {
            float_row.push_back(elem.toFloat());
        }
        matrix.push_back(std::move(float_row));
    }
    
    double result = calculateDeterminant(matrix);
    return Value(result);
}

} // namespace array
} // namespace builtin
} // namespace rumina
