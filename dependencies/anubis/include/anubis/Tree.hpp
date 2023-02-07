#ifndef ANUBIS_TREE_HPP
#define ANUBIS_TREE_HPP

#include <vector>
#include <memory>
#include <functional>
#include <stack>

namespace anubis {

    template <typename T>
    class Tree
    {

        using Node_t = Tree<T>*;
        using value_type = T;
        static constexpr size_t SIZE_T_MAX = std::numeric_limits<size_t>::max();
    public:
        Tree(const T& val = T());

        template <typename T_RETURN>
        T_RETURN traverse(std::function<T_RETURN(T& value, T_RETURN accumulator)> op, T_RETURN init = T_RETURN(), size_t minDepth = 0, size_t maxDepth = SIZE_T_MAX);

        template <typename T_RETURN>
        T_RETURN traverse(std::function<T_RETURN(const T& value, T_RETURN accumulator)> op, T_RETURN init = T_RETURN(), size_t minDepth = 0, size_t maxDepth = SIZE_T_MAX) const;

        template <typename ...T_ARGS>
        Tree* addNode(T_ARGS&& ...args);

        size_t size(void) const noexcept;

    private:
        template <typename T_RETURN, typename T_NODE, typename T_DATA>
        static T_RETURN traverseImpl(T_NODE* pNode, std::function<T_RETURN(T_DATA& value, T_RETURN accumulator)>& op, T_RETURN init, size_t minDepth, size_t maxDepth);


        T m_Value;
        Tree* m_Parent;
        std::vector<std::unique_ptr<Tree>> m_Chilren;
    };
}

#include "anubis/Tree.inl"

#endif // !ANUBIS_TREE_HPP