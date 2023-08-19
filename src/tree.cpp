/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "tree.h"

#include <QList>
#include <assert.h>

#include "log.h"
#include "util.h"

TreeNode::TreeNode()
    : m_parent(NULL)
{

}

TreeNode::TreeNode(QString name)
    : m_parent(NULL)
    ,m_name(name)
{
    
}

int TreeNode::getDataInt(int defaultValue) const
{
    bool ok = false;
    int val = m_data.toInt(&ok,0);
    if(ok)
        return val;
    return defaultValue;
}

void TreeNode::addChild(TreeNode *child)
{
    child->m_parent = this;
   
    m_children.push_back(child);
    m_childMap[child->m_name] = child;

}


void TreeNode::copy(const TreeNode &other)
{

    // Remove all children
    removeAll();

    // Set name and data
    m_name = other.m_name;
    m_data = other.m_data;

    // Copy all children
    for(int i = 0;i < other.m_children.size();i++)
    {
        const TreeNode* otherNode = other.m_children[i];
        TreeNode* thisNode = new TreeNode;
        thisNode->copy(*otherNode);

        addChild(thisNode);
    }

}




TreeNode::~TreeNode()
{
    for(int i = 0;i < m_children.size();i++)
    {
        TreeNode* node = m_children[i];
        delete node;
    }

}



void TreeNode::removeAll()
{
    for(int i = 0;i < m_children.size();i++)
    {
        TreeNode* node = m_children[i];
        delete node;
    }
    m_children.clear();
        
}

    

void TreeNode::dump(int parentCnt)
{
    QString text;
    text = QString::asprintf("+- %s='%s'",
            stringToCStr(m_name), stringToCStr(m_data));

    for(int i = 0;i < parentCnt;i++)
        text  = "    " + text;
    debugMsg("%s", stringToCStr(text));
    for(int i = 0;i < (int)m_children.size();i++)
    {
        TreeNode *node = m_children[i];
        node->dump(parentCnt+1);
    }

}



void TreeNode::dump()
{
    
    dump(0);
}


Tree::Tree()
{
}





int TreeNode::getChildDataInt(QString childName, int defaultValue) const
{
    TreeNode *child = findChild(childName);
    if(child)
        return child->getDataInt(defaultValue);
    return defaultValue;
}


QString TreeNode::getChildDataString(QString childName) const
{
    TreeNode *child = findChild(childName);
    if(child)
        return child->m_data;
    return "";
}

long long TreeNode::getChildDataLongLong(QString childPath, long long defaultValue) const
{
    TreeNode *child = findChild(childPath);
    if(child)
        return stringToLongLong(stringToCStr(child->m_data));
    return defaultValue;
}

    
TreeNode *TreeNode::findChild(QString path) const
{
    QString childName;
    QString restPath;
    int indexPos;

    // Find the seperator in the string 
    indexPos = path.indexOf('/');
    if(indexPos == 0)
        return findChild(path.mid(1));

    // Get the first child name
    if(indexPos == -1)
        childName = path;
    else
    {
        childName = path.left(indexPos);
        restPath = path.mid(indexPos+1);
    }

    if(childName.startsWith('#'))
    {
        QString numStr = childName.mid(1);
        int idx = atoi(stringToCStr(numStr))-1;
        if(0 <= idx  && idx < getChildCount())
        {
            TreeNode *child = getChild(idx);
            if(restPath.isEmpty())
                return child;
            else
                return child->findChild(restPath);
        }
    }
    else
    {
        
        TreeNode *child;

        if(m_childMap.contains(childName))
        {
            child = m_childMap[childName];
            assert(child->m_name == childName);
            if(restPath.isEmpty())
                return child;
            else
                return child->findChild(restPath);
        }
        else
        {
        // Look for the child
        for(int u = 0;u < getChildCount();u++)
        {
            child = getChild(u);

            if(child->getName() == childName)
            {
                if(restPath.isEmpty())
                    return child;
                else
                    return child->findChild(restPath);
            }
        }
        }
    }
        
    return NULL;
}



QString Tree::getString(QString path) const
{
    return m_root.getChildDataString(path);
}


int Tree::getInt(QString path, int defaultValue) const
{
    return m_root.getChildDataInt(path, defaultValue);
}


long long Tree::getLongLong(QString path) const
{
    return m_root.getChildDataLongLong(path);
}

TreeNode* Tree::findChild(QString path) const
{
    return m_root.findChild(path);
}



void Tree::removeAll()
{
    m_root.removeAll();
}


void Tree::copy(const Tree &other)
{
    m_root.copy(other.m_root);

}


    
