#ifndef VIMMOTIONRESOLVER_H
#define VIMMOTIONRESOLVER_H

#include <QChar>
#include <QTextCursor>

#include <optional>

struct VimMotion
{
    QTextCursor::MoveOperation operation = QTextCursor::NoMove;
};

class VimMotionResolver
{
public:
    static std::optional<VimMotion> resolve(QChar key);
};

#endif
