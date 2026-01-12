#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStringList>

// -------------------------------
// 更新模式变化处理（复选框）
// -------------------------------
void MainWindow::onUpdateModeChanged(int value)
{
    // QCheckBox::stateChanged 传入 0/2，这里统一归一到 0/1
    value = (value != 0) ? 1 : 0;
    // 当从"仅更新数值"滑到"同步表达式"时
    if (lastUpdateMode == 0 && value == 1) {
        // 清空表达式
        ui->editExpression->clear();

        // 根据当前进制，将当前输入框中的内容放到表达式输入框中
        QString currentValue;
        if (currentBase == HEX) {
            currentValue = ui->editHex->text();
        } else if (currentBase == DEC) {
            currentValue = ui->editDec->text();
        } else if (currentBase == OCT) {
            currentValue = ui->editOct->text();
        } else if (currentBase == BIN) {
            currentValue = ui->editBin->text();
        }

        if (!currentValue.isEmpty()) {
            ui->editExpression->setText(currentValue);
        }
    }

    lastUpdateMode = value;
}

// -------------------------------
// 从输入值更新所有显示
// -------------------------------
void MainWindow::updateFromInputValue(long long value, Base inputBase)
{
    isUpdating = true;

    // 保存当前的分割规则
    QString splitRule = ui->editSplitRule->text();

    // 更新所有基础显示
    updateAllDisplays(value);

    // 恢复分割规则（updateAllDisplays可能会触发onSplitRuleChanged，但我们已经保存了规则）
    ui->editSplitRule->setText(splitRule);

    // 根据复选框位置决定是否更新表达式
    int updateMode = ui->chkSyncExpression->isChecked() ? 1 : 0;
    if (updateMode == 1) { // 同步表达式模式
        // 根据被修改的输入框，更新表达式
        QString exprValue;
        if (inputBase == HEX) {
            exprValue = ui->editHex->text();
        } else if (inputBase == DEC) {
            exprValue = ui->editDec->text();
        } else if (inputBase == OCT) {
            exprValue = ui->editOct->text();
        } else if (inputBase == BIN) {
            exprValue = ui->editBin->text();
        }

        if (!exprValue.isEmpty()) {
            ui->editExpression->setText(exprValue);
        }
    }
    // 如果updateMode == 0（仅更新数值），则不更新表达式

    isUpdating = false;
}

// -------------------------------
// 从结果值更新所有显示（保持BIN分割不变）
// -------------------------------
void MainWindow::updateFromResultValue(const QString &resultText, Base resultBase)
{
    if (isUpdating) return;
    isUpdating = true;

    // 获取当前的分割规则和BIN结果
    QString splitRule = ui->editSplitRule->text();
    QString currentBinResult = ui->editBinResult->text();

    // 如果分割规则为空，直接按单个值处理
    if (splitRule.isEmpty() || !currentBinResult.contains('|')) {
        bool ok;
        long long value = 0;
        QString cleanText = resultText;
        if (resultBase == BIN) {
            value = cleanText.replace("|", "").toLongLong(&ok, 2);
        } else if (resultBase == DEC) {
            value = cleanText.replace("|", "").toLongLong(&ok, 10);
        } else if (resultBase == HEX) {
            value = cleanText.replace("|", "").toLongLong(&ok, 16);
        }

        if (ok) {
            updateAllDisplays(value);
        }
        isUpdating = false;
        return;
    }

    // 获取当前BIN结果的段数（用于确定分割结构）
    QStringList binParts = currentBinResult.split('|');
    int numParts = binParts.size();

    // 解析结果文本的每一段
    QStringList resultParts = resultText.split('|');

    // 如果结果段数不匹配，尝试按单个值处理
    if (resultParts.size() != numParts) {
        // 尝试将整个结果文本作为一个值解析
        bool ok;
        long long totalValue = 0;
        QString cleanText = resultText;
        if (resultBase == BIN) {
            totalValue = cleanText.replace("|", "").toLongLong(&ok, 2);
        } else if (resultBase == DEC) {
            totalValue = cleanText.replace("|", "").toLongLong(&ok, 10);
        } else if (resultBase == HEX) {
            totalValue = cleanText.replace("|", "").toLongLong(&ok, 16);
        }

        if (ok) {
            updateAllDisplays(totalValue);
        }
        isUpdating = false;
        return;
    }

    // 解析分割规则，获取每段的位数
    QStringList ruleStrings = splitRule.split(',');
    QList<int> bitLengths;
    int totalRuleLen = 0;
    for (const QString& s : ruleStrings) {
        bool ok;
        int l = s.trimmed().toInt(&ok);
        if (ok && l > 0) {
            bitLengths << l;
            totalRuleLen += l;
        }
    }

    // 获取当前完整的二进制值，用于确定总长度
    QString currentBin = ui->editBin->text();
    int binLen = currentBin.length();

    // 计算高位剩余部分的长度（与formatBinWithSplit逻辑一致）
    int remainderLen = qMax(0, binLen - totalRuleLen);

    // 计算总的二进制值
    long long totalValue = 0;
    int bitOffset = 0;

    // 从高位到低位处理每一段
    // 第一段可能是高位剩余部分
    for (int i = 0; i < qMin(resultParts.size(), binParts.size()); i++) {
        bool ok;
        long long partValue = 0;

        if (resultBase == BIN) {
            partValue = resultParts[i].toLongLong(&ok, 2);
        } else if (resultBase == DEC) {
            partValue = resultParts[i].toLongLong(&ok, 10);
        } else if (resultBase == HEX) {
            partValue = resultParts[i].toLongLong(&ok, 16);
        }

        if (!ok) continue;

        // 确定该段应该占用的位数
        int bitLen = 0;
        if (i == 0 && remainderLen > 0) {
            // 第一段是高位剩余部分
            bitLen = remainderLen;
        } else {
            // 其他段按规则分割
            int ruleIndex = (remainderLen > 0) ? (i - 1) : i;
            if (ruleIndex >= 0 && ruleIndex < bitLengths.size()) {
                bitLen = bitLengths[ruleIndex];
            } else if (!binParts[i].isEmpty()) {
                // 如果没有规则，使用当前段的长度
                bitLen = binParts[i].length();
            }
        }

        if (bitLen == 0) continue;

        // 计算该段能表示的最大值
        long long maxValue = (1LL << bitLen) - 1;

        // 如果值超出范围，截取低位
        if (partValue > maxValue) {
            partValue = partValue & maxValue;
        }

        // 计算该段在总二进制中的位置（从低位开始）
        // binParts是从高位到低位的顺序，所以需要计算该段之后所有段的位数
        int currentBitOffset = 0;

        // 计算该段之后的所有段的位数（从低位开始，即从最后一个段开始）
        for (int j = resultParts.size() - 1; j > i; j--) {
            int segBitLen = 0;
            if (j == 0 && remainderLen > 0) {
                segBitLen = remainderLen;
            } else {
                int ruleIdx = (remainderLen > 0) ? (j - 1) : j;
                if (ruleIdx >= 0 && ruleIdx < bitLengths.size()) {
                    segBitLen = bitLengths[ruleIdx];
                } else if (j < binParts.size() && !binParts[j].isEmpty()) {
                    segBitLen = binParts[j].length();
                }
            }
            currentBitOffset += segBitLen;
        }

        // 将这段值放到正确的位置（从低位开始）
        totalValue |= (partValue << currentBitOffset);
    }

    // 更新所有显示
    updateAllDisplays(totalValue);

    isUpdating = false;
}
