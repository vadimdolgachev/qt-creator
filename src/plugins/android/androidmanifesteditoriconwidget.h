/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#pragma once

#include <QWidget>

namespace TextEditor {
    class TextEditorWidget;
}

class QLabel;
class QToolButton;

namespace Android {
namespace Internal {

class AndroidManifestEditorIconWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AndroidManifestEditorIconWidget(QWidget *parent);
    AndroidManifestEditorIconWidget(QWidget *parent,
                                    const QSize &buttonSize,
                                    const QString &title,
                                    const QString &tooltip,
                                    TextEditor::TextEditorWidget *textEditorWidget = nullptr,
                                    const QString &targetIconPath = {});
    void setIcon(const QIcon &icon);
    void loadIcon();
    void setIconFromPath(const QString &iconPath);
    bool hasIcon();

signals:
    void iconSelected(const QString &path);

private:
    void selectIcon();
    void removeIcon();
    void copyIcon();
    void setScaleWarningLabelVisible(bool visible);
private:
    QToolButton *m_button = nullptr;
    QSize m_buttonSize;
    QLabel *m_scaleWarningLabel = nullptr;
    TextEditor::TextEditorWidget *m_textEditorWidget = nullptr;
    QString m_iconPath;
    QString m_targetIconPath;
    QString m_iconSelectionText;
};

} // namespace Internal
} // namespace Android