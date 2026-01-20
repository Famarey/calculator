#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QPushButton>
#include <QLineEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override; // 双击切换进制
    void resizeEvent(QResizeEvent *event) override;          // 窗口大小变化时自适应字体和按钮高度

private slots:
    void onDigitButtonClicked();
    void onOperatorButtonClicked();
    void onBackspaceClicked();
    void onEqualClicked();
    void onEditChanged(const QString &text);
    void onSplitRuleChanged(const QString &text);
    void onBitOperatorClicked(); // 逗号按钮处理
    void onHexInputChanged(const QString &text);
    void onDecInputChanged(const QString &text);
    void onOctInputChanged(const QString &text);
    void onBinInputChanged(const QString &text);
    void onBinResultChanged(const QString &text);
    void onDecResultChanged(const QString &text);
    void onHexResultChanged(const QString &text);
    void onUpdateModeChanged(int value);
    void onClearClicked();
    void onResetClicked(); // 归零按钮处理

private:
    Ui::MainWindow *ui;

    enum Base { BIN=2, OCT=8, DEC=10, HEX=16 };
    Base currentBase;

    QMap<QString, QPushButton*> digitButtons;
    QMap<QString, QPushButton*> operatorButtons;

    void setButtonEnabledByBase(Base base);
    void updateAllDisplays(long long value);
    QString formatBinWithSplit(const QString &bin, const QString &rule);
    QString formatBinWithSpaces(const QString &bin); // 每四位数字后加空格
    long long evaluateExpression(const QString &expr, Base base);
    void updateFromInputValue(long long value, Base inputBase = DEC);
    void updateFromResultValue(const QString &resultText, Base resultBase);
    bool checkValueOverflow(const QString &text, Base base); // 检查输入是否超出64位范围
    bool validateExpression(const QString &expr, Base base, QString &errorMsg); // 检查表达式是否合法
    bool handleBinResultKeyEvent(QKeyEvent *keyEvent); // 处理二进制分割结果的键盘事件
    bool handleBinResultDigitInput(const QString &digit); // 处理二进制分割结果的数字输入（用于按钮点击）
    int findLeftDigitPos(const QString &text, int cursorPos); // 找到光标左边最近的数字位位置
    int findRightDigitPos(const QString &text, int cursorPos); // 找到光标右边最近的数字位位置（跳过空格和|）
    QLineEdit* getFocusedEditBox(); // 获取当前获得焦点的输入框
    QLineEdit* lastFocusedEdit; // 记录最后获得焦点的输入框
    bool isUpdating; // 防止循环更新
    int lastUpdateMode; // 记录上一次的更新模式
};

#endif // MAINWINDOW_H
