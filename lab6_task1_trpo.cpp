#include <iostream>
#include <string>

// Объявления всех классов и структур, которые будут использованы в программе.

struct Transformer;
struct Number;
struct BinaryOperation;
struct FunctionCall;
struct Variable;
struct Expression
{
    virtual ~Expression() { }
    virtual double evaluate() const = 0;
    virtual Expression* transform(Transformer* tr) const = 0;
};

// Интерфейс посетителя (Visitor).
struct Transformer
{
    virtual ~Transformer() { }
    // Методы для трансформации различных типов выражений.
    virtual Expression* transformNumber(Number const*) = 0;
    virtual Expression* transformBinaryOperation(BinaryOperation const*) = 0;
    virtual Expression* transformFunctionCall(FunctionCall const*) = 0;
    virtual Expression* transformVariable(Variable const*) = 0;
};

// Класс, представляющий числовое значение.
struct Number : Expression
{
    Number(double value) : value_(value) {}
    double value() const { return value_; }
    double evaluate() const { return value_; }
    Expression* transform(Transformer* tr) const { return tr->transformNumber(this); }
private:
    double value_;
};

// Класс, представляющий бинарную операцию (+, -, *, /).
struct BinaryOperation : Expression
{
    // Определим строковые константы для операций
    static constexpr char PLUS = '+';
    static constexpr char MINUS = '-';
    static constexpr char DIV = '/';
    static constexpr char MUL = '*';

    BinaryOperation(Expression const* left, int op, Expression const* right)
        : left_(left), op_(op), right_(right) {}
    ~BinaryOperation() { delete left_; delete right_; }
    double evaluate() const;
    Expression* transform(Transformer* tr) const { return tr->transformBinaryOperation(this); }
    Expression const* left() const { return left_; }
    Expression const* right() const { return right_; }
    char operation() const { return op_; }
private:
    Expression const* left_;
    Expression const* right_;
    int op_;
};
double BinaryOperation::evaluate() const {
    // Вычисляем левый и правый операнды
    double left_value = left_->evaluate();
    double right_value = right_->evaluate();

    // Выполняем операцию в зависимости от значения op_
    switch (op_) {
    case PLUS:
        return left_value + right_value;
    case MINUS:
        return left_value - right_value;
    case DIV:
        return left_value / right_value;
    case MUL:
        return left_value * right_value;
    default:
        // Если операция неизвестна, просто возвращаем ноль.
        return 0.0;
    }
}
// Класс, представляющий вызов функции.
struct FunctionCall : Expression
{
    FunctionCall(std::string const& name, Expression const* arg)
        : name_(name), arg_(arg) {}
    ~FunctionCall() { delete arg_; }
    double evaluate() const override;
    Expression* transform(Transformer* tr) const override { return tr->transformFunctionCall(this); }
    std::string const& name() const { return name_; }
    Expression const* arg() const { return arg_; }
private:
    std::string const name_;
    Expression const* arg_;
};
double FunctionCall::evaluate() const {
    // Вычисляем аргумент функции
    double arg_value = arg_->evaluate();

    // Применяем функцию к аргументу
    if (name_ == "sqrt") {
        return sqrt(arg_value);
    }
    else if (name_ == "abs") {
        return abs(arg_value);
    }
    else {
        // Если функция неизвестна, просто возвращаем ноль.
        return 0.0;
    }
}
// Класс, представляющий переменную.
struct Variable : Expression
{
    Variable(std::string const name, double value) : name_(name), value_(value) {} // Добавляем конструктор с указанием значения переменной
    std::string const& name() const { return name_; }
    double evaluate() const override { return value_; } // Возвращаем значение переменной
    Expression* transform(Transformer* tr) const override { return tr->transformVariable(this); }
private:
    std::string const name_;
    double value_; // Добавляем поле для хранения значения переменной
};

// Класс, реализующий копирование AST с использованием шаблона посетителя.
struct CopySyntaxTree : Transformer
{
    // Методы для трансформации различных типов выражений.

    // Копирование числового значения.
    Expression* transformNumber(Number const* number) override
    {
        return new Number(number->value());
    }

    // Копирование бинарной операции.
    Expression* transformBinaryOperation(BinaryOperation const* binop) override
    {
        Expression* left = binop->left()->transform(this); // Рекурсивно копируем левое поддерево.
        Expression* right = binop->right()->transform(this); // Рекурсивно копируем правое поддерево.
        return new BinaryOperation(left, binop->operation(), right); // Создаем новую бинарную операцию.
    }

    // Копирование вызова функции.
    Expression* transformFunctionCall(FunctionCall const* fcall) override
    {
        Expression* arg = fcall->arg()->transform(this); // Рекурсивно копируем аргумент функции.
        return new FunctionCall(fcall->name(), arg); // Создаем новый вызов функции.
    }

    // Копирование переменной.
    Expression* transformVariable(Variable const* var) override
    {
        return new Variable(var->name(), var->evaluate()); // Создаем новую переменную.
    }
};

int main()
{
    // Создание тестового AST.
// Создание объекта Number, представляющего число 32.0
    Number* n32 = new Number(32.0);

    // Создание объекта Number, представляющего число 16.0
    Number* n16 = new Number(16.0);

    // Создание объекта BinaryOperation, представляющего операцию вычитания
    // между числами n32 и n16
    BinaryOperation* minus = new BinaryOperation(n32, BinaryOperation::MINUS, n16);

    // Создание объекта FunctionCall, представляющего вызов функции sqrt
    // с аргументом minus (результатом операции вычитания)
    FunctionCall* callSqrt = new FunctionCall("sqrt", minus);

    // Создание объекта Variable, представляющего переменную с именем "var"
    // и значением 10.0
    Variable* var = new Variable("var", 10.0);

    // Создание объекта BinaryOperation, представляющего операцию умножения
    // между переменной var и результатом вызова функции sqrt
    BinaryOperation* mult = new BinaryOperation(var, BinaryOperation::MUL, callSqrt);

    // Создание объекта FunctionCall, представляющего вызов функции abs
    // с аргументом mult (результатом операции умножения)
    FunctionCall* callAbs = new FunctionCall("abs", mult);

    // Вычисляем значение выражения
    double result = callAbs->evaluate();
    std::cout << "Result: " << result << std::endl;
    // Создание экземпляра класса для копирования AST.
    CopySyntaxTree CST;

    //Создание копии AST. В результате выполнения создается копия,
    //представляющая тот же самый граф вычислений, что и оригинал, переменная newExpr будет указывать на корень этой копии.
    Expression* newExpr = callAbs->transform(&CST);

    // Вычисляем значение копии AST
    double newResult = newExpr->evaluate();
    std::cout << "New Result: " << newResult << std::endl;

    // Пример использования нового AST.

    // Освобождение памяти.
    delete callAbs;
    delete newExpr;

    return 0;
}
