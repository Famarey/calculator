#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStack>
#include <QStringList>
#include <QRegularExpression>

// -------------------------------
// 表达式计算逻辑 (中缀转后缀计算)
// -------------------------------
int getPrecedence(const QString &op)
{
    if(op == "~") return 5;
    if(op == "<<" || op == ">>") return 4;
    if(op == "*" || op == "/" || op == "%") return 3;
    if(op == "+" || op == "-") return 2;
    if(op == "&") return 1;
    if(op == "^") return 0;
    if(op == "|") return -1;
    return -2;
}

bool MainWindow::validateExpression(const QString &expr, Base base, QString &errorMsg)
{
    if (expr.isEmpty()) {
        errorMsg = "表达式为空";
        return false;
    }
    
    QString cleanExpr = expr;
    cleanExpr.remove(QRegularExpression("\\s+")); // 移除所有空格
    
    if (cleanExpr.isEmpty()) {
        errorMsg = "表达式为空";
        return false;
    }
    
    // 检查括号匹配
    int parenCount = 0;
    for (int i = 0; i < cleanExpr.length(); i++) {
        if (cleanExpr[i] == '(') parenCount++;
        else if (cleanExpr[i] == ')') {
            parenCount--;
            if (parenCount < 0) {
                errorMsg = "括号不匹配：右括号过多";
                return false;
            }
        }
    }
    if (parenCount > 0) {
        errorMsg = "括号不匹配：左括号过多";
        return false;
    }
    
    // 检查是否包含非法字符
    QRegularExpression validChars;
    switch (base) {
        case BIN:
            validChars = QRegularExpression("^[01+\\-*/%&|^~()<>]+$");
            break;
        case OCT:
            validChars = QRegularExpression("^[0-7+\\-*/%&|^~()<>]+$");
            break;
        case DEC:
            validChars = QRegularExpression("^[0-9+\\-*/%&|^~()<>]+$");
            break;
        case HEX:
            validChars = QRegularExpression("^[0-9A-Fa-f+\\-*/%&|^~()<>]+$");
            break;
    }
    
    if (!validChars.match(cleanExpr).hasMatch()) {
        errorMsg = "表达式包含非法字符";
        return false;
    }
    
    // 检查运算符的合理性
    // 不能以双目运算符开头（除了-和~）
    QString firstChar = cleanExpr.left(1);
    if (firstChar == "+" || firstChar == "*" || firstChar == "/" || 
        firstChar == "%" || firstChar == "&" || firstChar == "|" || 
        firstChar == "^" || firstChar == ">" || firstChar == "<") {
        errorMsg = "表达式不能以运算符开头";
        return false;
    }
    
    // 检查连续的运算符（除了~和-）
    for (int i = 0; i < cleanExpr.length() - 1; i++) {
        QChar c1 = cleanExpr[i];
        QChar c2 = cleanExpr[i + 1];
        
        // 处理双字符运算符
        if (i < cleanExpr.length() - 1) {
            QString twoChar = cleanExpr.mid(i, 2);
            if (twoChar == "<<" || twoChar == ">>") {
                i++; // 跳过第二个字符
                continue;
            }
        }
        
        // 检查连续运算符
        if ((c1 == '+' || c1 == '*' || c1 == '/' || c1 == '%' || 
             c1 == '&' || c1 == '|' || c1 == '^' || c1 == '<' || c1 == '>') &&
            (c2 == '+' || c2 == '*' || c2 == '/' || c2 == '%' || 
             c2 == '&' || c2 == '|' || c2 == '^' || c2 == '<' || c2 == '>')) {
            errorMsg = "表达式包含连续的运算符";
            return false;
        }
        // 检查\0
        if (c1 == '/'  &&c2 == '0') {
            errorMsg = "除数不得为0";
            return false;
        }
    }
    
    // 检查是否以运算符结尾
    QString lastChar = cleanExpr.right(1);
    if (lastChar == "+" || lastChar == "-" || lastChar == "*" || 
        lastChar == "/" || lastChar == "%" || lastChar == "&" || 
        lastChar == "|" || lastChar == "^" || lastChar == "~") {
        errorMsg = "表达式不能以运算符结尾";
        return false;
    }
    
    return true;
}

