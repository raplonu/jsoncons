// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_STAJ_ITERATOR_HPP
#define JSONCONS_STAJ_ITERATOR_HPP

#include <new> // placement new
#include <memory>
#include <string>
#include <stdexcept>
#include <system_error>
#include <ios>
#include <iterator> // std::input_iterator_tag
#include <jsoncons/json_exception.hpp>
#include <jsoncons/staj_cursor.hpp>
#include <jsoncons/basic_json.hpp>
#include <jsoncons/deser_traits.hpp>

namespace jsoncons {

    template <class Json,class T=Json>
    class staj_array_view;

    template<class Json, class T>
    class staj_array_iterator
    {
        using char_type = typename Json::char_type;

        staj_array_view<Json, T>* view_;
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::input_iterator_tag;

        staj_array_iterator() noexcept
            : view_(nullptr)
        {
        }

        staj_array_iterator(staj_array_view<Json, T>& view)
            : view_(std::addressof(view))
        {
            if (view_->cursor_->current().event_type() == staj_event_type::begin_array)
            {
                next();
            }
            else
            {
                view_->cursor_ = nullptr;
            }
        }

        staj_array_iterator(staj_array_view<Json, T>& view,
                            std::error_code& ec)
            : view_(std::addressof(view))
        {
            if (view_->cursor_->current().event_type() == staj_event_type::begin_array)
            {
                next(ec);
                if (ec) {view_ = nullptr;}
            }
            else
            {
                view_ = nullptr;
            }
        }

        ~staj_array_iterator() noexcept
        {
        }

        const T& operator*() const
        {
            return *view_->value_;
        }

        const T* operator->() const
        {
            return view_->value_.operator->();
        }

        staj_array_iterator& operator++()
        {
            next();
            return *this;
        }

        staj_array_iterator& increment(std::error_code& ec)
        {
            next(ec);
            if (ec) {view_ = nullptr;}
            return *this;
        }

        staj_array_iterator operator++(int) // postfix increment
        {
            staj_array_iterator temp(*this);
            next();
            return temp;
        }

        friend bool operator==(const staj_array_iterator<Json,T>& a, const staj_array_iterator<Json,T>& b)
        {
            return (!a.view_ && !b.view_)
                || (!a.view_ && b.done())
                || (!b.view_ && a.done());
        }

        friend bool operator!=(const staj_array_iterator<Json,T>& a, const staj_array_iterator<Json,T>& b)
        {
            return !(a == b);
        }

    private:

        bool done() const
        {
            return view_->cursor_->done() || view_->cursor_->current().event_type() == staj_event_type::end_array;
        }


        void next()
        {
            using char_type = typename Json::char_type;

            if (!done())
            {
                view_->cursor_->next();
                if (!done())
                {
                    std::error_code ec;
                    view_->value_ = deser_traits<T,char_type>::deserialize(*view_->cursor_, view_->decoder_, ec);
                    if (ec)
                    {
                        JSONCONS_THROW(ser_error(ec, view_->cursor_->context().line(), view_->cursor_->context().column()));
                    }
                }
            }
        }

        void next(std::error_code& ec)
        {
            using char_type = typename Json::char_type;

            if (!done())
            {
                view_->cursor_->next(ec);
                if (ec)
                {
                    return;
                }
                if (!done())
                {
                    view_->value_ = deser_traits<T,char_type>::deserialize(*view_->cursor_, view_->decoder_, ec);
                }
            }
        }
    };

    template <class Json,class T=Json>
    class staj_object_view;

    template <class Json, class T=Json>
    class staj_object_iterator
    {
        using char_type = typename Json::char_type;

        staj_object_view<Json, T>* view_;
    public:
        using key_type = std::basic_string<char_type>;
        using value_type = std::pair<key_type,T>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::input_iterator_tag;

    public:

        staj_object_iterator() noexcept
            : view_(nullptr)
        {
        }

        staj_object_iterator(staj_object_view<Json, T>& view)
            : view_(std::addressof(view))
        {
            if (view_->cursor_->current().event_type() == staj_event_type::begin_object)
            {
                next();
            }
            else
            {
                view_ = nullptr;
            }
        }

        staj_object_iterator(staj_object_view<Json, T>& view, 
                             std::error_code& ec)
            : view_(std::addressof(view))
        {
            if (view_->cursor_->current().event_type() == staj_event_type::begin_object)
            {
                next(ec);
                if (ec) {view_ = nullptr;}
            }
            else
            {
                view_ = nullptr;
            }
        }

        ~staj_object_iterator() noexcept
        {
        }

