#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

// -------------------------------
// 工具函数
// -------------------------------
bool MainWindow::checkValueOverflow(const QString &text, Base base)
{
    if (text.isEmpty()) return false;
    
    QString cleanText = text;
    cleanText.remove(' '); // 移除空格
    cleanText.remove('|'); // 移除分割符
    
    if (cleanText.isEmpty()) return false;
    
    // 64位无符号整数的最大值是 2^64 - 1 = 18446744073709551615
    const unsigned long long MAX_64BIT = 9223372036854775807ULL;
    
    // 先检查字符串长度，如果明显超出，直接返回true
    // 二进制：64位 = 64个字符
    // 八进制：64位 = 22个字符 (1777777777777777777777)
    // 十进制：64位 = 20个字符 (18446744073709551615)
    // 十六进制：64位 = 16个字符 (FFFFFFFFFFFFFFFF)
    int maxLength;
    switch (base) {
        case BIN:
            maxLength = 64;
            // 移除前导0
            while (cleanText.startsWith('0') && cleanText.length() > 1) {
                cleanText.remove(0, 1);
            }
            if (cleanText.length() > maxLength) return true;
            // 检查是否包含无效字符
            for (QChar c : cleanText) {
                if (c != '0' && c != '1') return false; // 无效字符，不认为是溢出
            }
            break;
        case OCT:
            maxLength = 22;
            while (cleanText.startsWith('0') && cleanText.length() > 1) {
                cleanText.remove(0, 1);
            }
            if (cleanText.length() > maxLength) return true;
            for (QChar c : cleanText) {
                if (c < '0' || c > '7') return false;
            }
            break;
        case DEC:
            maxLength = 20;
            while (cleanText.startsWith('0') && cleanText.length() > 1) {
                cleanText.remove(0, 1);
            }
            if (cleanText.length() > maxLength) return true;
            for (QChar c : cleanText) {
                if (c < '0' || c > '9') return false;
            }
            break;
        case HEX:
            maxLength = 16;
            cleanText = cleanText.toUpper();
            while (cleanText.startsWith('0') && cleanText.length() > 1) {
                cleanText.remove(0, 1);
            }
            if (cleanText.length() > maxLength) return true;
            for (QChar c : cleanText) {
                if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) return false;
            }
            break;
    }
    
    // 如果长度在范围内，尝试转换并比较值
    bool ok;
    unsigned long long value;
    
    switch (base) {
        case BIN:
            value = cleanText.toULongLong(&ok, 2);
            break;
        case OCT:
            value = cleanText.toULongLong(&ok, 8);
            break;
        case DEC:
            value = cleanText.toULongLong(&ok, 10);
            break;
        case HEX:
            value = cleanText.toULongLong(&ok, 16);
            break;
    }
    
    // 如果转换失败，可能是无效字符，不认为是溢出
    if (!ok) return false;
    
    // 比较值
    return value > MAX_64BIT;
}

// -------------------------------
// 输入框文本变化处理
// -------------------------------
void MainWindow::onHexInputChanged(const QString &text)
{
    if (isUpdating) return;
    if (text.isEmpty()) return;

    // 检查是否超出64位范围
    if (checkValueOverflow(text, HEX)) {
        QMessageBox::warning(this, "输入过多", "输入内容超出64位二进制数能表示的范围！");
        // 阻止最后一个输入：删除最后一个字符
        QLineEdit* edit = qobject_cast<QLineEdit*>(sender());
        if (edit) {
            QString currentText = edit->text();
            if (!currentText.isEmpty()) {
                currentText.chop(1);
                isUpdating = true;
                edit->setText(currentText);
                isUpdating = false;
            }
        }
        return;
    }

    bool ok;
    long long value = text.toLongLong(&ok, 16);
    if (ok) {
        updateFromInputValue(value, HEX);
    }
}

