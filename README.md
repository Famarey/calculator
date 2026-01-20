# Calculator

一个功能完整的Qt计算器应用，支持基本计算和按位分割功能。

## 功能特性

- 基本算术运算（加、减、乘、除）
- 按位分割功能
- 清晰的用户界面
- 支持中文界面

## 技术栈

- C++
- Qt框架

## 项目结构

```
.
├── .vscode/          # VSCode配置文件
├── github/           # GitHub相关文件
├── buttons.cpp       # 按钮功能实现
├── cal.pro           # Qt项目配置文件
├── cal_zh_CN.ts      # 中文翻译文件
├── display.cpp       # 显示功能实现
├── expression.cpp    # 表达式处理
├── input.cpp         # 输入处理
├── main.cpp          # 主程序入口
├── mainwindow.cpp    # 主窗口实现
├── mainwindow.h      # 主窗口头文件
├── mainwindow.ui     # 主窗口UI设计
├── result.cpp        # 结果处理
└── update.cpp        # 更新功能
```

## 构建和运行

### 前提条件

- Qt开发环境（推荐Qt 5.15或更高版本）
- C++编译器（如GCC、MSVC等）

### 构建步骤

1. 使用Qt Creator打开`cal.pro`文件
2. 选择合适的构建套件
3. 点击构建按钮
4. 运行应用程序

### 命令行构建

```bash
# 在项目根目录执行
qmake
make
./cal
```

## 许可证

本项目采用MIT许可证，详情请查看`github/LICENSE`文件。

## 作者

Famarey

