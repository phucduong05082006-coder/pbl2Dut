#ifndef CINEMA_PRESENTATION_THEME_H
#define CINEMA_PRESENTATION_THEME_H

#include <QApplication>
#include <QFont>

inline void applyCinemaTheme(QApplication& application)
{
    application.setStyle(QStringLiteral("Fusion"));
    application.setFont(QFont(QStringLiteral("Noto Sans"), 10));
    application.setStyleSheet(QStringLiteral(
        "QMainWindow, QDialog { background: #f5f3ec; }"
        "QWidget { color: #25312c; }"
        "QLabel { background: transparent; }"
        "QLabel#brand { color: #285b43; font: bold 25pt 'Noto Serif'; }"
        "QLabel#heading, QLabel#pageTitle { font: 20pt 'Noto Serif'; }"
        "QLabel#eyebrow { color: #285b43; font-weight: bold; }"
        "QLabel#muted, QLabel#pageDescription { color: #627068; }"
        "QLabel#permissions { background: #e8efe4; color: #285b43; padding: 6px 10px; border-radius: 4px; }"
        "QLabel#auditoriumScreen { background: #e8efe4; color: #285b43; padding: 10px; border-bottom: 3px solid #285b43; font-weight: bold; }"
        "QFrame#sidebar, QFrame#topbar { background: #fffefa; border: 1px solid #dbdfd4; }"
        "QScrollArea { border: 0; background: #f5f3ec; }"
        "QScrollArea > QWidget > QWidget { background: #f5f3ec; }"
        "QLineEdit, QComboBox, QSpinBox, QDateTimeEdit, QTextEdit { background: #fffefa; border: 1px solid #879589; border-radius: 5px; padding: 8px; min-height: 24px; }"
        "QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border: 1px solid #bd4129; }"
        "QLineEdit:read-only { background: #e8efe4; }"
        "QPushButton { background: #fffefa; border: 1px solid #a5afa3; border-radius: 5px; padding: 8px 12px; min-height: 26px; }"
        "QPushButton:hover { background: #e8efe4; }"
        "QPushButton:focus { border: 1px solid #bd4129; }"
        "QPushButton[primary=\"true\"], QPushButton#submit { background: #285b43; border-color: #285b43; color: white; font-weight: bold; }"
        "QPushButton[primary=\"true\"]:hover, QPushButton#submit:hover { background: #1c4431; }"
        "QPushButton:disabled { background: #e4e6df; color: #59645e; border-color: #dbdfd4; }"
        "QPushButton[nav=\"true\"] { text-align: left; border: 0; background: transparent; padding: 10px 12px; }"
        "QPushButton[nav=\"true\"]:checked { background: #e8efe4; color: #285b43; font-weight: bold; }"
        "QPushButton[nav=\"true\"]:disabled { color: #627068; }"
        "QLabel#error, QLabel#filmError, QLabel#filmFormError, QLabel#accountError, QLabel#accountFormError { color: #9c2929; }"
        "QTableWidget { background: #fffefa; alternate-background-color: #f5f3ec; border: 1px solid #dbdfd4; gridline-color: #dbdfd4; selection-background-color: #285b43; selection-color: white; }"
        "QTableWidget::item { padding: 8px; }"
        "QHeaderView::section { background: #e8efe4; padding: 8px; border: 0; border-bottom: 1px solid #dbdfd4; font-weight: bold; }"
        "QCheckBox { spacing: 8px; min-height: 30px; }"
        "QCheckBox::indicator { width: 20px; height: 20px; }"));
}

#endif
