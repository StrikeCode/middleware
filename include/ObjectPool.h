//
// Created by 江宇豪 on 2025/2/28.
//

#ifndef MIDDLEWARES_OBJECTPOOL_H
#define MIDDLEWARES_OBJECTPOOL_H
#include <iostream>
#include <ctime>
using namespace std;


template<typename T>
class Queue
{
public:
    Queue()
    {
        _front = _rear = new QueueItem();
    }
    ~Queue()
    {
        QueueItem* cur = _front;
        while (cur != nullptr)
        {
            _front = _front->_next;
            delete cur;
            cur = _front;
        }
    }

    void push(const T& val)
    {
        // 有了对象池后，这个操作在多数情况下只需要构造而没有分配内存的操作
        QueueItem* item = new QueueItem(val);
        _rear->_next = item;
        _rear = item;
    }
    void pop()
    {
        if (empty())
        {
            return;
        }
        // _front指向队首元素前一个位置
        QueueItem* first = _front->_next;;
        _front->_next = first->_next;
        if (_front->_next == nullptr)
        {
            _rear = _front; // 置空
        }
        delete first;
    }

    T front() const
    {
        return _front->_next->_data;
    }
    bool empty()const { return _front == _rear; }
private:
    // 内部类，队列的元素抽象
    struct QueueItem
    {
        QueueItem(T data = T()) : _data(data), _next(nullptr) {}

#if 0
        // 给QueueItem提供自定义内存管理
        void* operator new(size_t size)
        {
            // 提前开辟POOL_ITEM_SIZE个对象的空间，不构造
            if (_itemPool == nullptr)
            {
                // 分配字节，如果用new QueueItem会陷入递归调用
                _itemPool = (QueueItem*)new char[POOL_ITEM_SIZE * sizeof(QueueItem)];
                QueueItem* p = _itemPool;
                // 将每个对象空间组织成链表
                // 每次循环跳跃空间大小为sizeof(QueueItem)
                for (; p < _itemPool + POOL_ITEM_SIZE - 1; ++p)
                {
                    p->_next = p + 1;
                }
                p->_next = nullptr;
            }
            // 已经有对象池，则从头取出一块对象空间来
            QueueItem* p = _itemPool;
            _itemPool = _itemPool->_next;
            return p;
        }

        void operator delete(void* ptr)
        {
            // 头插法归还空间
            QueueItem* p = (QueueItem*)ptr;
            p->_next = _itemPool;
            _itemPool = p;
        }
#endif
        T _data;
        QueueItem* _next;
        static QueueItem* _itemPool; // 指向对象池首个元素的指针
        static const int POOL_ITEM_SIZE = 100000; // 对象池大小
    };

    QueueItem* _front; // 队头不指向具体元素
    QueueItem* _rear;
};

// typename Queue<T>::QueueItem是告知编译器 Queue<T>::QueueItem是类型而不是成员变量，从而把后面*当乘号
template<typename T>
typename Queue<T>::QueueItem *Queue<T>::QueueItem::_itemPool = nullptr;



#endif //MIDDLEWARES_OBJECTPOOL_H
