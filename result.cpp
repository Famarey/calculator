#include "mainwindow.h"
#include "ui_mainwindow.h"

// -------------------------------
// 结果框文本变化处理
// -------------------------------
void MainWindow::onBinResultChanged(const QString &text)
{
    if (isUpdating) return;
    if (text.isEmpty()) return;

    updateFromResultValue(text, BIN);
}

void MainWindow::onDecResultChanged(const QString &text)
{
    if (isUpdating) return;
    if (text.isEmpty()) return;

    updateFromResultValue(text, DEC);
}

void MainWindow::onHexResultChanged(const QString &text)
{
    if (isUpdating) return;
    if (text.isEmpty()) return;

    updateFromResultValue(text, HEX);
}
