#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QResizeEvent>
#include <QLabel>
#include <QCheckBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentBase(DEC) // 默认十进制
    , lastFocusedEdit(nullptr)
    , isUpdating(false)
    , lastUpdateMode(0) // 默认仅更新数值
{
    ui->setupUi(this);

    // 1. 初始化数字按钮映射
    digitButtons = {
        {"0", ui->btn0}, {"1", ui->btn1}, {"2", ui->btn2}, {"3", ui->btn3},
        {"4", ui->btn4}, {"5", ui->btn5}, {"6", ui->btn6}, {"7", ui->btn7},
        {"8", ui->btn8}, {"9", ui->btn9},
        {"A", ui->btnA}, {"B", ui->btnB}, {"C", ui->btnC},
        {"D", ui->btnD}, {"E", ui->btnE}, {"F", ui->btnF}
    };

    // 2. 初始化运算符按钮映射
    // 注意：btnAnd按钮显示的是"&&"，但实际插入的应该是"&"
    operatorButtons = {
        {"+", ui->btnAdd}, {"-", ui->btnSub}, {"*", ui->btnMul}, {"/", ui->btnDiv},
        {"%", ui->btnMod}, {"&", ui->btnAnd}, {"|", ui->btnOr}, {"^", ui->btnXor},
        {"~", ui->btnNot}, {"(", ui->btnLParen}, {")", ui->btnRParen},
        {"<<", ui->btnShl}, {">>", ui->btnShr}
    };

    // 3. 设置输入校验，禁止非法键盘输入
    // 表达式：允许 0-9 A-F a-f、空格和常用运算符
    ui->editExpression->setValidator(new QRegularExpressionValidator(
                                         QRegularExpression("[0-9A-Fa-f\\s\\+\\-\\*/%&|^~()<>]*"), this));

    // HEX: 0-9 A-F a-f
    ui->editHex->setValidator(new QRegularExpressionValidator(
                                  QRegularExpression("[0-9A-Fa-f]*"), this));
    // DEC: 可选负号 + 数字
    ui->editDec->setValidator(new QRegularExpressionValidator(
                                  QRegularExpression("-?[0-9]*"), this));
    // OCT: 0-7
    ui->editOct->setValidator(new QRegularExpressionValidator(
                                  QRegularExpression("[0-7]*"), this));
    // BIN: 0/1
    ui->editBin->setValidator(new QRegularExpressionValidator(
                                  QRegularExpression("[01]*"), this));
    // BIN 结果: 0/1 和分段分隔符 |
    ui->editBinResult->setValidator(new QRegularExpressionValidator(
                                        QRegularExpression("[01|]*"), this));
    // DEC 结果: 数字和分隔符 |
    ui->editDecResult->setValidator(new QRegularExpressionValidator(
                                        QRegularExpression("[0-9|]*"), this));
    // HEX 结果: 0-9 A-F a-f 和分隔符 |
    ui->editHexResult->setValidator(new QRegularExpressionValidator(
                                        QRegularExpression("[0-9A-Fa-f|]*"), this));
    // 分割规则: 数字和逗号
    ui->editSplitRule->setValidator(new QRegularExpressionValidator(
                                        QRegularExpression("[0-9,]*"), this));

    // 3. 信号槽连接
    for(auto btn : digitButtons.values())
        connect(btn, &QPushButton::clicked, this, &MainWindow::onDigitButtonClicked);

    for(auto btn : operatorButtons.values())
        connect(btn, &QPushButton::clicked, this, &MainWindow::onOperatorButtonClicked);

    connect(ui->btnBackspace, &QPushButton::clicked, this, &MainWindow::onBackspaceClicked);
    connect(ui->btnEq, &QPushButton::clicked, this, &MainWindow::onEqualClicked);
    connect(ui->btnClear, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(ui->btnReset, &QPushButton::clicked, this, &MainWindow::onResetClicked);
    connect(ui->btnComma, &QPushButton::clicked, this, &MainWindow::onBitOperatorClicked);

    // 4. 安装事件过滤器 (核心修改点：涵盖所有相关输入框)
    QList<QLineEdit*> editBoxes = {
        ui->editHex, ui->editHexResult,
        ui->editDec, ui->editDecResult,
        ui->editOct,
        ui->editBin, ui->editBinResult
    };

    for(QLineEdit* edit : editBoxes) {
        edit->installEventFilter(this);
    }

    // 5. 实时分割转换
    connect(ui->editSplitRule, &QLineEdit::textChanged, this, &MainWindow::onSplitRuleChanged);

    // 6. 连接输入框的文本变化信号
    connect(ui->editHex, &QLineEdit::textChanged, this, &MainWindow::onHexInputChanged);
    connect(ui->editDec, &QLineEdit::textChanged, this, &MainWindow::onDecInputChanged);
    connect(ui->editOct, &QLineEdit::textChanged, this, &MainWindow::onOctInputChanged);
    connect(ui->editBin, &QLineEdit::textChanged, this, &MainWindow::onBinInputChanged);

    // 7. 连接结果框的文本变化信号
    connect(ui->editBinResult, &QLineEdit::textChanged, this, &MainWindow::onBinResultChanged);
    connect(ui->editDecResult, &QLineEdit::textChanged, this, &MainWindow::onDecResultChanged);
    connect(ui->editHexResult, &QLineEdit::textChanged, this, &MainWindow::onHexResultChanged);

    // 8. 连接复选框的值变化信号（替代原滑块）
    connect(ui->chkSyncExpression, &QCheckBox::stateChanged,
            this, &MainWindow::onUpdateModeChanged);

    // 初始化状态
    ui->editSplitRule->installEventFilter(this);
    setButtonEnabledByBase(currentBase);
    ui->chkSyncExpression->setChecked(false); // 默认仅更新数值
}

MainWindow::~MainWindow()
{
    delete ui;
}

// -------------------------------
// 窗口大小变化时自适应字体和按钮高度
// -------------------------------
void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    // 以设计时高度 600 作为基准，按比例缩放
    const int baseHeight = 600;
    const int currentH = height();
    double scale = static_cast<double>(currentH) / baseHeight;
    if (scale < 0.7) scale = 0.7;
    if (scale > 1.6) scale = 1.6;

    // 缩放字体大小
    const int basePointSize = 10;
    int newPointSize = static_cast<int>(basePointSize * scale);
    if (newPointSize < 8) newPointSize = 8;
    if (newPointSize > 18) newPointSize = 18;

    QFont f = this->font();
    f.setPointSize(newPointSize);

    // 应用到所有 QLineEdit
    const auto lineEdits = findChildren<QLineEdit*>();
    for (QLineEdit *edit : lineEdits) {
        edit->setFont(f);
    }

    // 应用到左侧标签和“同步表达式”复选框
    const auto labels = findChildren<QLabel*>();
    for (QLabel *lab : labels) {
        lab->setFont(f);
    }
    const auto checks = findChildren<QCheckBox*>();
    for (QCheckBox *chk : checks) {
        chk->setFont(f);
    }

    // 应用到所有 QPushButton，并按比例设置最小高度
    const int baseBtnMinH = 26;
    int newBtnMinH = static_cast<int>(baseBtnMinH * scale);
    if (newBtnMinH < 20) newBtnMinH = 20;

    const auto buttons = findChildren<QPushButton*>();
    for (QPushButton *btn : buttons) {
        btn->setFont(f);
        btn->setMinimumHeight(newBtnMinH);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
}

// -------------------------------
// 二进制分割结果编辑辅助函数
// -------------------------------
int MainWindow::findLeftDigitPos(const QString &text, int cursorPos)
{
    // 从光标位置向左查找，找到第一个数字位（0或1）
    for (int i = cursorPos - 1; i >= 0; i--) {
        QChar c = text[i];
        if (c == '0' || c == '1') {
            return i;
        }
    }
    return -1; // 没找到
}

int MainWindow::findRightDigitPos(const QString &text, int cursorPos)
{
    // 从光标位置向右查找，跳过空格和|，找到第一个数字位（0或1）
    for (int i = cursorPos; i < text.length(); i++) {
        QChar c = text[i];
        if (c == '0' || c == '1') {
            return i;
        }
    }
    return -1; // 没找到
}

bool MainWindow::handleBinResultDigitInput(const QString &digit)
{
    QLineEdit *edit = ui->editBinResult;
    if (!edit || (digit != "0" && digit != "1")) return false;
    
    QString text = edit->text();
    int cursorPos = edit->cursorPosition();
    
    // 找到光标左边最近的数字位
    int leftPos = findLeftDigitPos(text, cursorPos);
    
    if (leftPos >= 0) {
        // 修改该位置的数字
        text[leftPos] = digit[0];
        
        // 更新文本
        isUpdating = true;
        edit->setText(text);
        isUpdating = false;
        
        // 从修改位置向右找下一个数字位（跳过空格和|）
        int rightPos = findRightDigitPos(text, leftPos + 1);
        if (rightPos >= 0) {
            edit->setCursorPosition(rightPos + 1);
        } else {
            // 如果右边没有数字位了，光标移到末尾
            edit->setCursorPosition(text.length());
        }
        
        // 触发文本变化信号
        onBinResultChanged(text);
        return true; // 已处理
    }
    return false; // 未处理
}

bool MainWindow::handleBinResultKeyEvent(QKeyEvent *keyEvent)
{
    QLineEdit *edit = ui->editBinResult;
    if (!edit) return false;
    
    QString text = edit->text();
    int cursorPos = edit->cursorPosition();
    
    // 处理数字输入（0或1）
    if (keyEvent->key() >= Qt::Key_0 && keyEvent->key() <= Qt::Key_1) {
        QString digit = QString::number(keyEvent->key() - Qt::Key_0);
        
        // 找到光标左边最近的数字位
        int leftPos = findLeftDigitPos(text, cursorPos);
        
        if (leftPos >= 0) {
            // 修改该位置的数字
            text[leftPos] = digit[0];
            
            // 更新文本
            isUpdating = true;
            edit->setText(text);
            isUpdating = false;
            
            // 从修改位置向右找下一个数字位（跳过空格和|）
            int rightPos = findRightDigitPos(text, leftPos + 1);
            if (rightPos >= 0) {
                edit->setCursorPosition(rightPos + 1);
            } else {
                // 如果右边没有数字位了，光标移到末尾
                edit->setCursorPosition(text.length());
            }
            
            // 触发文本变化信号
            onBinResultChanged(text);
            return true; // 已处理
        }
        return true; // 即使没找到也阻止默认行为
    }
    
    // 处理Backspace
    if (keyEvent->key() == Qt::Key_Backspace) {
        // 找到光标左边最近的数字位
        int leftPos = findLeftDigitPos(text, cursorPos);
        
        if (leftPos >= 0) {
            // 将该位置置零
            text[leftPos] = '0';
            
            // 更新文本
            isUpdating = true;
            edit->setText(text);
            isUpdating = false;
            
            // 光标移到被置零的位置，然后左移一位（跳过空格）
            int newPos = leftPos;
            if (newPos > 0) {
                // 跳过空格
                while (newPos >= 0 && text[newPos] == ' ') {
                    newPos--;
                }
            }
            
            // 检查修改完后光标是否位于"|"右侧
            if (newPos >= 0 && newPos < text.length()) {
                // 检查newPos右侧是否有|
                bool isAfterPipe = false;
                for (int i = newPos + 1; i < text.length(); i++) {
                    QChar c = text[i];
                    if (c == '|') {
                        isAfterPipe = true;
                        break;
                    } else if (c == '0' || c == '1') {
                        break; // 遇到数字，说明不在|右侧
                    } else if (c != ' ') {
                        break; // 遇到其他字符
                    }
                }
                
                if (isAfterPipe) {
                    // 位于|右侧，额外左移一位（跳过空格）
                    if (newPos > 0) {
                        // 跳过空格
                        while (newPos >= 0 && text[newPos] == ' ') {
                            newPos--;
                        }
                    }
                }
            }
            
            edit->setCursorPosition(qMax(0, newPos));
            
            // 触发文本变化信号
            onBinResultChanged(text);
            return true; // 已处理
        }
        return true; // 即使没找到也阻止默认行为
    }
    
    return false; // 未处理，使用默认行为
}

// -------------------------------
// 核心逻辑：事件过滤器（修改后）
// -------------------------------
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // 0. 特殊处理：二进制分割结果输入框的键盘事件
    if (obj == ui->editBinResult && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (handleBinResultKeyEvent(keyEvent)) {
            return true; // 已处理，阻止默认行为
        }
    }
    
    // 1. 处理 BIN 分割规则输入框 (editSplitRule)
    if (obj == ui->editSplitRule) {
        static Base splitPrevBase = DEC;
        static bool splitActive = false;

        if (event->type() == QEvent::FocusIn) {
            if (!splitActive) {
                splitPrevBase = currentBase;
                splitActive = true;
            }

            // 规则：进入时切换为十进制环境，但允许输入逗号
            currentBase = DEC;
            setButtonEnabledByBase(currentBase);

            // 特殊处理：启用 0-9 数字键和逗号键，禁用其他（如运算符）
            for (auto it = digitButtons.begin(); it != digitButtons.end(); ++it) {
                QString key = it.key();
                bool isDigit = (key.size() == 1 && key[0].isDigit());
                it.value()->setEnabled(isDigit);
            }

            // 强制启用逗号按钮 (假设对象名为 btnComma)
            if (ui->btnComma) ui->btnComma->setEnabled(true);

            // 禁用不相关的运算符按钮
            for (auto it = operatorButtons.begin(); it != operatorButtons.end(); ++it) {
                it.value()->setEnabled(false);
            }

        } else if (event->type() == QEvent::FocusOut) {
            if (splitActive) {
                currentBase = splitPrevBase;
                splitActive = false;

                // 恢复之前进制的按钮状态
                setButtonEnabledByBase(currentBase);

                // 退出分割编辑，必须禁用逗号按钮（因为基础进制不需要逗号）
                if (ui->btnComma) ui->btnComma->setEnabled(false);

                // 恢复所有非数字按钮（运算符、清空键等）
                const auto allButtons = this->findChildren<QPushButton*>();
                for (QPushButton *btn : allButtons) {
                    // 如果不是数字按钮，则恢复启用（如 AC, Backspace, Operators）
                    bool isDigit = false;
                    for (auto it = digitButtons.begin(); it != digitButtons.end(); ++it) {
                        if (it.value() == btn) { isDigit = true; break; }
                    }
                    if (!isDigit && btn != ui->btnComma) {
                        btn->setEnabled(true);
                    }
                }
            }
        }
    }
    // 2. 处理各进制输入框的点击切换
    else if (event->type() == QEvent::FocusIn || event->type() == QEvent::MouseButtonDblClick) {
        QLineEdit* edit = qobject_cast<QLineEdit*>(obj);
        if (edit) {
            // 记录焦点，用于按钮点击后的字符插入
            if (edit == ui->editExpression ||
                edit == ui->editHex || edit == ui->editDec || edit == ui->editOct || edit == ui->editBin ||
                edit == ui->editHexResult || edit == ui->editDecResult || edit == ui->editBinResult) {
                lastFocusedEdit = edit;
            }
        }

        Base newBase = currentBase;
        bool isBaseField = true;

        if (obj == ui->editHex || obj == ui->editHexResult) {
            newBase = HEX;
        } else if (obj == ui->editDec || obj == ui->editDecResult) {
            newBase = DEC;
        } else if (obj == ui->editOct) {
            newBase = OCT;
        } else if (obj == ui->editBin || obj == ui->editBinResult) {
            newBase = BIN;
        } else {
            isBaseField = false;
        }

        if (isBaseField) {
            currentBase = newBase;
            setButtonEnabledByBase(currentBase);
            // 只要不是在编辑 SplitRule，逗号永远禁用
            if (ui->btnComma) ui->btnComma->setEnabled(false);
            qDebug() << "Switch to Base:" << currentBase;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

// -------------------------------
// 获取当前获得焦点的输入框
// -------------------------------
QLineEdit* MainWindow::getFocusedEditBox()
{
    QWidget* focused = QApplication::focusWidget();
    QLineEdit* edit = qobject_cast<QLineEdit*>(focused);
    if (edit) {
        // 检查是否是我们的输入框或结果框
        if (edit == ui->editExpression ||
            edit == ui->editHex || edit == ui->editDec || edit == ui->editOct || edit == ui->editBin ||
            edit == ui->editHexResult || edit == ui->editDecResult || edit == ui->editBinResult) {
            return edit;
        }
    }
    return nullptr;
}
