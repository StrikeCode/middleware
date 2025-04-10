//
// Created by 江宇豪 on 2025/3/10.
//
#include "../include/Msg.h"

#include <atomic>

namespace PolyM {

    namespace {

        MsgUID generateUniqueId()
        {
            static std::atomic<MsgUID> i(0);
            return ++i;
        }

    }

    Msg::Msg(int msgId)
            : msgId_(msgId), uniqueId_(generateUniqueId())
    {
    }

    std::unique_ptr<Msg> Msg::move()
    {
        return std::unique_ptr<Msg>(new Msg(std::move(*this)));
    }

    int Msg::getMsgId() const
    {
        return msgId_;
    }

    MsgUID Msg::getUniqueId() const
    {
        return uniqueId_;
    }

}
