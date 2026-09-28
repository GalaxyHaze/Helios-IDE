#include "VimMotionResolver.h"

std::optional<VimMotion> VimMotionResolver::resolve(QChar key)
{
    switch (key.unicode()) {
    case 'h':
        return VimMotion{QTextCursor::Left};
    case 'j':
        return VimMotion{QTextCursor::Down};
    case 'k':
        return VimMotion{QTextCursor::Up};
    case 'l':
        return VimMotion{QTextCursor::Right};
    case 'w':
    case 'W':
        return VimMotion{QTextCursor::NextWord};
    case 'b':
    case 'B':
        return VimMotion{QTextCursor::PreviousWord};
    case 'e':
        return VimMotion{QTextCursor::EndOfWord};
    default:
        return std::nullopt;
    }
}
