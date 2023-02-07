#ifndef ANUBIS_TREE_HPP
#include "anubis/Tree.hpp"
#endif 

#include <cassert>

namespace anubis {


template <typename T>
Tree<T>::Tree(const T& val)
    : m_Value(val), m_Parent(nullptr)
{

}

template <typename T>
template <typename T_RETURN>
T_RETURN Tree<T>::traverse(std::function<T_RETURN(T& value, T_RETURN accumulator)> op, T_RETURN init, size_t minDepth, size_t maxDepth)
{
    return Tree<T>::traverseImpl<T_RETURN, Tree<T>, T>(this, op, init, minDepth, maxDepth);
}

template <typename T>
template <typename T_RETURN>
T_RETURN Tree<T>::traverse(std::function<T_RETURN(const T& value, T_RETURN accumulator)> op, T_RETURN init, size_t minDepth, size_t maxDepth) const
{
    return Tree<T>::traverseImpl<T_RETURN, const Tree<T>, const T>(this, op, init, minDepth, maxDepth);
}

template <typename T>
template <typename T_RETURN, typename T_NODE, typename T_DATA>
T_RETURN Tree<T>::traverseImpl(T_NODE* pNode, std::function<T_RETURN(T_DATA& value, T_RETURN accumulator)>& op, T_RETURN init, size_t minDepth, size_t maxDepth)
{
    using NodePtr_t = T_NODE*;
    struct NodeInfo {
        NodePtr_t node;
        size_t depth;
    };

    assert(minDepth <= maxDepth);
    std::stack<NodeInfo> remainingNodes;
    remainingNodes.push({ .node = pNode, .depth = 0 });
    T_RETURN accumulatedVal = init;

    do {
        NodeInfo currentNodeinfo = remainingNodes.top();
        remainingNodes.pop();

        // insert the nodes in the stack (right to left)
        for (auto it = currentNodeinfo.node->m_Chilren.rbegin(); it != currentNodeinfo.node->m_Chilren.rend(); ++it)
        {
            NodePtr_t childNode = it->get();
            remainingNodes.push({ .node = childNode, .depth = currentNodeinfo.depth + 1 });
        }

        if (currentNodeinfo.depth >= minDepth && currentNodeinfo.depth <= maxDepth) {
            accumulatedVal = op(currentNodeinfo.node->m_Value, accumulatedVal);
        }

    } while (!remainingNodes.empty());

    return accumulatedVal;
}

template <typename T>
template <typename ...T_ARGS>
Tree<T>* Tree<T>::addNode(T_ARGS&& ...args)
{
    std::unique_ptr<Tree<T>>& newTree = m_Chilren.emplace_back(std::make_unique<Tree<T>>(std::forward<T_ARGS>(args)...));
    return newTree.get();
}

template <typename T>
size_t Tree<T>::size(void) const noexcept
{
    return traverse<size_t>([](const T& value, size_t accumulator) { return accumulator + 1; });
}


}