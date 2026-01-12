#include "mainwindow.h"
#include "ui_mainwindow.h"

// -------------------------------
// 输入框文本变化处理
// -------------------------------
void MainWindow::onHexInputChanged(const QString &text)
{
    if (isUpdating) return;
    if (text.isEmpty()) return;

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

    bool ok;
    long long value = text.toLongLong(&ok, 2);
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

    // 只要规则变了，就基于当前的 editDec（原始数值）重新跑一遍所有显示逻辑
    bool ok;
    long long currentVal = ui->editDec->text().toLongLong(&ok);
    if(ok) {
        updateAllDisplays(currentVal);
    }
}

void MainWindow::onEditChanged(const QString &text)
{
    Q_UNUSED(text);
}
