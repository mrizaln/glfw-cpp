#include "glfw_cpp/event.hpp"

namespace glfw_cpp
{
    EventQueue::EventQueue(std::size_t capacity) noexcept
        : m_buffer{ std::make_unique<Event[]>(capacity) }
        , m_capacity{ capacity }
    {
    }

    std::size_t EventQueue::capacity() const noexcept
    {
        return m_capacity;
    }

    std::size_t EventQueue::size() const noexcept
    {
        return m_end == npos ? capacity() : (m_end + capacity() - m_begin) % capacity();
    }

    bool EventQueue::empty() const noexcept
    {
        return size() == 0;
    }

    bool EventQueue::full() const noexcept
    {
        return size() == capacity();
    }

    void EventQueue::swap(EventQueue& other) noexcept
    {
        std::swap(m_buffer, other.m_buffer);
        std::swap(m_capacity, other.m_capacity);
        std::swap(m_begin, other.m_begin);
        std::swap(m_end, other.m_end);
    }

    void EventQueue::reset() noexcept
    {
        m_begin = 0;
        m_end   = 0;
    }

    void EventQueue::clear() noexcept
    {
        reset();

        for (std::size_t i = 0; i < m_capacity; ++i) {
            m_buffer[i] = Event{};
        }
    }

    EventQueue::Iterator<> EventQueue::push(Event&& event) noexcept
    {
        auto current = m_begin;

        // this branch only taken when the buffer is not full
        if (m_end != npos) {
            current           = m_end;
            m_buffer[current] = std::move(event);
            if (++m_end == capacity()) {
                m_end = 0;
            }
            if (m_end == m_begin) {
                m_end = npos;
            }
        } else {
            current           = m_begin;
            m_buffer[current] = std::move(event);
            if (++m_begin == capacity()) {
                m_begin = 0;
            }
        }

        return { this, current };
    }

    std::optional<Event> EventQueue::pop() noexcept
    {
        if (empty()) {
            return std::nullopt;
        }

        std::optional<Event> value{ std::in_place, std::move(m_buffer[m_begin]) };
        if (m_end == npos) {
            m_end = m_begin;
        }
        if (++m_begin == capacity()) {
            m_begin = 0;
        }

        return value;
    }

    void EventQueue::resize(std::size_t new_capacity, ResizePolicy policy) noexcept
    {
        if (new_capacity == capacity()) {
            return;
        }

        if (empty()) {
            m_buffer   = std::make_unique<Event[]>(new_capacity);
            m_capacity = new_capacity;
            m_begin    = 0;
            m_end      = 0;

            return;
        }

        if (new_capacity > capacity()) {
            const auto b = [buf = m_buffer.get()](std::size_t offset) {
                return std::move_iterator{ buf + offset };
            };

            auto buffer = std::make_unique<Event[]>(new_capacity);
            std::rotate_copy(b(0), b(m_begin), b(capacity()), buffer.get());
            m_buffer   = std::move(buffer);
            m_end      = m_end == npos ? capacity() : (m_end + capacity() - m_begin) % capacity();
            m_begin    = 0;
            m_capacity = new_capacity;

            return;
        }

        auto buffer = std::make_unique<Event[]>(new_capacity);
        auto count  = size();
        auto offset = count <= new_capacity ? 0ul : count - new_capacity;

        switch (policy) {
        case ResizePolicy::DiscardOld: {
            auto begin = (m_begin + offset) % capacity();
            for (std::size_t i = 0; i < std::min(new_capacity, count); ++i) {
                buffer[i] = std::move(m_buffer[(begin + i) % capacity()]);
            }
        } break;
        case ResizePolicy::DiscardNew: {
            auto end = m_end == npos ? m_begin : m_end;
            end      = (end + capacity() - offset) % capacity();
            for (std::size_t i = std::min(new_capacity, count); i-- > 0;) {
                end       = (end + capacity() - 1) % capacity();
                buffer[i] = std::move(m_buffer[end]);
            }
        } break;
        }

        m_buffer   = std::move(buffer);
        m_capacity = new_capacity;
        m_begin    = 0;
        m_end      = count <= new_capacity ? count : npos;
    }

    EventQueue::Iterator<> EventQueue::begin() noexcept
    {
        return { this, m_begin };
    }

    EventQueue::Iterator<> EventQueue::end() noexcept
    {
        return { this, m_end };
    }

    EventQueue::Iterator<true> EventQueue::begin() const noexcept
    {
        return { this, m_begin };
    }

    EventQueue::Iterator<true> EventQueue::end() const noexcept
    {
        return { this, m_end };
    }

    EventQueue::Iterator<true> EventQueue::cbegin() const noexcept
    {
        return { this, m_begin };
    }

    EventQueue::Iterator<true> EventQueue::cend() const noexcept
    {
        return { this, m_end };
    }
}
