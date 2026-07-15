#include "TomlSettingsStore.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>

TomlSettingsStore::TomlSettingsStore(QObject *parent)
    : QObject(parent)
{
    load();
}

TomlSettingsStore &TomlSettingsStore::instance()
{
    static TomlSettingsStore store;
    return store;
}

QString TomlSettingsStore::filePath() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return QDir(dir).filePath("settings.toml");
}

void TomlSettingsStore::load()
{
    QString path = filePath();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#') || line.startsWith('['))
            continue;

        int eqIdx = line.indexOf('=');
        if (eqIdx == -1)
            continue;

        QString key = line.left(eqIdx).trimmed();
        QString val = line.mid(eqIdx + 1).trimmed();

        if (key == "theme") {
            if (val.startsWith('"') && val.endsWith('"') && val.length() >= 2)
                m_theme = val.mid(1, val.length() - 2);
        } else if (key == "locale") {
            if (val.startsWith('"') && val.endsWith('"') && val.length() >= 2)
                m_locale = val.mid(1, val.length() - 2);
        } else if (key == "fontFamily") {
            if (val.startsWith('"') && val.endsWith('"') && val.length() >= 2) m_fontFamily = val.mid(1, val.length() - 2); else m_fontFamily = val.trimmed();
        } else if (key == "fontSize") {
            m_fontSize = val.toInt();
            if (m_fontSize < 6) m_fontSize = 13;
        } else if (key == "wordWrap") {
            m_wordWrap = (val == "true");
        } else if (key == "sidebarWidth") {
            m_sidebarWidth = val.toInt();
            if (m_sidebarWidth < 50) m_sidebarWidth = 280;
        } else if (key == "sidebarVisible") {
            m_sidebarVisible = (val == "true");
        } else if (key == "treeMaxDepth") {
            m_treeMaxDepth = val.toInt();
            if (m_treeMaxDepth < 1 || m_treeMaxDepth > 64) m_treeMaxDepth = 12;
        } else if (key == "onboardingDismissed") {
            m_onboardingDismissed = (val == "true");
        } else if (key == "lspEnabled") {
            m_lspEnabled = (val == "true");
        } else if (key == "recentProjects") {
            m_recentProjects.clear();
            if (val.startsWith('[') && val.endsWith(']')) {
                QString content = val.mid(1, val.length() - 2).trimmed();
                if (!content.isEmpty()) {
                    QStringList items = content.split(',');
                    for (const QString &item : items) {
                        QString clean = item.trimmed();
                        if (clean.startsWith('"') && clean.endsWith('"') && clean.length() >= 2) {
                            clean = clean.mid(1, clean.length() - 2);
                            if (!clean.isEmpty()) {
                                m_recentProjects.append(clean);
                            }
                        }
                    }
                }
            }
        }
    }
}

void TomlSettingsStore::save()
{
    QString path = filePath();
    QFileInfo info(path);
    QDir().mkpath(info.absolutePath());

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);
    out << "# Helios configuration file\n\n";

    out << "[editor]\n";
    out << "fontFamily = \"" << m_fontFamily << "\"\n";
    out << "fontSize = " << m_fontSize << "\n";
    out << "wordWrap = " << (m_wordWrap ? "true" : "false") << "\n\n";

    out << "[ui]\n";
    out << "theme = \"" << m_theme << "\"\n";
    out << "locale = \"" << m_locale << "\"\n";
    out << "sidebarWidth = " << m_sidebarWidth << "\n";
    out << "sidebarVisible = " << (m_sidebarVisible ? "true" : "false") << "\n";
    out << "treeMaxDepth = " << m_treeMaxDepth << "\n";
    out << "onboardingDismissed = " << (m_onboardingDismissed ? "true" : "false") << "\n\n";

    out << "[lsp]\n";
    out << "lspEnabled = " << (m_lspEnabled ? "true" : "false") << "\n\n";

    out << "[projects]\n";
    out << "recentProjects = [";
    for (int i = 0; i < m_recentProjects.size(); ++i) {
        out << "\"" << m_recentProjects[i] << "\"";
        if (i < m_recentProjects.size() - 1)
            out << ", ";
    }
    out << "]\n";
}

void TomlSettingsStore::addRecentProject(const QString &project)
{
    if (project.isEmpty()) return;
    m_recentProjects.removeAll(project);
    m_recentProjects.prepend(project);
    while (m_recentProjects.size() > 10) {
        m_recentProjects.removeLast();
    }
    save();
}
