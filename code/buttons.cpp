#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QMessageBox>

// -------------------------------
// 按钮槽函数
// -------------------------------
void MainWindow::onDigitButtonClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    QString text = btn->text();
    QLineEdit* focusedEdit = getFocusedEditBox();
    // 如果当前没有焦点在输入框上，使用最后获得焦点的输入框
    if (!focusedEdit) {
        focusedEdit = lastFocusedEdit;
    }

    int updateMode = ui->chkSyncExpression->isChecked() ? 1 : 0;

    // 在"仅更新数值"模式下，不更新表达式框
    if (updateMode == 0) {
        if (focusedEdit && focusedEdit != ui->editExpression) {
            // 焦点在结果框或输入框上，直接插入到该输入框
            focusedEdit->insert(text);

            // 触发文本变化信号来更新其他显示
            if (focusedEdit == ui->editBinResult) {
                onBinResultChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editDecResult) {
                onDecResultChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editHexResult) {
                onHexResultChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editHex) {
                onHexInputChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editDec) {
                onDecInputChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editOct) {
                onOctInputChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editBin) {
                onBinInputChanged(focusedEdit->text());
            }
        } else {
            // 如果没有焦点在输入框上，根据当前进制更新对应的输入框
            if (currentBase == HEX) {
                ui->editHex->insert(text);
                onHexInputChanged(ui->editHex->text());
            } else if (currentBase == DEC) {
                ui->editDec->insert(text);
                onDecInputChanged(ui->editDec->text());
            } else if (currentBase == OCT) {
                ui->editOct->insert(text);
                onOctInputChanged(ui->editOct->text());
            } else if (currentBase == BIN) {
                ui->editBin->insert(text);
                onBinInputChanged(ui->editBin->text());
            }
        }
    } else {
        // "同步表达式"模式下，如果焦点在表达式框，更新表达式；否则根据焦点位置更新
        if (focusedEdit == ui->editExpression) {
            focusedEdit->insert(text);
        } else if (focusedEdit && focusedEdit != ui->editExpression) {
            // 焦点在结果框或输入框上，直接插入到该输入框
            focusedEdit->insert(text);

            // 触发文本变化信号来更新其他显示
            if (focusedEdit == ui->editBinResult) {
                onBinResultChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editDecResult) {
                onDecResultChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editHexResult) {
                onHexResultChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editHex) {
                onHexInputChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editDec) {
                onDecInputChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editOct) {
                onOctInputChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editBin) {
                onBinInputChanged(focusedEdit->text());
            }
        } else {
            // 如果没有焦点在输入框上，默认插入到表达式框
            ui->editExpression->insert(text);
        }
    }
}

void MainWindow::onOperatorButtonClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    QString text = btn->text();
    QLineEdit* focusedEdit = getFocusedEditBox();
    
    // 如果当前没有焦点在输入框上，使用最后获得焦点的输入框
    if (!focusedEdit) {
        focusedEdit = lastFocusedEdit;
    }

    int updateMode = ui->chkSyncExpression->isChecked() ? 1 : 0;

    // 运算符只能插入到表达式框
    if (focusedEdit == ui->editExpression) {
        focusedEdit->insert(text);
    } else {
        // 如果焦点不在表达式框，插入到表达式框
        // 在"同步表达式"模式下，确保表达式框获得焦点
        if (updateMode == 1) {
            ui->editExpression->setFocus();
            lastFocusedEdit = ui->editExpression;
        }
        ui->editExpression->insert(text);
    }
}

void MainWindow::onBackspaceClicked()
{
    QLineEdit* focusedEdit = getFocusedEditBox();
    // 如果当前没有焦点在输入框上，使用最后获得焦点的输入框
    if (!focusedEdit) {
        focusedEdit = lastFocusedEdit;
    }

    int updateMode = ui->chkSyncExpression->isChecked() ? 1 : 0;

    // 在"仅更新数值"模式下，不更新表达式框
    if (updateMode == 0) {
        if (focusedEdit && focusedEdit != ui->editExpression) {
            focusedEdit->backspace();

            // 触发文本变化信号来更新其他显示
            if (focusedEdit == ui->editBinResult) {
                onBinResultChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editDecResult) {
                onDecResultChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editHexResult) {
                onHexResultChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editHex) {
                onHexInputChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editDec) {
                onDecInputChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editOct) {
                onOctInputChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editBin) {
                onBinInputChanged(focusedEdit->text());
            }
        } else {
            // 如果没有焦点在输入框上，根据当前进制更新对应的输入框
            if (currentBase == HEX) {
                ui->editHex->backspace();
                onHexInputChanged(ui->editHex->text());
            } else if (currentBase == DEC) {
                ui->editDec->backspace();
                onDecInputChanged(ui->editDec->text());
            } else if (currentBase == OCT) {
                ui->editOct->backspace();
                onOctInputChanged(ui->editOct->text());
            } else if (currentBase == BIN) {
                ui->editBin->backspace();
                onBinInputChanged(ui->editBin->text());
            }
        }
    } else {
        // "同步表达式"模式下，如果焦点在表达式框，更新表达式；否则根据焦点位置更新
        if (focusedEdit == ui->editExpression) {
            focusedEdit->backspace();
        } else if (focusedEdit && focusedEdit != ui->editExpression) {
            focusedEdit->backspace();

            // 触发文本变化信号来更新其他显示
            if (focusedEdit == ui->editBinResult) {
                onBinResultChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editDecResult) {
                onDecResultChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editHexResult) {
                onHexResultChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editHex) {
                onHexInputChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editDec) {
                onDecInputChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editOct) {
                onOctInputChanged(focusedEdit->text());
            } else if (focusedEdit == ui->editBin) {
                onBinInputChanged(focusedEdit->text());
            }
        } else {
            // 如果没有焦点在输入框上，默认操作表达式框
            ui->editExpression->backspace();
        }
    }
}

void MainWindow::onEqualClicked()
{
    QString expr = ui->editExpression->text();
    if(expr.isEmpty()) return;
    try {
        // 假设 evaluateExpression 返回 long long
        long long result = evaluateExpression(expr, currentBase);

        // 更新所有显示框
        updateAllDisplays(result);

        // 显式触发一次分割刷新
        QString rawBin = ui->editBin->text();
        ui->editBinResult->setText(formatBinWithSplit(rawBin, ui->editSplitRule->text()));
    } catch (...) {
        QMessageBox::warning(this, "Error", "表达式错误");
    }
}

void MainWindow::onClearClicked()
{
    isUpdating = true;

    // 清空所有输入框
    ui->editExpression->clear();
    ui->editHex->clear();
    ui->editDec->clear();
    ui->editOct->clear();
    ui->editBin->clear();
    ui->editBinResult->clear();
    ui->editDecResult->clear();
    ui->editHexResult->clear();
    ui->editSplitRule->clear();

    // 分割规则恢复默认样式
    ui->editSplitRule->setStyleSheet(QString());

    // 重置最后获得焦点的输入框为表达式框
    lastFocusedEdit = ui->editExpression;
    // 将焦点设置到表达式框
    ui->editExpression->setFocus();

    isUpdating = false;
}

// -------------------------------
// 逗号按钮（用于 BIN 分割规则）
// -------------------------------
void MainWindow::onBitOperatorClicked()
{
    // 这里复用现有空槽名，专门处理逗号按钮
    // 仅在分割规则输入框有焦点时插入逗号
    if (ui->editSplitRule->hasFocus()) {
        ui->editSplitRule->insert(",");
    }
}
