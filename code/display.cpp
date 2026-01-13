#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStringList>

// -------------------------------
// 工具函数
// -------------------------------
void MainWindow::setButtonEnabledByBase(Base base)
{
    // 更新按钮可用性
    for(auto it = digitButtons.begin(); it != digitButtons.end(); ++it) {
        QString key = it.key();
        bool enable = false;
        if(base == BIN) enable = (key == "0" || key == "1");
        else if(base == OCT) enable = (key >= "0" && key <= "7");
        else if(base == DEC) enable = (key >= "0" && key <= "9");
        else if(base == HEX) enable = true;
        it.value()->setEnabled(enable);
    }
    ui->btnComma->setEnabled(ui->editSplitRule->hasFocus());

    // 更新UI提示（可选：改变背景色区分当前进制）
    QString style = "QLineEdit { background-color: #FFFFFF; }";
    QString activeStyle = "QLineEdit { background-color: #E1F5FE; border: 1px solid #03A9F4; }";

    ui->editHex->setStyleSheet(base == HEX ? activeStyle : style);
    ui->editDec->setStyleSheet(base == DEC ? activeStyle : style);
    ui->editOct->setStyleSheet(base == OCT ? activeStyle : style);
    ui->editBin->setStyleSheet(base == BIN ? activeStyle : style);
}

void MainWindow::updateAllDisplays(long long value)
{
    // 保存所有输入框的光标位置
    QMap<QLineEdit*, int> cursorPositions;
    cursorPositions[ui->editDec] = ui->editDec->cursorPosition();
    cursorPositions[ui->editHex] = ui->editHex->cursorPosition();
    cursorPositions[ui->editOct] = ui->editOct->cursorPosition();
    cursorPositions[ui->editBin] = ui->editBin->cursorPosition();
    cursorPositions[ui->editBinResult] = ui->editBinResult->cursorPosition();
    cursorPositions[ui->editDecResult] = ui->editDecResult->cursorPosition();
    cursorPositions[ui->editHexResult] = ui->editHexResult->cursorPosition();
    cursorPositions[ui->editExpression] = ui->editExpression->cursorPosition();
    cursorPositions[ui->editSplitRule] = ui->editSplitRule->cursorPosition();

    // 更新基础显示
    ui->editDec->setText(QString::number(value, 10));
    ui->editHex->setText(QString::number(value, 16).toUpper());
    ui->editOct->setText(QString::number(value, 8));
    QString rawBin = QString::number(value, 2);
    ui->editBin->setText(formatBinWithSpaces(rawBin));

    // 获取新的分割结果
    QString splitRule = ui->editSplitRule->text();
    QString formattedBin = formatBinWithSplit(rawBin, splitRule);
    ui->editBinResult->setText(formattedBin);

    // 如果没有分割，显示原始值
    if (splitRule.isEmpty() || !formattedBin.contains('|')) {
        ui->editDecResult->setText(ui->editDec->text());
        ui->editHexResult->setText(ui->editHex->text());
        // 恢复所有输入框的光标位置
        for (auto it = cursorPositions.begin(); it != cursorPositions.end(); ++it) {
            QLineEdit* edit = it.key();
            int savedPos = it.value();
            int maxPos = edit->text().length();
            int restorePos = qMin(savedPos, maxPos);
            edit->setCursorPosition(restorePos);
        }
        return;
    }

    // 处理每一段的 DEC 和 HEX
    QStringList binParts = formattedBin.split('|');
    QStringList decParts;
    QStringList hexParts;

    for (const QString &part : binParts) {
        if (part.isEmpty()) {
            decParts << "0";
            hexParts << "0";
            continue;
        }
        bool ok;
        // 注意：每一段都是一个独立的二进制数
        // 移除空格以便解析
        QString cleanPart = part;
        cleanPart.remove(' ');
        long long partVal = cleanPart.toLongLong(&ok, 2);
        if (ok) {
            decParts << QString::number(partVal, 10);
            hexParts << QString::number(partVal, 16).toUpper();
        }
    }

    ui->editDecResult->setText(decParts.join('|'));
    ui->editHexResult->setText(hexParts.join('|'));

    // 恢复所有输入框的光标位置
    for (auto it = cursorPositions.begin(); it != cursorPositions.end(); ++it) {
        QLineEdit* edit = it.key();
        int savedPos = it.value();
        int maxPos = edit->text().length();
        int restorePos = qMin(savedPos, maxPos);
        edit->setCursorPosition(restorePos);
    }
}

QString MainWindow::formatBinWithSpaces(const QString &bin)
{
    if (bin.isEmpty()) return bin;
    
    QString result;
    // 从右到左每四位加一个空格
    for (int i = bin.length() - 1; i >= 0; i--) {
        if ((bin.length() - 1 - i) > 0 && (bin.length() - 1 - i) % 4 == 0) {
            result.prepend(' ');
        }
        result.prepend(bin[i]);
    }
    return result;
}

QString MainWindow::formatBinWithSplit(const QString &bin, const QString &rule)
{
    if (rule.isEmpty() || bin.isEmpty()) return bin;

    // 1. 解析规则并计算规则要求的总长度
    QStringList ruleStrings = rule.split(',');
    QList<int> lens;
    int totalRuleLen = 0;
    for (const QString& s : ruleStrings) {
        bool ok;
        int l = s.trimmed().toInt(&ok);
        if (ok && l > 0) {
            lens << l;
            totalRuleLen += l;
        }
    }

    if (lens.isEmpty()) return bin;

    // 2. 补0逻辑：如果原始二进制长度不足规则总长，在高位（左侧）补0
    QString paddedBin = bin;
    if (bin.length() < totalRuleLen) {
        paddedBin = bin.rightJustified(totalRuleLen, '0');
    }

    QStringList resultParts;
    int currentLen = paddedBin.length();
    int currentPos = 0;

    // 3. 计算"规则外"的高位部分
    // 如果 paddedBin 比 totalRuleLen 长（即用户规则只定义了低位的一部分），多出的高位作为第一段
    int remainderLen = currentLen - totalRuleLen;
    if (remainderLen > 0) {
        QString firstSegment = paddedBin.left(remainderLen);
        // 对第一段内部添加空格格式化
        resultParts << formatBinWithSpaces(firstSegment);
        currentPos = remainderLen;
    }

    // 4. 按照规则从高位向低位依次切分
    // 此时由于已经补过0，规则定义的每一段都能取满
    for (int l : lens) {
        if (currentPos >= currentLen) break;
        QString segment = paddedBin.mid(currentPos, l);
        // 对每段内部添加空格格式化
        resultParts << formatBinWithSpaces(segment);
        currentPos += l;
    }

    return resultParts.join('|');
}
