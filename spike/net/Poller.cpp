#include "Poller.h"

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

namespace lim_webserver
{
    bool Poller::hasChannel(IoChannel *channel)
    {
        auto it = m_channels_map.find(channel->fd());
        return it != m_channels_map.end() && it->second == channel;
    }

    EpollPoller::EpollPoller(EventLoop *loop)
        : Poller(loop), m_epfd(epoll_create(5000))
    {
    }

    EpollPoller::~EpollPoller()
    {
        ::close(m_epfd);
    }

    void EpollPoller::updateChannel(IoChannel *channel)
    {
        // 获取Channel状态与句柄
        IoChannelState state = channel->state();
        int fd = channel->fd();
        // 若为新Channel或为空Channel
        if (state == IoChannelState::NEW || state == IoChannelState::DISCARD)
        {
            // 新Channel则存入红黑树
            if (state == IoChannelState::NEW)
            {
                assert(m_channels_map.find(fd) == m_channels_map.end());
                m_channels_map[fd] = channel;
            }
            else // 老Channel则确保存在
            {
                assert(m_channels_map.find(fd) != m_channels_map.end());
                assert(m_channels_map[fd] == channel);
            }
            // 设置Channel状态为常规
            channel->setState(IoChannelState::EXIST);
            // 添加句柄
            update(EPOLL_CTL_ADD, channel);
        }
        else // 已经存在的Channel则确保其状态
        {
            assert(m_channels_map.find(fd) != m_channels_map.end());
            assert(m_channels_map[fd] == channel);
            assert(state == IoChannelState::EXIST);
            // 若Channel没有事件则删除
            if (channel->isNoneEvent())
            {
                update(EPOLL_CTL_DEL, channel);
                channel->setState(IoChannelState::DISCARD);
            }
            else // 有事件则修改
            {
                update(EPOLL_CTL_MOD, channel);
            }
        }
    }

    void EpollPoller::removeChannel(IoChannel *channel)
    {
        // 获得文件句柄
        int fd = channel->fd();
        assert(m_channels_map.find(fd) != m_channels_map.end());
        assert(m_channels_map[fd] == channel);
        assert(channel->isNoneEvent());

        // 获得状态
        IoChannelState state = channel->state();
        assert(state == IoChannelState::EXIST || state == IoChannelState::DISCARD);

        // 移除Channel
        size_t n = m_channels_map.erase(fd);
        (void)n;
        assert(n == 1);

        // 若为活动句柄，则废弃
        if (state == IoChannelState::EXIST)
        {
            update(EPOLL_CTL_DEL, channel);
        }

        // 重置句柄状态
        channel->setState(IoChannelState::NEW);
    }

    void EpollPoller::poll(int ms)
    {
        // 等待文件描述符
        int n = epoll_wait(m_epfd, &*m_event_vec.begin(), m_event_vec.size(), ms);

        // 若有则触发回调
        if (n > 0)
        {
            for (auto i = 0; i < n; ++i)
            {
                epoll_event &event = m_event_vec[i];
                IoChannel *channel = static_cast<IoChannel *>(event.data.ptr);
                // 根据得到的events触发回调
                channel->trigger(event.events);
            }
        }
    }

    void EpollPoller::update(int op, IoChannel *channel)
    {
        struct epoll_event ev;
        ev.events = channel->event();
        ev.data.ptr = channel;
        int fd = channel->fd();

        int rt = epoll_ctl(m_epfd, op, fd, &ev);
    }

} // namespace lim_webserver
