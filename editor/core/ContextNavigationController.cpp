#include "ContextNavigationController.h"

#include "ContextManager.h"
#include "WorkspaceRootInteractionController.h"

#include <QKeySequence>
#include <QShortcut>
#include <QWidget>

#include <utility>

ContextNavigationController::ContextNavigationController(
    ContextManager *contextManager, QWidget *shortcutParent,
    SaveContext saveContext,
    WorkspaceRootInteractionController *rootInteraction,
    QObject *parent)
    : QObject(parent),
      m_contextManager(contextManager),
      m_saveContext(std::move(saveContext)),
      m_rootInteraction(rootInteraction)
{
    if (shortcutParent == nullptr) {
        return;
    }

    auto *left = new QShortcut(QKeySequence(QStringLiteral("Alt+Left")),
                               shortcutParent);
    connect(left, &QShortcut::activated, this,
            &ContextNavigationController::navigateLeft);

    auto *right = new QShortcut(QKeySequence(QStringLiteral("Alt+Right")),
                                shortcutParent);
    connect(right, &QShortcut::activated, this,
            &ContextNavigationController::navigateRight);
}

void ContextNavigationController::navigateLeft()
{
    if (m_contextManager == nullptr) {
        return;
    }

    if (m_saveContext) {
        m_saveContext();
    }
    m_contextManager->navigateLeft();
}

void ContextNavigationController::navigateRight()
{
    if (m_contextManager == nullptr) {
        return;
    }

    const int last = m_contextManager->count() - 1;
    const int current = m_contextManager->currentIndex();
    if (current == last && !m_contextManager->currentRoot().isEmpty()) {
        if (m_rootInteraction != nullptr) {
            m_rootInteraction->createContext();
        }
        return;
    }

    if (current < last) {
        if (m_saveContext) {
            m_saveContext();
        }
        m_contextManager->navigateRight();
    }
}