        const value_type& operator*() const
        {
            return *view_->key_value_;
        }

        const value_type* operator->() const
        {
            return view_->key_value_.operator->();
        }

        staj_object_iterator& operator++()
        {
            next();
            return *this;
        }

        staj_object_iterator& increment(std::error_code& ec)
        {
            next(ec);
            if (ec)
            {
                view_ = nullptr;
            }
            return *this;
        }

        staj_object_iterator operator++(int) // postfix increment
        {
            staj_object_iterator temp(*this);
            next();
            return temp;
        }

        friend bool operator==(const staj_object_iterator<Json,T>& a, const staj_object_iterator<Json,T>& b)
        {
            return (!a.view_ && !b.view_)
                   || (!a.view_ && b.done())
                   || (!b.view_ && a.done());
        }

        friend bool operator!=(const staj_object_iterator<Json,T>& a, const staj_object_iterator<Json,T>& b)
        {
            return !(a == b);
        }

    private:

        bool done() const
        {
            return view_->cursor_->done() || view_->cursor_->current().event_type() == staj_event_type::end_object;
        }

        void next()
        {
            using char_type = typename Json::char_type;

            view_->cursor_->next();
            if (!done())
            {
                JSONCONS_ASSERT(view_->cursor_->current().event_type() == staj_event_type::key);
                key_type key = view_->cursor_->current(). template get<key_type>();
                view_->cursor_->next();
                if (!done())
                {
                    std::error_code ec;
                    view_->key_value_ = value_type(std::move(key),deser_traits<T,char_type>::deserialize(*view_->cursor_, view_->decoder_, ec));
                    if (ec)
                    {
                        JSONCONS_THROW(ser_error(ec, view_->cursor_->context().line(), view_->cursor_->context().column()));
                    }
                }
            }
        }

        void next(std::error_code& ec)
        {
            using char_type = typename Json::char_type;

            view_->cursor_->next(ec);
            if (ec)
            {
                return;
            }
            if (!done())
            {
                JSONCONS_ASSERT(view_->cursor_->current().event_type() == staj_event_type::key);
                auto key = view_->cursor_->current(). template get<key_type>();
                view_->cursor_->next(ec);
                if (ec)
                {
                    return;
                }
                if (!done())
                {
                    view_->key_value_ = value_type(std::move(key),deser_traits<T,char_type>::deserialize(*view_->cursor_, view_->decoder_, ec));
                }
            }
        }
    };

    // staj_array_view

    template <class Json,class T>
    class staj_array_view
    {
        friend class staj_array_iterator<Json,T>;
    public:
        using char_type = typename Json::char_type;
        using iterator = staj_array_iterator<Json,T>;
    private:
        basic_staj_cursor<char_type>* cursor_;
        json_decoder<Json> decoder_;
        optional<T> value_;
    public:
        staj_array_view(basic_staj_cursor<char_type>& cursor) 
            : cursor_(std::addressof(cursor))
        {
        }

        iterator begin()
        {
            return staj_array_iterator<Json,T>(*this);
        }

        iterator end()
        {
            return staj_array_iterator<Json,T>();
        }
    };

    // staj_object_view

    template <class Json,class T>
    class staj_object_view
    {
        friend class staj_object_iterator<Json,T>;
    public:
        using char_type = typename Json::char_type;
        using iterator = staj_object_iterator<Json,T>;
        using key_type = std::basic_string<char_type>;
        using value_type = std::pair<key_type,T>;
    private:
        basic_staj_cursor<char_type>* cursor_;
        json_decoder<Json> decoder_;
        optional<value_type> key_value_;
    public:
        staj_object_view(basic_staj_cursor<char_type>& cursor) 
            : cursor_(std::addressof(cursor))
        {
        }

        iterator begin()
        {
            return staj_object_iterator<Json,T>(*this);
        }

        iterator end()
        {
            return staj_object_iterator<Json,T>();
        }
    };

    template <class T, class CharT, class Json=typename std::conditional<is_basic_json<T>::value,T,basic_json<CharT>>::type>
    staj_array_view<Json,T> staj_array(basic_staj_cursor<CharT>& cursor)
    {
        return staj_array_view<Json,T>(cursor);
    }

    template <class T, class CharT, class Json=typename std::conditional<is_basic_json<T>::value,T,basic_json<CharT>>::type>
    staj_object_view<Json,T> staj_object(basic_staj_cursor<CharT>& cursor)
    {
        return staj_object_view<Json,T>(cursor);
    }

} // namespace jsoncons

#endif

