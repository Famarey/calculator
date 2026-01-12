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
    // 更新基础显示
    ui->editDec->setText(QString::number(value, 10));
    ui->editHex->setText(QString::number(value, 16).toUpper());
    ui->editOct->setText(QString::number(value, 8));
    QString rawBin = QString::number(value, 2);
    ui->editBin->setText(rawBin);

    // 获取新的分割结果
    QString splitRule = ui->editSplitRule->text();
    QString formattedBin = formatBinWithSplit(rawBin, splitRule);
    ui->editBinResult->setText(formattedBin);

    // 如果没有分割，显示原始值
    if (splitRule.isEmpty() || !formattedBin.contains('|')) {
        ui->editDecResult->setText(ui->editDec->text());
        ui->editHexResult->setText(ui->editHex->text());
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
        long long partVal = part.toLongLong(&ok, 2);
        if (ok) {
            decParts << QString::number(partVal, 10);
            hexParts << QString::number(partVal, 16).toUpper();
        }
    }

    ui->editDecResult->setText(decParts.join('|'));
    ui->editHexResult->setText(hexParts.join('|'));
}

QString MainWindow::formatBinWithSplit(const QString &bin, const QString &rule)
{
    if (rule.isEmpty() || bin.isEmpty()) return bin;

    // 1. 解析规则并计算总长度
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

    QStringList resultParts;
    int binLen = bin.length();
    int currentPos = 0;

    // 2. 保证低位被分割到：计算高位剩下的长度
    // 如果规则总和大于二进制长度，剩余长度为0
    int remainderLen = qMax(0, binLen - totalRuleLen);

    // 3. 提取高位剩余部分
    if (remainderLen > 0) {
        resultParts << bin.left(remainderLen);
        currentPos = remainderLen;
    }

    // 4. 从相对高位开始，按规则顺序分割
    for (int l : lens) {
        if (currentPos >= binLen) break;

        // 提取当前长度，如果不满（说明规则设长了），则取到末尾
        int actualTake = qMin(l, binLen - currentPos);
        resultParts << bin.mid(currentPos, actualTake);
        currentPos += actualTake;
    }

    // 5. 如果规则跑完了还有剩下的（通常在 remainderLen 逻辑下不会发生，除非逻辑溢出），补齐
    if (currentPos < binLen) {
        resultParts << bin.mid(currentPos);
    }

    return resultParts.join('|');
}
