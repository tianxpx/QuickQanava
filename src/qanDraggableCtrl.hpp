/*
    This file is part of QuickQanava library.

    Copyright (C) 2008-2017 Benoit AUTHEMAN

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//-----------------------------------------------------------------------------
// This file is a part of the QuickQanava software library.
//
// \file	DraggableCtrl.cpp
// \author	benoit@destrat.io
// \date	2017 03 15
//-----------------------------------------------------------------------------

// Qt headers
// Nil

// QuickQanava headers
// Nil

namespace qan { // ::qan

/* Node Object Management *///-------------------------------------------------
template <class Node_t, class NodeItem_t>
DraggableCtrl<Node_t, NodeItem_t>::DraggableCtrl()
{
}

template <class Node_t, class NodeItem_t>
DraggableCtrl<Node_t, NodeItem_t>::~DraggableCtrl() { /* Nil */ }
//-----------------------------------------------------------------------------


/* Drag'nDrop Management *///--------------------------------------------------
template <class Node_t, class NodeItem_t>
bool    DraggableCtrl<Node_t, NodeItem_t>::handleDragEnterEvent( QDragEnterEvent* event )
{
    if ( _targetItem->getAcceptDrops() ) {
        if ( event->source() == nullptr ) {
            event->accept(); // This is propably a drag initated with type=Drag.Internal, for exemple a connector drop node, accept by default...
            return true;
        }
    }
    return false;
}

template <class Node_t, class NodeItem_t>
void	DraggableCtrl<Node_t, NodeItem_t>::handleDragMoveEvent( QDragMoveEvent* event )
{
    if ( _targetItem &&
         _targetItem->getAcceptDrops() ) {
        event->acceptProposedAction();
        event->accept();
    }
}

template <class Node_t, class NodeItem_t>
void	DraggableCtrl<Node_t, NodeItem_t>::handleDragLeaveEvent( QDragLeaveEvent* event )
{
    if ( _targetItem &&
         _targetItem->getAcceptDrops() )
        event->ignore();
}

template <class Node_t, class NodeItem_t>
void    DraggableCtrl<Node_t, NodeItem_t>::handleDropEvent( QDropEvent* event )
{
/*    if ( getAcceptDrops() &&
         event->source() != nullptr ) { // Get the source item from the quick drag attached object received
        QQuickItem* sourceItem = qobject_cast<QQuickItem*>(event->source());
        if ( sourceItem != nullptr ) {
            QVariant draggedStyle = sourceItem->property( "draggedNodeStyle" ); // The source item (usually a style node or edge delegate must expose a draggedStyle property).
            if ( draggedStyle.isValid() ) {
                qan::NodeStyle* draggedNodeStyle = draggedStyle.value< qan::NodeStyle* >( );
                if ( draggedNodeStyle != nullptr )
                    setStyle( draggedNodeStyle );
            }
        }
    }
    */
}

template <class Node_t, class NodeItem_t>
void    DraggableCtrl<Node_t, NodeItem_t>::handleMouseDoubleClickEvent(QMouseEvent* event )
{

}

template <class Node_t, class NodeItem_t>
bool    DraggableCtrl<Node_t, NodeItem_t>::handleMouseMoveEvent(QMouseEvent* event )
{
    if ( _target == nullptr ||
         _targetItem == nullptr )
        return false;

    if ( _targetItem->getDraggable() &&      // Dragging management
         event->buttons() | Qt::LeftButton) {
        if ( !_targetItem->getDragged() ) {
            beginDragMove( event->windowPos() );
            return true;
        } else {
            // Inspired by void QQuickMouseArea::mouseMoveEvent(QMouseEvent *event)
            // https://code.woboq.org/qt5/qtdeclarative/src/quick/items/qquickmousearea.cpp.html#47curLocalPos
            // Coordinate mapping in qt quick is even more a nightmare than with graphics view...
            // BTW, this code is probably buggy for deep quick item hierarchy.
            QPointF startLocalPos;
            QPointF curLocalPos;
            if ( _targetItem->parentItem() != nullptr ) {
                startLocalPos = _targetItem->parentItem()->mapFromScene( _dragInitialMousePos );
                curLocalPos = _targetItem->parentItem()->mapFromScene( event->windowPos() );
            } else {
                startLocalPos = _dragInitialMousePos;
                curLocalPos = event->windowPos();
            }
            QPointF delta( curLocalPos - startLocalPos );
            dragMove( event->windowPos(), delta );
            return true;
        }
    }
    return false;
}

template <class Node_t, class NodeItem_t>
void    DraggableCtrl<Node_t, NodeItem_t>::handleMousePressEvent( QMouseEvent* event )
{

}

template <class Node_t, class NodeItem_t>
void    DraggableCtrl<Node_t, NodeItem_t>::handleMouseReleaseEvent( QMouseEvent* event )
{
    Q_UNUSED( event );
    if ( _targetItem &&
         _targetItem->getDragged() )
        endDragMove();
}