long long MainWindow::evaluateExpression(const QString &expr, Base base)
{
    QStringList tokens;
    QString tempToken;

    // 简单的词法分析
    for(int i = 0; i < expr.length(); ++i) {
        QChar c = expr[i];
        if(c.isSpace()) continue;

        // 根据当前进制判断是否为数字字符
        bool isDigit = c.isDigit() || (base == HEX && ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')));

        if(isDigit) {
            tempToken.append(c);
        } else {
            if(!tempToken.isEmpty()) { tokens.append(tempToken); tempToken.clear(); }
            // 处理双字符操作符 << >>
            if(i + 1 < expr.length() && ((c == '<' && expr[i+1] == '<') || (c == '>' && expr[i+1] == '>'))) {
                tokens.append(expr.mid(i, 2));
                i++;
            } else {
                tokens.append(QString(c));
            }
        }
    }
    if(!tempToken.isEmpty()) tokens.append(tempToken);

    QStack<long long> values;
    QStack<QString> ops;

    auto applyOp = [&](const QString &op, long long a, long long b) -> long long {
        if(op == "+") return a + b;
        if(op == "-") return a - b;
        if(op == "*") return a * b;
        if(op == "/") return b != 0 ? a / b : 0;
        if(op == "%") return b != 0 ? a % b : 0;
        if(op == "&") return a & b;
        if(op == "|") return a | b;
        if(op == "^") return a ^ b;
        if(op == "<<") return a << b;
        if(op == ">>") return a >> b;
        return 0;
    };
    
    auto applyUnaryOp = [&](const QString &op, long long a) -> long long {
        if(op == "~") return ~a;
        if(op == "-") return -a;
        return a;
    };

    for(int i = 0; i < tokens.size(); i++) {
        const QString &tk = tokens[i];
        if(tk == "(") {
            ops.push(tk);
        } else if(tk == ")") {
            while(!ops.isEmpty() && ops.top() != "(") {
                QString op = ops.pop();
                if(op == "~" || op == "-") {
                    // 可能是单目运算符，检查栈中值的数量
                    if(values.size() == 1 || (i > 0 && tokens[i-1] == "(")) {
                        // 单目运算符
                        if(values.isEmpty()) break;
                        long long a = values.pop();
                        values.push(applyUnaryOp(op, a));
                    } else {
                        // 双目运算符
                        if(values.size() < 2) break;
                        long long b = values.pop();
                        long long a = values.pop();
                        values.push(applyOp(op, a, b));
                    }
                } else {
                    // 双目运算符
                    if(values.size() < 2) break;
                    long long b = values.pop();
                    long long a = values.pop();
                    values.push(applyOp(op, a, b));
                }
            }
            if(!ops.isEmpty()) ops.pop();
        } else if(tk == "~" || (tk == "-" && (i == 0 || (i > 0 && (tokens[i-1] == "(" || getPrecedence(tokens[i-1]) >= -1))))) {
            // 单目运算符：~ 或者开头的 - 或者 ( 后的 - 或者运算符后的 -
            ops.push(tk);
        } else if(getPrecedence(tk) >= -1) {
            while(!ops.isEmpty() && ops.top() != "(" && getPrecedence(ops.top()) >= getPrecedence(tk)) {
                QString op = ops.pop();
                if(op == "~") {
                    // 单目运算符
                    if(values.isEmpty()) break;
                    long long a = values.pop();
                    values.push(applyUnaryOp(op, a));
                } else if(op == "-" && values.size() == 1) {
                    // 可能是单目运算符
                    long long a = values.pop();
                    values.push(applyUnaryOp(op, a));
                } else {
                    // 双目运算符
                    if(values.size() < 2) break;
                    long long b = values.pop();
                    long long a = values.pop();
                    values.push(applyOp(op, a, b));
                }
            }
            ops.push(tk);
        } else {
            bool ok;
            long long v = tk.toLongLong(&ok, base);
            values.push(ok ? v : 0);
        }
    }

    while(!ops.isEmpty()) {
        QString op = ops.pop();
        if(op == "~") {
            // 单目运算符
            if(values.isEmpty()) break;
            long long a = values.pop();
            values.push(applyUnaryOp(op, a));
        } else if(op == "-" && values.size() == 1) {
            // 可能是单目运算符
            long long a = values.pop();
            values.push(applyUnaryOp(op, a));
        } else {
            // 双目运算符
            if(values.size() < 2) break;
            long long b = values.pop();
            long long a = values.pop();
            values.push(applyOp(op, a, b));
        }
    }

    return values.isEmpty() ? 0 : values.top();
}
