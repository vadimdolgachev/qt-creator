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

#include "edit3dcanvas.h"
#include "edit3dview.h"
#include "edit3dwidget.h"

#include "nodehints.h"
#include "qmlvisualnode.h"

#include <qmldesignerplugin.h>
#include <qmldesignerconstants.h>

#include <QtCore/qmimedata.h>
#include <QPainter>

namespace QmlDesigner {

Edit3DCanvas::Edit3DCanvas(Edit3DWidget *parent)
    : QWidget(parent),
    m_parent(parent)
{
    setMouseTracking(true);
    setAcceptDrops(true);
    setFocusPolicy(Qt::ClickFocus);
}

void Edit3DCanvas::updateRenderImage(const QImage &img)
{
    m_image = img;
    update();
}

void Edit3DCanvas::updateActiveScene(qint32 activeScene)
{
    m_activeScene = activeScene;
}

void Edit3DCanvas::mousePressEvent(QMouseEvent *e)
{
    m_parent->view()->sendInputEvent(e);
    QWidget::mousePressEvent(e);
}

void Edit3DCanvas::mouseReleaseEvent(QMouseEvent *e)
{
    m_parent->view()->sendInputEvent(e);
    QWidget::mouseReleaseEvent(e);
}

void Edit3DCanvas::mouseDoubleClickEvent(QMouseEvent *e)
{
    m_parent->view()->sendInputEvent(e);
    QWidget::mouseDoubleClickEvent(e);
}

void Edit3DCanvas::mouseMoveEvent(QMouseEvent *e)
{
    m_parent->view()->sendInputEvent(e);
    QWidget::mouseMoveEvent(e);
}

void Edit3DCanvas::wheelEvent(QWheelEvent *e)
{
    m_parent->view()->sendInputEvent(e);
    QWidget::wheelEvent(e);
}

void Edit3DCanvas::keyPressEvent(QKeyEvent *e)
{
    m_parent->view()->sendInputEvent(e);
    QWidget::keyPressEvent(e);
}

void Edit3DCanvas::keyReleaseEvent(QKeyEvent *e)
{
    m_parent->view()->sendInputEvent(e);
    QWidget::keyReleaseEvent(e);
}

void Edit3DCanvas::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    QWidget::paintEvent(e);

    QPainter painter(this);

    painter.drawImage(rect(), m_image, QRect(0, 0, m_image.width(), m_image.height()));
}

void Edit3DCanvas::resizeEvent(QResizeEvent *e)
{
    m_parent->view()->edit3DViewResized(e->size());
}

void Edit3DCanvas::dragEnterEvent(QDragEnterEvent *e)
{
    // Block all drags if scene root node is locked
    ModelNode node;
    if (m_parent->view()->hasModelNodeForInternalId(m_activeScene))
        node = m_parent->view()->modelNodeForInternalId(m_activeScene);

    // Allow drop when there is no valid active scene, as the drop goes under the root node of
    // the document in that case.
    if (!node.isValid() || !ModelNode::isThisOrAncestorLocked(node)) {
        QByteArray data = e->mimeData()->data(QStringLiteral("application/vnd.bauhaus.itemlibraryinfo"));
        if (!data.isEmpty()) {
            QDataStream stream(data);
            stream >> m_itemLibraryEntry;
            if (NodeHints::fromItemLibraryEntry(m_itemLibraryEntry).canBeDroppedInView3D())
                e->accept();
        }
    }
}

void Edit3DCanvas::dropEvent(QDropEvent *e)
{
    Q_UNUSED(e)

    auto modelNode = QmlVisualNode::createQml3DNode(m_parent->view(), m_itemLibraryEntry, m_activeScene).modelNode();

    if (modelNode.isValid()) {
        e->accept();
        m_parent->view()->setSelectedModelNode(modelNode);
    }
}

void Edit3DCanvas::focusOutEvent(QFocusEvent *focusEvent)
{
    QmlDesignerPlugin::emitUsageStatisticsTime(Constants::EVENT_3DEDITOR_TIME,
                                               m_usageTimer.elapsed());
    QWidget::focusOutEvent(focusEvent);
}

void Edit3DCanvas::focusInEvent(QFocusEvent *focusEvent)
{
    m_usageTimer.restart();
    QWidget::focusInEvent(focusEvent);
}

}
