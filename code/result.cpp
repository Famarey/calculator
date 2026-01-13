#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

// -------------------------------
// 结果框文本变化处理
// -------------------------------
void MainWindow::onBinResultChanged(const QString &text)
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

    updateFromResultValue(text, BIN);
}

void MainWindow::onDecResultChanged(const QString &text)
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

    updateFromResultValue(text, DEC);
}

void MainWindow::onHexResultChanged(const QString &text)
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

    updateFromResultValue(text, HEX);
}
