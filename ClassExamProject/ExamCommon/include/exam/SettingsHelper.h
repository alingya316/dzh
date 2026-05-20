/* -------------------------------------------------------------------------
//  文件名    : SettingsHelper.h
//  创建者    : dengzihang
//  创建时间  : 2026-05-04
//  功能描述  : 设置面板公共工具——createSettingRow、字体样式、滑条 CSS
// -------------------------------------------------------------------------*/

#pragma once

#include <QLabel>
#include <QSlider>
#include <QString>
#include <QWidget>

/// 设置面板通用滑条样式（SaveApple & SpaceBattle 共用）。
inline const char* kSettingSliderStyle =
    "QSlider{background:transparent;min-height:36px;}"
    "QSlider::groove:horizontal{height:10px;background:rgba(0,0,0,110);border-radius:5px;}"
    "QSlider::handle:horizontal{width:24px;margin:-8px 0;background:#f3f3f3;border:1px solid #777;border-radius:12px;}";

/// 设置面板标签通用样式。
inline const char* kSettingLabelStyle = "color:#2b2b2b;background:transparent;border:none;";

/// 创建一行"标题 + 数值标签 + 滑条"，返回控件引用。
inline void createSettingRow(QWidget* parent,
                             const QString& title,
                             int minValue,
                             int maxValue,
                             int initialValue,
                             int step,
                             QLabel*& titleLabel,
                             QLabel*& valueLabel,
                             QSlider*& slider) {
    titleLabel = new QLabel(title, parent);
    valueLabel = new QLabel(QString::number(initialValue), parent);
    valueLabel->setAlignment(Qt::AlignCenter);
    slider = new QSlider(Qt::Horizontal, parent);
    slider->setRange(minValue, maxValue);
    slider->setSingleStep(step);
    slider->setValue(initialValue);
}

/// 设置面板中文标签字体（SimSun, 14pt, Bold）。
inline void applyCnLabelFont(QLabel* label, int pointSize = 14) {
    if (!label) return;
    QFont f("SimSun");
    f.setPointSize(pointSize);
    f.setBold(true);
    label->setFont(f);
    label->setStyleSheet(QLatin1String(kSettingLabelStyle));
}

/// 数值标签字体（Times New Roman, 14pt, Bold）。
inline void applyValueLabelFont(QLabel* label, int pointSize = 14) {
    if (!label) return;
    QFont f("Times New Roman");
    f.setPointSize(pointSize);
    f.setBold(true);
    label->setFont(f);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(QLatin1String(kSettingLabelStyle));
}

/// 以 blockSignals 方式静默设置控件值，避免触发信号。
template <typename Widget, typename ValueType>
inline void setValueSilently(Widget* widget, const ValueType& value) {
    if (!widget) return;
    widget->blockSignals(true);
    widget->setValue(value);
    widget->blockSignals(false);
}