template <class Node_t, class NodeItem_t>
auto    DraggableCtrl<Node_t, NodeItem_t>::beginDragMove( const QPointF& dragInitialMousePos, bool dragSelection ) -> void
{
    if ( _targetItem == nullptr )
        return;

    _targetItem->setDragged( true );
    _dragInitialMousePos = dragInitialMousePos;
    _dragInitialPos = _targetItem->parentItem() != nullptr ? _targetItem->parentItem()->mapToScene( _targetItem->position() ) :
                                                             _targetItem->position();

    // If there is a selection, keep start position for all selected nodes.
    if ( dragSelection ) {
        const auto graph = getGraph();
        if ( graph != nullptr &&
             graph->hasMultipleSelection() ) {
            for ( auto& selectedNode : graph->getSelectedNodes() )
                if ( selectedNode != nullptr &&
                     selectedNode->getItem() != nullptr &&
                     static_cast<QQuickItem*>(selectedNode->getItem()) != static_cast<QQuickItem*>(_targetItem.data()) &&
                     selectedNode->getGroup().expired() )       // getGroup.expired() === node not in a group
                    selectedNode->getItem()->draggableCtrl().beginDragMove( dragInitialMousePos, false );
        }
    }
}

template <class Node_t, class NodeItem_t>
auto    DraggableCtrl<Node_t, NodeItem_t>::dragMove( const QPointF& dragInitialMousePos, const QPointF& delta, bool dragSelection ) -> void
{
    const auto graph = getGraph();
    if ( _target &&
         _targetItem &&
         graph != nullptr ) {
        if ( _target->getGroup().lock() ) {
            graph->ungroupNode(_target->getGroup().lock().get(), _target );
            _dragInitialMousePos = dragInitialMousePos;  // Note 20160811: Reset position cache since the node has changed parent and
                                                         // thus position (same scene pos but different local pos)
            _dragInitialPos = _targetItem->parentItem() != nullptr ? _targetItem->parentItem()->mapToScene( _targetItem->position() ) :
                                                                     _targetItem->position();
        }

        QPointF startPos = _targetItem->parentItem() != nullptr ? _targetItem->parentItem()->mapFromScene( _dragInitialPos ) :
                                                                  _dragInitialPos;
        _targetItem->setX( startPos.x() + delta.x() );
        _targetItem->setY( startPos.y() + delta.y() );

        if ( dragSelection ) {
            for ( auto& node : graph->getSelectedNodes() ) {
                qDebug() << "Dragging selection:" << node->getItem() << "\t_sourceItem=" << _targetItem;
                if ( node != nullptr &&
                     node->getItem() != nullptr &&
                     static_cast<QQuickItem*>(node->getItem()) != static_cast<QQuickItem*>(_targetItem.data()) &&
                     node->getGroup().expired() )       // Do not drag nodes that are inside a group
                    node->getItem()->draggableCtrl().dragMove( dragInitialMousePos, delta, false );
            }
        }

        // Eventually, propose a node group drop after move
        if ( _targetItem->getDroppable() ) {
            qan::Group* group = graph->groupAt( _targetItem->position(), { _targetItem->width(), _targetItem->height() } );
            if ( group != nullptr &&
                 group->getItem() != nullptr &&
                 static_cast<QQuickItem*>(group->getItem()) != static_cast<QQuickItem*>(_targetItem.data()) )  { // Do not drop a group in itself
                group->getItem()->proposeNodeDrop();
                _lastProposedGroup = group;
            } else if ( group == nullptr &&
                        _lastProposedGroup != nullptr &&
                        _lastProposedGroup->getItem() != nullptr ) {
                _lastProposedGroup->getItem()->endProposeNodeDrop();
                _lastProposedGroup = nullptr;
            }
        }
    }
}

template <class Node_t, class NodeItem_t>
auto    DraggableCtrl<Node_t, NodeItem_t>::endDragMove( bool dragSelection ) -> void
{
    const auto graph = getGraph();
    if ( _targetItem ) {
        if ( _targetItem->getDroppable() &&
             graph != nullptr &&
             _target ) {
            // FIXME QAN3: map to global graph container
            const auto pos = _targetItem->position(); //parentItem() != nullptr ? parentItem()->mapToScene( position() ) : position();
            qan::Group* group = graph->groupAt( pos, { _targetItem->width(), _targetItem->height() } );
            if ( group != nullptr &&
                 static_cast<QQuickItem*>(group->getItem()) != static_cast<QQuickItem*>(_targetItem.data()) )  // Do not drop a group in itself
                graph->groupNode( group, _target.data() );
        }
        _targetItem->setDragged(false);
    }
    _dragInitialMousePos = { 0., 0. }; // Invalid all cached coordinates when drag ends
    _dragInitialPos = { 0., 0. };
    _lastProposedGroup = nullptr;

    // If there is a selection, end drag for the whole selection
    if ( dragSelection &&
         graph != nullptr &&
         graph->hasMultipleSelection() &&
         _targetItem ) {
        for ( auto& selectedNode : graph->getSelectedNodes() )
            if ( selectedNode != nullptr &&
                 selectedNode->getItem() != nullptr &&
                 static_cast<QQuickItem*>(selectedNode->getItem()) != static_cast<QQuickItem*>(_targetItem) )
                selectedNode->getItem()->draggableCtrl().endDragMove( false );
    }
}
//-----------------------------------------------------------------------------

} // ::qan