void MainWindow::onDecInputChanged(const QString &text)
{
    if (isUpdating) return;
    if (text.isEmpty()) return;

    // 检查是否超出64位范围
    if (checkValueOverflow(text, DEC)) {
        QMessageBox::warning(this, "输入过多", "输入内容超出64位二进制数能表示的范围！");
        // 阻止最后一个输入：删除最后一个字符
        QLineEdit* edit = qobject_cast<QLineEdit*>(sender());
        if (edit) {
            QString currentText = edit->text();
            if (!currentText.isEmpty()) {
                currentText.chop(1);
                isUpdating = true;
                edit->setText(currentText);
                isUpdating = false;
            }
        }
        return;
    }

    bool ok;
    long long value = text.toLongLong(&ok, 10);
    if (ok) {
        updateFromInputValue(value, DEC);
    }
}

void MainWindow::onOctInputChanged(const QString &text)
{
    if (isUpdating) return;
    if (text.isEmpty()) return;

    // 检查是否超出64位范围
    if (checkValueOverflow(text, OCT)) {
        QMessageBox::warning(this, "输入过多", "输入内容超出64位二进制数能表示的范围！");
        // 阻止最后一个输入：删除最后一个字符
        QLineEdit* edit = qobject_cast<QLineEdit*>(sender());
        if (edit) {
            QString currentText = edit->text();
            if (!currentText.isEmpty()) {
                currentText.chop(1);
                isUpdating = true;
                edit->setText(currentText);
                isUpdating = false;
            }
        }
        return;
    }

    bool ok;
    long long value = text.toLongLong(&ok, 8);
    if (ok) {
        updateFromInputValue(value, OCT);
    }
}

void MainWindow::onBinInputChanged(const QString &text)
{
    if (isUpdating) return;
    if (text.isEmpty()) return;

    // 检查是否超出64位范围
    if (checkValueOverflow(text, BIN)) {
        QMessageBox::warning(this, "输入过多", "输入内容超出64位二进制数能表示的范围！");
        // 阻止最后一个输入：删除最后一个字符
        QLineEdit* edit = qobject_cast<QLineEdit*>(sender());
        if (edit) {
            QString currentText = edit->text();
            if (!currentText.isEmpty()) {
                currentText.chop(1);
                isUpdating = true;
                edit->setText(currentText);
                isUpdating = false;
            }
        }
        return;
    }

    // 移除空格以便解析
    QString cleanText = text;
    cleanText.remove(' ');
    
    bool ok;
    long long value = cleanText.toLongLong(&ok, 2);
    if (ok) {
        updateFromInputValue(value, BIN);
    }
}

void MainWindow::onSplitRuleChanged(const QString &text)
{
    // 检查分割位数之和是否超过 64 位，并设置颜色
    int totalBits = 0;
    const QStringList parts = text.split(',', QString::SkipEmptyParts);
    for (const QString &p : parts) {
        bool okLen = false;
        int len = p.trimmed().toInt(&okLen);
        if (okLen && len > 0) {
            totalBits += len;
        }
    }
    if (totalBits > 64) {
        ui->editSplitRule->setStyleSheet("QLineEdit { color: red; }");
    } else {
        ui->editSplitRule->setStyleSheet(QString());
    }

    // 保存分割规则输入框的光标位置
    int savedCursorPos = ui->editSplitRule->cursorPosition();
    
    // 只要规则变了，就基于当前的 editDec（原始数值）重新跑一遍所有显示逻辑
    bool ok;
    long long currentVal = ui->editDec->text().toLongLong(&ok);
    if(ok) {
        updateAllDisplays(currentVal);
    }
    
    // 恢复分割规则输入框的光标位置
    int maxPos = ui->editSplitRule->text().length();
    int restorePos = qMin(savedCursorPos, maxPos);
    ui->editSplitRule->setCursorPosition(restorePos);
}

void MainWindow::onEditChanged(const QString &text)
{
    Q_UNUSED(text);
}
