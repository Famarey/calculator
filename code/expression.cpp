#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStack>
#include <QStringList>

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

    for(const QString &tk : tokens) {
        if(tk == "(") {
            ops.push(tk);
        } else if(tk == ")") {
            while(!ops.isEmpty() && ops.top() != "(") {
                QString op = ops.pop();
                long long b = values.pop();
                long long a = values.pop();
                values.push(applyOp(op, a, b));
            }
            if(!ops.isEmpty()) ops.pop();
        } else if(getPrecedence(tk) >= -1) {
            while(!ops.isEmpty() && getPrecedence(ops.top()) >= getPrecedence(tk)) {
                QString op = ops.pop();
                long long b = values.pop();
                long long a = values.pop();
                values.push(applyOp(op, a, b));
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
        if(values.size() < 2) break;
        long long b = values.pop();
        long long a = values.pop();
        values.push(applyOp(op, a, b));
    }

    return values.isEmpty() ? 0 : values.top();
}
